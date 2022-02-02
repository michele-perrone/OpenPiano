#ifndef PIANO_H
#define PIANO_H

#include <thread>
#include "string_hammer.h"
#include "thread_pool.h"

/* **************************************************** *
 * The piano class contains all the strings and hammers *
 * **************************************************** */

enum Indices
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
const int N_WHITE_KEYS = 31; // 52 for the entire range

const int MAX_BLOCK_SIZE = 4096;

struct Piano
{
    Hammer* hammers[N_STRINGS];
    String* strings[N_STRINGS];
    int sample_rate;

    thread_pool pool;
    float** buffers;

    Piano(int sample_rate)
    {
        this->sample_rate = sample_rate;

        // Initialize the audio buffers
        buffers = (float**)malloc(N_STRINGS * sizeof(float*));
        for(int i = 0; i < N_STRINGS; i++)
        {
            buffers[i] = (float*)malloc(MAX_BLOCK_SIZE * sizeof(float));
        }
        pool.sleep_duration = 1666;

        // Initialize the hammers with their physical parameters
        init_hammers();

        // Initialize the strings with its physical parameters
        init_strings();
    }
    ~Piano()
    {
        // Delete the hammers and strings
        for(int i = 0; i < N_STRINGS; i++)
        {
            delete hammers[i];
            delete strings[i];
        }

        // Free the audio buffers
        for(int i = 0; i < N_STRINGS; i++)
        {
            free(buffers[i]);
        }
        free(buffers);
    }
    void get_next_block_multithreaded(float* buffer, size_t block_length, float gain)
    {
        for(int i = 0; i < N_STRINGS; i++)
        {
            pool.push_task([=]
            {
                strings[i]->get_next_block(buffers[i], block_length, gain);
            });
        }
        pool.wait_for_tasks();

        for(int j = 0; j < block_length; j++)
        {
            buffer[j] = 0;
            for(int i = 0; i < N_STRINGS; i++)
            {
                buffer[j] += buffers[i][j];
            }
        }
    }
    float get_next_sample(float gain)
    {
        return (strings[C2]->get_next_sample()
                + strings[C2s]->get_next_sample()
                + strings[D2]->get_next_sample()
                + strings[D2s]->get_next_sample()
                + strings[E2]->get_next_sample()
                + strings[F2]->get_next_sample()
                + strings[F2s]->get_next_sample()
                + strings[G2]->get_next_sample()
                + strings[G2s]->get_next_sample()
                + strings[A2]->get_next_sample()
                + strings[A2s]->get_next_sample()
                + strings[B2]->get_next_sample()
                + strings[C3]->get_next_sample()
                + strings[C3s]->get_next_sample()
                + strings[D3]->get_next_sample()
                + strings[D3s]->get_next_sample()
                + strings[E3]->get_next_sample()
                + strings[F3]->get_next_sample()
                + strings[F3s]->get_next_sample()
                + strings[G3]->get_next_sample()
                + strings[G3s]->get_next_sample()
                + strings[A3]->get_next_sample()
                + strings[A3s]->get_next_sample()
                + strings[B3]->get_next_sample()
                + strings[C4]->get_next_sample())*gain;
    }
    void get_next_block(float* buffer, size_t length, float gain)
    {
        for(size_t i = 0; i < length; i++)
        {
            buffer[i] = get_next_sample(gain);
        }
    }
    void get_next_sample_octave_2(float* sample, float gain)
    {
        *sample = 0.0f;
        for(int i = C2; i <= B2; i++)
        {
            *sample += gain*strings[i]->get_next_sample();
        }

        /*
        *sample = (strings[C2]->get_next_sample()
                + strings[C2s]->get_next_sample()
                + strings[D2]->get_next_sample()
                + strings[D2s]->get_next_sample()
                + strings[E2]->get_next_sample()
                + strings[F2]->get_next_sample()
                + strings[F2s]->get_next_sample()
                + strings[G2]->get_next_sample()
                + strings[G2s]->get_next_sample()
                + strings[A2]->get_next_sample()
                + strings[A2s]->get_next_sample()
                + strings[B2]->get_next_sample())*gain;
*/
    }
    void get_next_sample_octave_3(float* sample, float gain)
    {
        *sample = 0.0f;
        for(int i = C3; i <= C4; i++)
        {
            *sample += gain*strings[i]->get_next_sample();
        }

        /*
        *sample = (strings[C3]->get_next_sample()
                + strings[C3s]->get_next_sample()
                + strings[D3]->get_next_sample()
                + strings[D3s]->get_next_sample()
                + strings[E3]->get_next_sample()
                + strings[F3]->get_next_sample()
                + strings[F3s]->get_next_sample()
                + strings[G3]->get_next_sample()
                + strings[G3s]->get_next_sample()
                + strings[A3]->get_next_sample()
                + strings[A3s]->get_next_sample()
                + strings[B3]->get_next_sample()
                + strings[C4]->get_next_sample())*gain;
                */
    }
    void get_next_block_octave_2(float* buffer, size_t length, float gain)
    {
        for(size_t i = 0; i < length; i++)
        {
            get_next_sample_octave_2(&buffer[i], gain);
        }
    }
    void get_next_block_octave_3(float* buffer, size_t length, float gain)
    {
        for(size_t i = 0; i < length; i++)
        {
            get_next_sample_octave_3(&buffer[i], gain);
        }
    }
    void get_next_block_multithreaded_old(float* buffer_mix, float* buffer_oct_2, float* buffer_oct_3, size_t length, float gain)
    {
        std::thread thr_oct_2 (&Piano::get_next_block_octave_2, this, buffer_oct_2, length, gain);
        std::thread thr_oct_3 (&Piano::get_next_block_octave_3, this, buffer_oct_3, length, gain);

        thr_oct_2.join();
        thr_oct_3.join();

        for(size_t i = 0; i < length; i++)
        {
            buffer_mix[i] = buffer_oct_2[i] + buffer_oct_3[i];
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
        strings[A0] = new String(sample_rate, 27.5, 1.92, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[A0]);
        strings[A0s] = new String(sample_rate, 29.14, 1.92, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[A0s]);
        strings[B0] = new String(sample_rate, 30.87, 1.92, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[B0]);

        strings[C1] = new String(sample_rate, 32.7, 1.92, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[C1]);
        strings[C1s] = new String(sample_rate, 34.65, 1.92, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[C1s]);
        strings[D1] = new String(sample_rate, 36.71, 1.92, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[D1]);
        strings[D1s] = new String(sample_rate, 38.89, 1.92, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[D1s]);
        strings[E1] = new String(sample_rate, 41.20, 1.92, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[E1]);
        strings[F1] = new String(sample_rate, 43.65, 1.92, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[F1]);
        strings[F1s] = new String(sample_rate, 46.25, 1.92, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[F1s]);
        strings[G1] = new String(sample_rate, 49.00, 1.92, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[G1]);
        strings[G1s] = new String(sample_rate, 51.91, 1.92, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[G1s]);
        strings[A1] = new String(sample_rate, 55.00, 1.92, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[A1]);
        strings[A1s] = new String(sample_rate, 58.27, 1.92, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[A1s]);
        strings[B1] = new String(sample_rate, 61.74, 1.92, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[B1]);

        strings[C2] = new String(sample_rate, 65.41, 1.92, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[C2]);
        strings[C2s] = new String(sample_rate, 69.30, 1.92, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[C2s]);
        strings[D2] = new String(sample_rate, 73.42, 1.92, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[D2]);
        strings[D2s] = new String(sample_rate, 77.78, 1.92, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[D2s]);
        strings[E2] = new String(sample_rate, 82.41, 1.92, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[E2]);
        strings[F2] = new String(sample_rate, 87.31, 1.92, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[F2]);
        strings[F2s] = new String(sample_rate, 92.50, 1.92, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[F2s]);
        strings[G2] = new String(sample_rate, 98.00, 1.92, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[G2]);
        strings[G2s] = new String(sample_rate, 103.83, 1.92, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[G2s]);
        strings[A2] = new String(sample_rate, 110.00, 1.92, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[A2]);
        strings[A2s] = new String(sample_rate, 116.54, 1.92, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[A2s]);
        strings[B2] = new String(sample_rate, 123.47, 1.92, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[B2]);

        strings[C3] = new String(sample_rate, 130.81, 0.96, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[C3]);
        strings[C3s] = new String(sample_rate, 138.59, 0.96, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[C3s]);
        strings[D3] = new String(sample_rate, 146.83, 0.96, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[D3]);
        strings[D3s] = new String(sample_rate, 155.56, 0.96, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[D3s]);
        strings[E3] = new String(sample_rate, 164.81, 0.96, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[E3]);
        strings[F3] = new String(sample_rate, 174.61, 0.96, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[F3]);
        strings[F3s] = new String(sample_rate, 185.00, 0.96, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[F3s]);
        strings[G3] = new String(sample_rate, 196, 0.96, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[G3]);
        strings[G3s] = new String(sample_rate, 207.65, 0.96, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[G3s]);
        strings[A3] = new String(sample_rate, 220, 0.96, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[A3]);
        strings[A3s] = new String(sample_rate, 233.08, 0.96, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[A3s]);
        strings[B3] = new String(sample_rate, 246.94, 0.96, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[B3]);

        strings[C4] = new String(sample_rate, 261.63, 0.96, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[C4]);
        strings[C4s] = new String(sample_rate, 277.18, 0.96, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[C4s]);
        strings[D4] = new String(sample_rate, 293.66, 0.96, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[D4]);
        strings[D4s] = new String(sample_rate, 311.13, 0.96, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[D4s]);
        strings[E4] = new String(sample_rate, 329.63, 0.96, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[E4]);
        strings[F4] = new String(sample_rate, 349.23, 0.96, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[F4]);
        strings[F4s] = new String(sample_rate, 369.99, 0.96, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[F4s]);
        strings[G4] = new String(sample_rate, 392.00, 0.96, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[G4]);
        strings[G4s] = new String(sample_rate, 415.30, 0.96, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[G4s]);
        strings[A4] = new String(sample_rate, 440.00, 0.96, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[A4]);
        strings[A4s] = new String(sample_rate, 466.16, 0.96, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[A4s]);
        strings[B4] = new String(sample_rate, 493.88, 0.96, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammers[B4]);

        strings[C5] = new String(sample_rate, 523.25, 0.96, 0.0182, 0.0008, 9e7, 0.003, 6.25e-9, hammers[C5]);
        /*strings[C5s] = new String(sample_rate, 554.37, 0.96, 0.0182, 0.0008, 9e7, 0.003, 6.25e-9, hammers[C5s]);
        strings[D5] = new String(sample_rate, 587.33, 0.96, 0.0182, 0.0008, 9e7, 0.003, 6.25e-9, hammers[D5]);
        strings[D5s] = new String(sample_rate, 622.25, 0.96, 0.0182, 0.0008, 9e7, 0.003, 6.25e-9, hammers[D5s]);
        strings[E5] = new String(sample_rate, 659.26, 0.96, 0.0182, 0.0008, 9e7, 0.003, 6.25e-9, hammers[E5]);
        strings[F5] = new String(sample_rate, 698.46, 0.96, 0.0182, 0.0008, 9e7, 0.003, 6.25e-9, hammers[F5]);
        strings[F5s] = new String(sample_rate, 739.99, 0.96, 0.0182, 0.0008, 9e7, 0.003, 6.25e-9, hammers[F5s]);
        strings[G5] = new String(sample_rate, 783.99, 0.96, 0.0182, 0.0008, 9e7, 0.003, 6.25e-9, hammers[G5]);
        strings[G5s] = new String(sample_rate, 830.61, 0.96, 0.0182, 0.0008, 9e7, 0.003, 6.25e-9, hammers[G5s]);
        strings[A5] = new String(sample_rate, 880.00, 0.96, 0.0182, 0.0008, 9e7, 0.003, 6.25e-9, hammers[A5]);
        strings[A5s] = new String(sample_rate, 932.33, 0.96, 0.0182, 0.0008, 9e7, 0.003, 6.25e-9, hammers[A5s]);
        strings[B5] = new String(sample_rate, 987.77, 0.96, 0.0182, 0.0008, 9e7, 0.003, 6.25e-9, hammers[B5]);

        strings[C6] = new String(sample_rate, 1046.50, 0.96, 0.0182, 0.0005, 9e7, 0.003, 6.25e-9, hammers[C6]);
        strings[C6s] = new String(sample_rate, 1108.73, 0.96, 0.0182, 0.0005, 9e7, 0.003, 6.25e-9, hammers[C6s]);
        strings[D6] = new String(sample_rate, 1174.66, 0.96, 0.0182, 0.0005, 9e7, 0.003, 6.25e-9, hammers[D6]);
        strings[D6s] = new String(sample_rate, 1244.51, 0.96, 0.0182, 0.0005, 9e7, 0.003, 6.25e-9, hammers[D6s]);
        strings[E6] = new String(sample_rate, 1318.51, 0.96, 0.0182, 0.0005, 9e7, 0.003, 6.25e-9, hammers[E6]);
        strings[F6] = new String(sample_rate, 1396.91, 0.96, 0.0182, 0.0005, 9e7, 0.003, 6.25e-9, hammers[F6]);
        strings[F6s] = new String(sample_rate, 1479.98, 0.96, 0.0182, 0.0005, 9e7, 0.003, 6.25e-9, hammers[F6s]);
        strings[G6] = new String(sample_rate, 1567.98, 0.96, 0.0182, 0.0005, 9e7, 0.003, 6.25e-9, hammers[G6]);
        strings[G6s] = new String(sample_rate, 1661.22, 0.96, 0.0182, 0.0005, 9e7, 0.003, 6.25e-9, hammers[G6s]);
        strings[A6] = new String(sample_rate, 1760.00, 0.96, 0.0182, 0.0005, 9e7, 0.003, 6.25e-9, hammers[A6]);
        strings[A6s] = new String(sample_rate, 1864.66, 0.96, 0.0182, 0.0005, 9e7, 0.003, 6.25e-9, hammers[A6s]);
        strings[B6] = new String(sample_rate, 1975.53, 0.96, 0.0182, 0.0005, 9e7, 0.003, 6.25e-9, hammers[B6]);

        strings[C7] = new String(sample_rate, 2093.00, 0.96, 0.0182, 0.0005, 9e7, 0.003, 6.25e-9, hammers[C7]);
        strings[C7s] = new String(sample_rate, 2217.46, 0.96, 0.0182, 0.0005, 9e7, 0.003, 6.25e-9, hammers[C7s]);
        strings[D7] = new String(sample_rate, 2349.32, 0.96, 0.0182, 0.0005, 9e7, 0.003, 6.25e-9, hammers[D7]);
        strings[D7s] = new String(sample_rate, 2489.02, 0.96, 0.0182, 0.0005, 9e7, 0.003, 6.25e-9, hammers[D7s]);
        strings[E7] = new String(sample_rate, 2637.02, 0.96, 0.0182, 0.0005, 9e7, 0.003, 6.25e-9, hammers[E7]);
        strings[F7] = new String(sample_rate, 2793.83, 0.96, 0.0182, 0.0005, 9e7, 0.003, 6.25e-9, hammers[F7]);
        strings[F7s] = new String(sample_rate, 2959.96, 0.96, 0.0182, 0.0005, 9e7, 0.003, 6.25e-9, hammers[F7s]);
        strings[G7] = new String(sample_rate, 3135.96, 0.96, 0.0182, 0.0005, 9e7, 0.003, 6.25e-9, hammers[G7]);
        strings[G7s] = new String(sample_rate, 3322.44, 0.96, 0.0182, 0.0005, 9e7, 0.003, 6.25e-9, hammers[G7s]);
        strings[A7] = new String(sample_rate, 3520.00, 0.96, 0.0182, 0.0005, 9e7, 0.003, 6.25e-9, hammers[A7]);
        strings[A7s] = new String(sample_rate, 3729.31, 0.96, 0.0182, 0.0005, 9e7, 0.003, 6.25e-9, hammers[A7s]);
        strings[B7] = new String(sample_rate, 3951.07, 0.96, 0.0182, 0.0005, 9e7, 0.003, 6.25e-9, hammers[B7]);

        strings[C8] = new String(sample_rate, 4186.01, 0.96, 0.0182, 0.0005, 9e7, 0.003, 6.25e-9, hammers[C8]);*/
    }
};

#endif // PIANO_H
