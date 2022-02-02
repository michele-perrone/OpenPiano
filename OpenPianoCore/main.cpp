#include <chrono>

#include "dr_wav.h"
#include "array_helpers.h"
#include "piano.h"

int main()
{
    // Temporal sampling parameters
    int Fs = 48000; // Sampling frequency [Hz]

    // Initialize the piano
    Piano piano(Fs);

    // Output sound init
    int duration = 30; // Duration of the synthesized signal [s]
    int duration_samples = duration*Fs;
    float* sound = (float*)malloc(duration_samples*sizeof (float));
    float* sound_oct_2 = (float*)malloc(duration_samples*sizeof (float));
    float* sound_oct_3 = (float*)malloc(duration_samples*sizeof (float));




    /**** BEGIN - get_next_block_multithreaded_OLD() test ****/

    // Hit the string with the hammer with an initial hammer velocity [m/s]
    // The typical velocity range for a piano is between 1 m/s and 6 m/s
    auto test_start = std::chrono::steady_clock::now();
    piano.strings[C2]->hit(2.5);
    piano.get_next_block_multithreaded_old(&sound[0], &sound_oct_2[0], &sound_oct_3[0], 2*Fs, 1);

    // Hit it again. HARDER!!!!
    piano.strings[C2]->hit(5.5);
    piano.get_next_block_multithreaded_old(&sound[2*Fs], &sound_oct_2[2*Fs], &sound_oct_3[2*Fs], duration_samples-2*Fs, 1);
    auto test_end = std::chrono::steady_clock::now();
    uint64_t test_get_next_block_multithreaded = std::chrono::duration_cast<std::chrono::milliseconds>(test_end-test_start).count();

    /**** END - get_next_block_multithreaded() test ****/





    /**** BEGIN - get_next_block_multithreaded_OLD() with many blocks test ****/

    int block_length = 128;
    int n_blocks = floorf(duration_samples/block_length);
    test_start = std::chrono::steady_clock::now();
    piano.strings[C2]->hit(2.5);
    for(uint64_t n = 0; n < n_blocks; n++)
    {
        piano.get_next_block_multithreaded_old(&sound[n*block_length], &sound_oct_2[n*block_length], &sound_oct_3[n*block_length], block_length, 1);
    }
    test_end = std::chrono::steady_clock::now();
    uint64_t test_2_get_next_block_multithreaded = std::chrono::duration_cast<std::chrono::milliseconds>(test_end-test_start).count();

    /**** END - get_next_block_multithreaded() with many blocks test ****/





    /**** BEGIN - get_next_block_multithreaded() with many blocks test ****/

    test_start = std::chrono::steady_clock::now();
    piano.strings[C2]->hit(2.5);
    for(uint64_t n = 0; n < n_blocks; n++)
    {
        piano.get_next_block_multithreaded(&sound[n*block_length], block_length, 1);
    }
    test_end = std::chrono::steady_clock::now();
    uint64_t test_3_get_next_block_multithreaded = std::chrono::duration_cast<std::chrono::milliseconds>(test_end-test_start).count();

    /**** END - get_next_block_multithreaded() with many blocks test ****/





    /**** BEGIN - get_next_block() test ****/

    test_start = std::chrono::steady_clock::now();
    piano.strings[C2]->hit(2.5);
    piano.get_next_block(&sound[0], 2*Fs, 1);

    // Hit it again. HARDER!!!!
    piano.strings[C2]->hit(5.5);
    piano.get_next_block(&sound[2*Fs], duration_samples-2*Fs, 1);
    test_end = std::chrono::steady_clock::now();
    uint64_t test_get_next_block = std::chrono::duration_cast<std::chrono::milliseconds>(test_end-test_start).count();

    /**** END - get_next_block() test ****/





    /**** BEGIN - get_next_sample() test ****/

    test_start = std::chrono::steady_clock::now();
    piano.strings[C2]->hit(2.5);
    for(uint64_t n = 0; n < 2*Fs; n++)
    {
        sound[n] = piano.get_next_sample(1);
    }
    piano.strings[C2]->hit(5.5);
    test_end = std::chrono::steady_clock::now();
    for(uint64_t n = 2*Fs; n < duration_samples; n++)
    {
        sound[n] = piano.get_next_sample(1);
    }
    test_end = std::chrono::steady_clock::now();
    uint64_t test_get_next_sample = std::chrono::duration_cast<std::chrono::milliseconds>(test_end-test_start).count();

    /**** END - get_next_sample() test ****/




    printf("****************** TEST RESULTS (milliseconds) ******************\n"
           "get_next_block_multithreaded_OLD(): %li\n"
           "get_next_block_multithreaded_OLD() (%i long blocks): %li\n"
           "get_next_block_multithreaded() (%i long blocks): %li\n"
           "get_next_block(): %li\n"
           "get_next_sample(): %li\n",
           test_get_next_block_multithreaded,
           block_length, test_2_get_next_block_multithreaded,
           block_length, test_3_get_next_block_multithreaded,
           test_get_next_block,
           test_get_next_sample);



    // Save the sound to file
    //char filename[] = "C2_doublehit.wav";
    //String::save_to_wav(filename, sound, duration_samples, true, false);

    free(sound);
    free(sound_oct_2);
    free(sound_oct_3);

    return 0;
}

