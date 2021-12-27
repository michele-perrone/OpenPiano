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

    // Hit the string with the hammer with an initial hammer velocity [m/s]
    // The typical velocity range for a piano is between 1 m/s and 6 m/s
    piano.string_C2->hit(2.5);
    piano.get_next_block_multithreaded(&sound[0], &sound_oct_2[0], &sound_oct_3[0], 2*Fs, 1);
    /*for(uint64_t n = 0; n < 2*Fs; n++)
    {
        sound[n] = piano.get_next_sample();
    }*/

    // Hit it again. HARDER!!!!
    piano.string_C2->hit(5.5);
    piano.get_next_block_multithreaded(&sound[2*Fs], &sound_oct_2[2*Fs], &sound_oct_3[2*Fs], duration_samples-2*Fs, 1);
    /*for(uint64_t n = 2*Fs; n < duration_samples; n++)
    {
        sound[n] = piano.get_next_sample();
    }*/

    // Save the sound to file
    char filename[] = "C2_doublehit.wav";
    sum(sound, sound_oct_2, sound_oct_3, duration_samples);
    String::save_to_wav(filename, sound, duration_samples, true, true);

    free(sound_oct_2);
    free(sound_oct_3);

    return 0;
}

