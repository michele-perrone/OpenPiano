#ifndef PIANO_H
#define PIANO_H

#include <thread>
#include "string_hammer.h"

/* **************************************************** *
 * The piano class contains all the strings and hammers *
 * **************************************************** */

struct Piano
{
    Hammer* hammer_C2;
    Hammer* hammer_C2s;
    Hammer* hammer_D2;
    Hammer* hammer_D2s;
    Hammer* hammer_E2;
    Hammer* hammer_F2;
    Hammer* hammer_F2s;
    Hammer* hammer_G2;
    Hammer* hammer_G2s;
    Hammer* hammer_A2;
    Hammer* hammer_A2s;
    Hammer* hammer_B2;
    Hammer* hammer_C3;
    Hammer* hammer_C3s;
    Hammer* hammer_D3;
    Hammer* hammer_D3s;
    Hammer* hammer_E3;
    Hammer* hammer_F3;
    Hammer* hammer_F3s;
    Hammer* hammer_G3;
    Hammer* hammer_G3s;
    Hammer* hammer_A3;
    Hammer* hammer_A3s;
    Hammer* hammer_B3;
    Hammer* hammer_C4;


    String* string_C2;
    String* string_C2s;
    String* string_D2;
    String* string_D2s;
    String* string_E2;
    String* string_F2;
    String* string_F2s;
    String* string_G2;
    String* string_G2s;
    String* string_A2;
    String* string_A2s;
    String* string_B2;
    String* string_C3;
    String* string_C3s;
    String* string_D3;
    String* string_D3s;
    String* string_E3;
    String* string_F3;
    String* string_F3s;
    String* string_G3;
    String* string_G3s;
    String* string_A3;
    String* string_A3s;
    String* string_B3;
    String* string_C4;

    Piano(int sample_rate)
    {
        // Initialize the hammers with their physical parameters
        hammer_C2 = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammer_C2s = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammer_D2 = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammer_D2s = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammer_E2 = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammer_F2 = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammer_F2s = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammer_G2 = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammer_G2s = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammer_A2 = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammer_A2s = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammer_B2 = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammer_C3 = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammer_C3s = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammer_D3 = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammer_D3s = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammer_E3 = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammer_F3 = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammer_F3s = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammer_G3 = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammer_G3s = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammer_A3 = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammer_A3s = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammer_B3 = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
        hammer_C4 = new Hammer(sample_rate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);

        // Initialize the strings with its physical parameters
        string_C2 = new String(sample_rate, 65.41, 1.92, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammer_C2);
        string_C2s = new String(sample_rate, 69.30, 1.92, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammer_C2s);
        string_D2 = new String(sample_rate, 73.42, 1.92, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammer_D2);
        string_D2s = new String(sample_rate, 77.78, 1.92, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammer_D2s);
        string_E2 = new String(sample_rate, 82.41, 1.92, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammer_E2);
        string_F2 = new String(sample_rate, 87.31, 1.92, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammer_F2);
        string_F2s = new String(sample_rate, 92.50, 1.92, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammer_F2s);
        string_G2 = new String(sample_rate, 98.00, 1.92, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammer_G2);
        string_G2s = new String(sample_rate, 103.83, 1.92, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammer_G2s);
        string_A2 = new String(sample_rate, 110.00, 1.92, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammer_A2);
        string_A2s = new String(sample_rate, 116.54, 1.92, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammer_A2s);
        string_B2 = new String(sample_rate, 123.47, 1.92, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammer_B2);
        string_C3 = new String(sample_rate, 130.81, 0.96, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammer_C3);
        string_C3s = new String(sample_rate, 138.59, 0.96, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammer_C3s);
        string_D3 = new String(sample_rate, 146.83, 0.96, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammer_D3);
        string_D3s = new String(sample_rate, 155.56, 0.96, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammer_D3s);
        string_E3 = new String(sample_rate, 164.81, 0.96, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammer_E3);
        string_F3 = new String(sample_rate, 174.61, 0.96, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammer_F3);
        string_F3s = new String(sample_rate, 185.00, 0.96, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammer_F3s);
        string_G3 = new String(sample_rate, 196, 0.96, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammer_G3);
        string_G3s = new String(sample_rate, 207.65, 0.96, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammer_G3s);
        string_A3 = new String(sample_rate, 220, 0.96, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammer_A3);
        string_A3s = new String(sample_rate, 233.08, 0.96, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammer_A3s);
        string_B3 = new String(sample_rate, 246.94, 0.96, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammer_B3);
        string_C4 = new String(sample_rate, 261.63, 0.96, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammer_C4);
    }
    float get_next_sample(float gain)
    {
        return (string_C2->get_next_sample()
                + string_C2s->get_next_sample()
                + string_D2->get_next_sample()
                + string_D2s->get_next_sample()
                + string_E2->get_next_sample()
                + string_F2->get_next_sample()
                + string_F2s->get_next_sample()
                + string_G2->get_next_sample()
                + string_G2s->get_next_sample()
                + string_A2->get_next_sample()
                + string_A2s->get_next_sample()
                + string_B2->get_next_sample()
                + string_C3->get_next_sample()
                + string_C3s->get_next_sample()
                + string_D3->get_next_sample()
                + string_D3s->get_next_sample()
                + string_E3->get_next_sample()
                + string_F3->get_next_sample()
                + string_F3s->get_next_sample()
                + string_G3->get_next_sample()
                + string_G3s->get_next_sample()
                + string_A3->get_next_sample()
                + string_A3s->get_next_sample()
                + string_B3->get_next_sample()
                + string_C4->get_next_sample())*gain;
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
        *sample = (string_C2->get_next_sample()
                + string_C2s->get_next_sample()
                + string_D2->get_next_sample()
                + string_D2s->get_next_sample()
                + string_E2->get_next_sample()
                + string_F2->get_next_sample()
                + string_F2s->get_next_sample()
                + string_G2->get_next_sample()
                + string_G2s->get_next_sample()
                + string_A2->get_next_sample()
                + string_A2s->get_next_sample()
                + string_B2->get_next_sample())*gain;
    }
    void get_next_sample_octave_3(float* sample, float gain)
    {
        *sample = (string_C3->get_next_sample()
                + string_C3s->get_next_sample()
                + string_D3->get_next_sample()
                + string_D3s->get_next_sample()
                + string_E3->get_next_sample()
                + string_F3->get_next_sample()
                + string_F3s->get_next_sample()
                + string_G3->get_next_sample()
                + string_G3s->get_next_sample()
                + string_A3->get_next_sample()
                + string_A3s->get_next_sample()
                + string_B3->get_next_sample()
                + string_C4->get_next_sample())*gain;
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
    void get_next_block_multithreaded(float* buffer_mix, float* buffer_oct_2, float* buffer_oct_3, size_t length, float gain)
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
    ~Piano()
    {
        delete hammer_C2;
        delete hammer_C2s;
        delete hammer_D2;
        delete hammer_D2s;
        delete hammer_E2;
        delete hammer_F2;
        delete hammer_F2s;
        delete hammer_G2;
        delete hammer_G2s;
        delete hammer_A2;
        delete hammer_A2s;
        delete hammer_B2;
        delete hammer_C3;
        delete hammer_C3s;
        delete hammer_D3;
        delete hammer_D3s;
        delete hammer_E3;
        delete hammer_F3;
        delete hammer_F3s;
        delete hammer_G3;
        delete hammer_G3s;
        delete hammer_A3;
        delete hammer_A3s;
        delete hammer_B3;
        delete hammer_C4;


        delete string_C2;
        delete string_C2s;
        delete string_D2;
        delete string_D2s;
        delete string_E2;
        delete string_F2;
        delete string_F2s;
        delete string_G2;
        delete string_G2s;
        delete string_A2;
        delete string_A2s;
        delete string_B2;
        delete string_C3;
        delete string_C3s;
        delete string_D3;
        delete string_D3s;
        delete string_E3;
        delete string_F3;
        delete string_F3s;
        delete string_G3;
        delete string_G3s;
        delete string_A3;
        delete string_A3s;
        delete string_B3;
        delete string_C4;
    }
};

#endif // PIANO_H
