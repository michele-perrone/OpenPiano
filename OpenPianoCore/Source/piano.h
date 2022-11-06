/*
OpenPiano: an open source piano engine based on physical modeling
Copyright (C) 2021-2022 Michele Perrone
Github: https://github.com/michele-perrone/OpenPiano
Author e-mail: perrone(dot)michele(at)outlook(dot)com
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as published
by the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef PIANO_H
#define PIANO_H

#include "string_hammer.h"
#include <thread>
#include <vector>
#include <atomic>
#include <mutex>
#include <iostream>

/* ***************************************************** *
 * The piano class contains all the strings and hammers. *
 * It provides a simple API to compute audio blocks.     *
 * ***************************************************** */

enum NoteIndices
{
                                            A0, A0s, B0,
    C1, C1s, D1, D1s, E1, F1, F1s, G1, G1s, A1, A1s, B1,
    C2, C2s, D2, D2s, E2, F2, F2s, G2, G2s, A2, A2s, B2,
    C3, C3s, D3, D3s, E3, F3, F3s, G3, G3s, A3, A3s, B3,
    C4, C4s, D4, D4s, E4, F4, F4s, G4, G4s, A4, A4s, B4,
    C5, C5s, D5, D5s, E5, F5, F5s, G5, G5s, A5, A5s, B5,
    C6, C6s, D6, D6s, E6, F6, F6s, G6, G6s, A6, A6s, B6,
    C7, C7s, D7, D7s, E7, F7, F7s, G7, G7s, A7, A7s, B7,
    C8
};

const int FIRST_NOTE = A0;
const int LAST_NOTE = C5;
const int N_STRINGS = LAST_NOTE-FIRST_NOTE+1;
const int MIDI_NOTE_OFFSET = 21;
const int N_WHITE_KEYS = 31; // 52 for the entire piano range

struct Piano
{
    Hammer* hammers[N_STRINGS];
    PianoString* strings[N_STRINGS];

    int sample_rate;
    size_t samples_per_block;

    size_t N_THREADS; // How many threads should we start
    std::thread** threads; // Array that stores the pointers to the active threads
    float** buffers; // Array of audio buffers, one for each thread, with length "samples_per_block"
    std::atomic<bool>* thr_running; // Array that contains one flag for each thread.
                                    // It is set to "true" to keep the thread inside a "while" loop.
                                    // It is set to "false" when we want to terminate the thread.
    std::atomic<bool>* thr_waiting_for_block; // Array that contains one flag for each thread.
                                              // Used to keep threads running in an empty "while" loop
                                              // until next audio block is requested
    std::atomic<int> n_running_threads; // How many threads are computing an audio block right now.
                                        // n_running_threads == 0 -> all threads are doing nothing
                                        // n_running_threads == N_THREADS -> all threads are working
    std::atomic<uint32_t> sleep_duration; // How often threads should wake up to check if the
                                          // next audio block has been requested
    int* thr_note_range; // For each thread store the note range to compute (first and last note)

    Piano(int sample_rate, int samples_per_block, int n_threads)
    {
        this->sample_rate = sample_rate;
        this->samples_per_block = samples_per_block;
        if (n_threads > 8) // More than 8 threads doesn't make much sense right now
            this->N_THREADS = 8;
        else if (n_threads < 1) // Could happen if "std::thread::hardware_concurrency" returns 0
            this->N_THREADS = 1;
        else
            this->N_THREADS = n_threads;
        this->n_running_threads = 0;

        // Initialize the audio buffers
        init_buffers();

        // Initialize the threads
        init_threads();

        // Initialize the hammers with their physical parameters
        init_hammers();

        // Initialize the strings with its physical parameters
        init_strings();
    }
    ~Piano()
    {
        // Delete the threads
        for(int i = 0; i < N_THREADS; i++)
        {
            // Stop the current audio block computation: set the "thr_waiting_for_block" flag to "true"
            // This is a precaution: typically, we don't exit the program in the middle of an audio block.
            // But, for example, we could be dealing with an extremely long block, and there's no need
            // to wait for its completion before exiting the program
            thr_waiting_for_block[i] = true;

            // Stop the "while" loop: set the "thread_running" flags to "false"
            thr_running[i] = false;

            // Each thread checks the "thr_running" flag every "sleep_duration" microseconds,
            // so let's sleep for a while and give him the time to check it
            std::this_thread::sleep_for(std::chrono::microseconds(sleep_duration*2));

            // Finally, we can safely delete the threads
            // (reminder: don't join them, because they're detached)
            delete threads[i];
        }
        free(threads);
        free(thr_note_range);

        // Delete the flag arrays
        free(thr_waiting_for_block);
        free(thr_running);

        // Delete the hammers and strings
        for(int i = 0; i < N_STRINGS; i++)
        {
            delete hammers[i];
            delete strings[i];
        }

        // Free the audio buffers
        for(int i = 0; i < N_THREADS; i++)
        {
            free(buffers[i]);
        }
        free(buffers);
    }
    void get_next_block_multithreaded(float* buffer, int samples_per_block, float gain)
    {
        // TODO: Each time this function is called, we have to check if "n_running_threads" not zero.
        //  If not, it means that this function got called before it had time to finish computing the previous block.
        //  In that case, the threads have to be immediately notified to stop computing the previous block
        //  and to start with the new one.

        // Activate the threads
        for(int idx_thread = 0; idx_thread < N_THREADS; idx_thread++)
        {
            thr_waiting_for_block[idx_thread] = false;
        }

        // Wait for the threads to compute their blocks
        // Remember that the threads are detached and are running in an infinite loop,
        // so we have to check every once in a while whether they're finished.
        while(true)
        {
            // Each thread increments the "n_running_threads" counter once it begins computing its block,
            // and decrements it once it's finished. When the counter is 0, then all threads are finished.
            if (n_running_threads == 0)
            {
                break;
            }
            //std::this_thread::sleep_for(std::chrono::microseconds(sleep_duration));
        }

        // Each thread has its own buffer. At this point, all threads have written
        // its computed audio block into it.
        // Now we have to mix (sum) these blocks into the output buffer.
        for(int i = 0; i < samples_per_block; i++)
        {
            buffer[i] = 0;
            for(int idx_thread = 0; idx_thread < N_THREADS; idx_thread++)
            {
                buffer[i] += gain*(buffers[idx_thread][i]);
            }
        }
    }
    void init_threads()
    {
        // Allocate and initialize the arrays of flags (one for each thread)

        // Flags that keep each thread running inside its infinite "while" loop
        thr_running = (std::atomic<bool>*)malloc(sizeof(std::atomic<bool>)*N_THREADS);
        for(int idx_thread = 0; idx_thread < N_THREADS; idx_thread++)
        {
            thr_running[idx_thread] = true;
        }

        // Flags used to signal to the thread that a new block has to be computed
        thr_waiting_for_block = (std::atomic<bool>*)malloc(sizeof(std::atomic<bool>)*N_THREADS);
        for(int idx_thread = 0; idx_thread < N_THREADS; idx_thread++)
        {
            thr_waiting_for_block[idx_thread] = true;
        }

        // Range notes for each thread
        // TODO: to be more fair when assigning a group of notes to each thread, we could take into account that strings
        //  with lower fundamental frequencies require more computational power (they have more spatial samples)
        //  in order to distribute the computational load more evenly.
        thr_note_range = (int*)malloc(sizeof(int)*N_THREADS*2); // thr_note_range[0] -> first note of thread 0
                                                                    // thr_note_range[1] -> last note of thread 0
                                                                    // thr_note_range[2] -> first note of thread 1
                                                                    // thr_note_range[3] -> last note of thread 2
                                                                    // ... and so on
        int n_notes_per_thread = floor(N_STRINGS/N_THREADS); // If N_THREADS is not divisible by N_STRINGS, the
                                                                // remainder will go to the last thread
        for(int idx_thread = 0; idx_thread < N_THREADS; idx_thread++)
        {
            int start_note = idx_thread * n_notes_per_thread; // Start note for the current thread
            int end_note = start_note + n_notes_per_thread-1; // End note for the current thread

            thr_note_range[idx_thread*2] = start_note;

            if(idx_thread == (N_THREADS-1)) // Check if we have reached the last thread, because...
                thr_note_range[idx_thread*2+1] = LAST_NOTE; // ... the last thread gets the remainder
            else
                thr_note_range[idx_thread*2+1] = end_note;

            //std::cout << "idx_thread: " << idx_thread << std::endl;
            //std::cout << "start_note: " << thr_note_range[idx_thread*2] << std::endl;
            //std::cout << "end_note: " << thr_note_range[idx_thread*2+1] << std::endl << std::endl;
        }

        // Create "n_threads" threads and pause them
        sleep_duration = 20; // (microsecs) TODO: it should be proportional to (samples_per_block/sampling_rate) seconds
        //sleep_duration = ((samples_per_block/sample_rate)*10e6)/25; // Sleep for 1/25th of the block duration
                                            // For example: (256/48000)*10e6 --> 5333 microseconds
                                            //                divided by 10  --> 533 microseconds of sleep
                                            //                divided by 25  --> 213 microseconds of sleep
        threads = (std::thread**)malloc(sizeof(std::thread*)*N_THREADS);
        for(int idx_thread = 0; idx_thread < N_THREADS; idx_thread++)
        {
            threads[idx_thread] = new std::thread([=]
            {
                while(thr_running[idx_thread].load() == true)
                {
                    // If the thread isn't paused
                    if(thr_waiting_for_block[idx_thread].load() == false)
                    {
                        // Signal that the block is about to be computed
                        n_running_threads++;

                        // Compute the block
                        for(size_t i = 0; i < samples_per_block; i++)
                        {
                            buffers[idx_thread][i] = 0.0f;
                            for(int j = thr_note_range[idx_thread*2]; j <= thr_note_range[idx_thread*2+1]; j++)
                            {
                                buffers[idx_thread][i] += strings[j]->get_next_sample();
                            }
                        }

                        // Signal that the block has been computed
                        n_running_threads--;

                        // Go to sleep
                        thr_waiting_for_block[idx_thread] = true;
                    }
                    // If the thread is paused, sleep for "sleep_duration" microseconds
                    else
                    {
                        std::this_thread::sleep_for(std::chrono::microseconds(sleep_duration));
                    }
                }
            });
        }

        // Detach the threads
        for(int idx_thread = 0; idx_thread < N_THREADS; idx_thread++)
        {
            threads[idx_thread]->detach();
        }
    }
    void init_buffers()
    {
        // Allocate the buffers
        buffers = (float**)malloc(N_THREADS * sizeof(float*));
        for(int idx_thread = 0; idx_thread < N_THREADS; idx_thread++)
        {
            buffers[idx_thread] = (float*)malloc(this->samples_per_block * sizeof(float));
        }

        // Initialize the buffers to zeros.
        // This prevents from drilling people's ears when they change the buffer size in the plugin!
        for (int idx_thread = 0; idx_thread < N_THREADS; idx_thread++)
            for (int i = 0; i < samples_per_block; i++)
                buffers[idx_thread][i] = 0;
    }
    float get_next_sample(float gain)
    {
        float sample = 0;
        for(int i = 0; i < N_STRINGS; i++)
        {
            sample += strings[i]->get_next_sample();
        }
        return gain*sample;
    }
    void get_next_block(float* buffer, size_t length, float gain)
    {
        for(size_t i = 0; i < length; i++)
        {
            buffer[i] = this->get_next_sample(gain);
        }
    }
    void init_hammers()
    {
        hammers[A0] = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammers[A0s] = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammers[B0] = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);

        hammers[C1] = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammers[C1s] = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammers[D1] = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammers[D1s] = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammers[E1] = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammers[F1] = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammers[F1s] = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammers[G1] = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammers[G1s] = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammers[A1] = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammers[A1s] = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammers[B1] = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);

        hammers[C2] = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammers[C2s] = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammers[D2] = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammers[D2s] = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammers[E2] = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammers[F2] = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammers[F2s] = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammers[G2] = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammers[G2s] = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammers[A2] = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammers[A2s] = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammers[B2] = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);

        hammers[C3] = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammers[C3s] = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammers[D3] = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammers[D3s] = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammers[E3] = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammers[F3] = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammers[F3s] = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammers[G3] = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammers[G3s] = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammers[A3] = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammers[A3s] = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammers[B3] = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);

        hammers[C4] = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammers[C4s] = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammers[D4] = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammers[D4s] = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammers[E4] = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammers[F4] = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammers[F4s] = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammers[G4] = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammers[G4s] = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammers[A4] = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammers[A4s] = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammers[B4] = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);

        hammers[C5] = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        /*hammers[C5s] = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammers[D5] = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammers[D5s] = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammers[E5] = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammers[F5] = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammers[F5s] = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammers[G5] = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammers[G5s] = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammers[A5] = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammers[A5s] = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammers[B5] = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);

        hammers[C6] = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammers[C6s] = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammers[D6] = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammers[D6s] = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammers[E6] = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammers[F6] = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammers[F6s] = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammers[G6] = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammers[G6s] = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammers[A6] = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammers[A6s] = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammers[B6] = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);

        hammers[C7] = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammers[C7s] = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammers[D7] = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammers[D7s] = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammers[E7] = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammers[F7] = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammers[F7s] = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammers[G7] = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammers[G7s] = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammers[A7] = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammers[A7s] = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammers[B7] = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);

        hammers[C8] = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);*/
    }
    void init_strings()
    {
        strings[A0] = new PianoString(sample_rate, 27.5, 1.92, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[A0]);
        strings[A0s] = new PianoString(sample_rate, 29.14, 1.92, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[A0s]);
        strings[B0] = new PianoString(sample_rate, 30.87, 1.92, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[B0]);

        strings[C1] = new PianoString(sample_rate, 32.7, 1.92, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[C1]);
        strings[C1s] = new PianoString(sample_rate, 34.65, 1.92, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[C1s]);
        strings[D1] = new PianoString(sample_rate, 36.71, 1.92, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[D1]);
        strings[D1s] = new PianoString(sample_rate, 38.89, 1.92, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[D1s]);
        strings[E1] = new PianoString(sample_rate, 41.20, 1.92, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[E1]);
        strings[F1] = new PianoString(sample_rate, 43.65, 1.92, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[F1]);
        strings[F1s] = new PianoString(sample_rate, 46.25, 1.92, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[F1s]);
        strings[G1] = new PianoString(sample_rate, 49.00, 1.92, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[G1]);
        strings[G1s] = new PianoString(sample_rate, 51.91, 1.92, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[G1s]);
        strings[A1] = new PianoString(sample_rate, 55.00, 1.92, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[A1]);
        strings[A1s] = new PianoString(sample_rate, 58.27, 1.92, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[A1s]);
        strings[B1] = new PianoString(sample_rate, 61.74, 1.92, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[B1]);

        strings[C2] = new PianoString(sample_rate, 65.41, 1.92, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[C2]);
        strings[C2s] = new PianoString(sample_rate, 69.30, 1.92, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[C2s]);
        strings[D2] = new PianoString(sample_rate, 73.42, 1.92, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[D2]);
        strings[D2s] = new PianoString(sample_rate, 77.78, 1.92, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[D2s]);
        strings[E2] = new PianoString(sample_rate, 82.41, 1.92, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[E2]);
        strings[F2] = new PianoString(sample_rate, 87.31, 1.92, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[F2]);
        strings[F2s] = new PianoString(sample_rate, 92.50, 1.92, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[F2s]);
        strings[G2] = new PianoString(sample_rate, 98.00, 1.92, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[G2]);
        strings[G2s] = new PianoString(sample_rate, 103.83, 1.92, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[G2s]);
        strings[A2] = new PianoString(sample_rate, 110.00, 1.92, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[A2]);
        strings[A2s] = new PianoString(sample_rate, 116.54, 1.92, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[A2s]);
        strings[B2] = new PianoString(sample_rate, 123.47, 1.92, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[B2]);

        strings[C3] = new PianoString(sample_rate, 130.81, 0.96, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[C3]);
        strings[C3s] = new PianoString(sample_rate, 138.59, 0.96, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[C3s]);
        strings[D3] = new PianoString(sample_rate, 146.83, 0.96, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[D3]);
        strings[D3s] = new PianoString(sample_rate, 155.56, 0.96, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[D3s]);
        strings[E3] = new PianoString(sample_rate, 164.81, 0.96, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[E3]);
        strings[F3] = new PianoString(sample_rate, 174.61, 0.96, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[F3]);
        strings[F3s] = new PianoString(sample_rate, 185.00, 0.96, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[F3s]);
        strings[G3] = new PianoString(sample_rate, 196, 0.96, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[G3]);
        strings[G3s] = new PianoString(sample_rate, 207.65, 0.96, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[G3s]);
        strings[A3] = new PianoString(sample_rate, 220, 0.96, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[A3]);
        strings[A3s] = new PianoString(sample_rate, 233.08, 0.96, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[A3s]);
        strings[B3] = new PianoString(sample_rate, 246.94, 0.96, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[B3]);

        strings[C4] = new PianoString(sample_rate, 261.63, 0.96, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[C4]);
        strings[C4s] = new PianoString(sample_rate, 277.18, 0.96, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[C4s]);
        strings[D4] = new PianoString(sample_rate, 293.66, 0.96, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[D4]);
        strings[D4s] = new PianoString(sample_rate, 311.13, 0.96, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[D4s]);
        strings[E4] = new PianoString(sample_rate, 329.63, 0.96, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[E4]);
        strings[F4] = new PianoString(sample_rate, 349.23, 0.96, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[F4]);
        strings[F4s] = new PianoString(sample_rate, 369.99, 0.96, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[F4s]);
        strings[G4] = new PianoString(sample_rate, 392.00, 0.96, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[G4]);
        strings[G4s] = new PianoString(sample_rate, 415.30, 0.96, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[G4s]);
        strings[A4] = new PianoString(sample_rate, 440.00, 0.96, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[A4]);
        strings[A4s] = new PianoString(sample_rate, 466.16, 0.96, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[A4s]);
        strings[B4] = new PianoString(sample_rate, 493.88, 0.96, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[B4]);

        strings[C5] = new PianoString(sample_rate, 523.25, 0.96, 0.0182, 0.0008, 9e7, 0.003, 6.25e-9, hammers[C5]);
        /*strings[C5s] = new PianoString(sample_rate, 554.37, 0.96, 0.0182, 0.0008, 9e7, 0.003, 6.25e-9, hammers[C5s]);
        strings[D5] = new PianoString(sample_rate, 587.33, 0.96, 0.0182, 0.0008, 9e7, 0.003, 6.25e-9, hammers[D5]);
        strings[D5s] = new PianoString(sample_rate, 622.25, 0.96, 0.0182, 0.0008, 9e7, 0.003, 6.25e-9, hammers[D5s]);
        strings[E5] = new PianoString(sample_rate, 659.26, 0.96, 0.0182, 0.0008, 9e7, 0.003, 6.25e-9, hammers[E5]);
        strings[F5] = new PianoString(sample_rate, 698.46, 0.96, 0.0182, 0.0008, 9e7, 0.003, 6.25e-9, hammers[F5]);
        strings[F5s] = new PianoString(sample_rate, 739.99, 0.96, 0.0182, 0.0008, 9e7, 0.003, 6.25e-9, hammers[F5s]);
        strings[G5] = new PianoString(sample_rate, 783.99, 0.96, 0.0182, 0.0008, 9e7, 0.003, 6.25e-9, hammers[G5]);
        strings[G5s] = new PianoString(sample_rate, 830.61, 0.96, 0.0182, 0.0008, 9e7, 0.003, 6.25e-9, hammers[G5s]);
        strings[A5] = new PianoString(sample_rate, 880.00, 0.96, 0.0182, 0.0008, 9e7, 0.003, 6.25e-9, hammers[A5]);
        strings[A5s] = new PianoString(sample_rate, 932.33, 0.96, 0.0182, 0.0008, 9e7, 0.003, 6.25e-9, hammers[A5s]);
        strings[B5] = new PianoString(sample_rate, 987.77, 0.96, 0.0182, 0.0008, 9e7, 0.003, 6.25e-9, hammers[B5]);

        strings[C6] = new PianoString(sample_rate, 1046.50, 0.96, 0.0182, 0.0005, 9e7, 0.003, 6.25e-9, hammers[C6]);
        strings[C6s] = new PianoString(sample_rate, 1108.73, 0.96, 0.0182, 0.0005, 9e7, 0.003, 6.25e-9, hammers[C6s]);
        strings[D6] = new PianoString(sample_rate, 1174.66, 0.96, 0.0182, 0.0005, 9e7, 0.003, 6.25e-9, hammers[D6]);
        strings[D6s] = new PianoString(sample_rate, 1244.51, 0.96, 0.0182, 0.0005, 9e7, 0.003, 6.25e-9, hammers[D6s]);
        strings[E6] = new PianoString(sample_rate, 1318.51, 0.96, 0.0182, 0.0005, 9e7, 0.003, 6.25e-9, hammers[E6]);
        strings[F6] = new PianoString(sample_rate, 1396.91, 0.96, 0.0182, 0.0005, 9e7, 0.003, 6.25e-9, hammers[F6]);
        strings[F6s] = new PianoString(sample_rate, 1479.98, 0.96, 0.0182, 0.0005, 9e7, 0.003, 6.25e-9, hammers[F6s]);
        strings[G6] = new PianoString(sample_rate, 1567.98, 0.96, 0.0182, 0.0005, 9e7, 0.003, 6.25e-9, hammers[G6]);
        strings[G6s] = new PianoString(sample_rate, 1661.22, 0.96, 0.0182, 0.0005, 9e7, 0.003, 6.25e-9, hammers[G6s]);
        strings[A6] = new PianoString(sample_rate, 1760.00, 0.96, 0.0182, 0.0005, 9e7, 0.003, 6.25e-9, hammers[A6]);
        strings[A6s] = new PianoString(sample_rate, 1864.66, 0.96, 0.0182, 0.0005, 9e7, 0.003, 6.25e-9, hammers[A6s]);
        strings[B6] = new PianoString(sample_rate, 1975.53, 0.96, 0.0182, 0.0005, 9e7, 0.003, 6.25e-9, hammers[B6]);

        strings[C7] = new PianoString(sample_rate, 2093.00, 0.96, 0.0182, 0.0005, 9e7, 0.003, 6.25e-9, hammers[C7]);
        strings[C7s] = new PianoString(sample_rate, 2217.46, 0.96, 0.0182, 0.0005, 9e7, 0.003, 6.25e-9, hammers[C7s]);
        strings[D7] = new PianoString(sample_rate, 2349.32, 0.96, 0.0182, 0.0005, 9e7, 0.003, 6.25e-9, hammers[D7]);
        strings[D7s] = new PianoString(sample_rate, 2489.02, 0.96, 0.0182, 0.0005, 9e7, 0.003, 6.25e-9, hammers[D7s]);
        strings[E7] = new PianoString(sample_rate, 2637.02, 0.96, 0.0182, 0.0005, 9e7, 0.003, 6.25e-9, hammers[E7]);
        strings[F7] = new PianoString(sample_rate, 2793.83, 0.96, 0.0182, 0.0005, 9e7, 0.003, 6.25e-9, hammers[F7]);
        strings[F7s] = new PianoString(sample_rate, 2959.96, 0.96, 0.0182, 0.0005, 9e7, 0.003, 6.25e-9, hammers[F7s]);
        strings[G7] = new PianoString(sample_rate, 3135.96, 0.96, 0.0182, 0.0005, 9e7, 0.003, 6.25e-9, hammers[G7]);
        strings[G7s] = new PianoString(sample_rate, 3322.44, 0.96, 0.0182, 0.0005, 9e7, 0.003, 6.25e-9, hammers[G7s]);
        strings[A7] = new PianoString(sample_rate, 3520.00, 0.96, 0.0182, 0.0005, 9e7, 0.003, 6.25e-9, hammers[A7]);
        strings[A7s] = new PianoString(sample_rate, 3729.31, 0.96, 0.0182, 0.0005, 9e7, 0.003, 6.25e-9, hammers[A7s]);
        strings[B7] = new PianoString(sample_rate, 3951.07, 0.96, 0.0182, 0.0005, 9e7, 0.003, 6.25e-9, hammers[B7]);

        strings[C8] = new PianoString(sample_rate, 4186.01, 0.96, 0.0182, 0.0005, 9e7, 0.003, 6.25e-9, hammers[C8]);*/
    }
};

#endif // PIANO_H
