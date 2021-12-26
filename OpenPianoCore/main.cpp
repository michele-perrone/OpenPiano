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
    int duration = 8; // Duration of the synthesized signal [s]
    int duration_samples = duration*Fs;
    double* sound = zeros1D(duration_samples);

    // Hit the string with the hammer with an initial hammer velocity [m/s]
    // The typical velocity range for a piano is between 1 m/s and 6 m/s
    piano.string_C2->hit(2.5);
    for(uint64_t n = 0; n < 2*Fs; n++)
    {
        sound[n] = piano.string_C2->get_next_sample();
    }

    // Hit it again. HARDER!!!!
    piano.string_C2->hit(5.5);
    for(uint64_t n = 2*Fs; n < duration_samples; n++)
    {
        sound[n] = piano.string_C2->get_next_sample();
    }

    // Save the sound to file
    char filename[] = "C2_doublehit.wav";
    String::save_to_wav(filename, sound, duration_samples, true, true);

    return 0;
}

