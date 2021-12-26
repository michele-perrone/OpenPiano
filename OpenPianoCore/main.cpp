#include "dr_wav.h"
#include "array_helpers.h"
#include "string_hammer.h"

int main()
{
    // Temporal sampling parameters
    int Fs = 48000; // Sampling frequency [Hz]

    // Initialize the hammer with its physical parameters
    Hammer hammer_C2(Fs, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);

    // Initialize the string with its physical parameters
    String string_C2(Fs, 65.4, 1.92, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, &hammer_C2);

    // Output sound init
    int duration = 30; // Duration of the synthesized signal [s]
    int duration_samples = duration*Fs;
    double* sound = zeros1D(duration_samples);

    // Hit the string with the hammer with an initial hammer velocity [m/s]
    // The typical velocity range for a piano is between 1 m/s and 6 m/s
    string_C2.hit(2.5);
    for(uint64_t n = 0; n < 2*Fs; n++)
    {
        sound[n] = string_C2.get_next_sample();
    }

    // Hit it again. HARDER!!!!
    string_C2.hit(5.5);
    for(uint64_t n = 2*Fs; n < duration_samples; n++)
    {
        sound[n] = string_C2.get_next_sample();
    }

    // Save the sound to file
    char filename[] = "C2_doublehit.wav";
    String::save_to_wav(filename, sound, duration_samples, true, true);

    return 0;
}

