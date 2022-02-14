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

#include <chrono>

#include "dr_wav.h"
#include "array_helpers.h"
#include "piano.h"

int main()
{
    // Temporal sampling parameters
    int Fs = 48000; // Sampling frequency [Hz]
    int samples_per_block = 256;
    int n_threads = std::thread::hardware_concurrency();

    // Initialize the piano
    Piano piano(Fs, samples_per_block, n_threads);

    // Output sound init
    uint32_t duration = 300; // Duration of the synthesized signal [s]
    int duration_samples = duration*Fs;
    int n_blocks = floorf(duration_samples/samples_per_block);
    float* sound = (float*)malloc(duration_samples*sizeof (float));



    /**** BEGIN - get_next_block_multithreaded() with many blocks test ****/

    auto test_start = std::chrono::steady_clock::now();

    piano.strings[C2]->hit(2.5);
    for(uint64_t n = 0; n < n_blocks; n++)
    {
        piano.get_next_block_multithreaded(&sound[n*samples_per_block], samples_per_block, 1);
    }
    while(piano.n_running_threads != 0) {}
    auto test_end = std::chrono::steady_clock::now();

    uint64_t test_2_get_next_block_multithreaded = std::chrono::duration_cast<std::chrono::milliseconds>(test_end-test_start).count();

    /**** END - get_next_block_multithreaded() with many blocks test ****/





    /**** BEGIN - get_next_block() test ****/

    test_start = std::chrono::steady_clock::now();

    piano.strings[C2]->hit(2.5);
    for(uint64_t n = 0; n < n_blocks; n++)
    {
        piano.get_next_block(&sound[n*samples_per_block], samples_per_block, 1);
    }

    test_end = std::chrono::steady_clock::now();
    uint64_t test_3_get_next_block = std::chrono::duration_cast<std::chrono::milliseconds>(test_end-test_start).count();

    /**** END - get_next_block() test ****/





    /**** BEGIN - get_next_sample() test ****/

    test_start = std::chrono::steady_clock::now();

    piano.strings[C2]->hit(2.5);
    for(uint64_t n = 0; n < duration_samples; n++)
    {
        sound[n] = piano.get_next_sample(1);
    }

    test_end = std::chrono::steady_clock::now();
    uint64_t test_4_get_next_sample = std::chrono::duration_cast<std::chrono::milliseconds>(test_end-test_start).count();

    /**** END - get_next_sample() test ****/




    printf("****************** TEST RESULTS (milliseconds) ******************\n"
           "*************** Benchmark for %i seconds of sound ***************\n"
           "get_next_block_multithreaded() (%i long blocks): %li\n"
           "get_next_block(): %li\n"
           "get_next_sample(): %li\n",
           duration,
           samples_per_block, test_2_get_next_block_multithreaded,
           test_3_get_next_block,
           test_4_get_next_sample
          );



    // Save the sound to file
    //char filename[] = "C2_doublehit.wav";
    //String::save_to_wav(filename, sound, duration_samples, true, false);

    free(sound);

    return 0;
}

