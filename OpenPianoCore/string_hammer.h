#ifndef STRING_HAMMER_H
#define STRING_HAMMER_H

#endif // STRING_HAMMER_H

/* *************************************************************** *
 * Implementation of the physical model for the string and hammer. *
 * It makes sense placing them inside the same source file because *
 * of their strong interconnection.                                *
 * *************************************************************** */

#include <stdio.h>
#include <string.h>
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__) || defined (_WIN64)
   #define _USE_MATH_DEFINES
#endif
#include <math.h>
#include <inttypes.h>

#include "dr_wav.h"
#include "array_helpers.h"

struct Hammer
{
    // Sampling frequency and period
    int Fs;
    double Ts;

    // Hammer parameters
    double Mh; // Total mass [kg]
    double p; // Stiffness nonlinear exponent
    double bH; // FD parameter
    double K; // Stiffness [N/m]

    // Central contact point btw string and hammer
    double a; // Normalized in the range (0,1]
    double x_contact; // In meters
    int Xs_contact; // In samples
    double Xs;
    int i;

    // Hammer felt parameters
    double d1;
    double d2;
    double dF;

    // Hammer contact window definition
    double g_meters; // hammer length [m]
    double g; //hammer_length in samples
    double* hammer_win;
    double* hammer_mask;

    // Hammer displacement and force over time
    double* eta;
    double* Fh;

    Hammer(int Fs, double Mh, double p, double bH, double K, double a, double g_meters)
    {
        this->Fs = Fs;
        this->Ts = 1./Fs;

        this->Mh = Mh;
        this->p = p;
        this->bH = bH;
        this->K = K;
        this->a = a;

        // Hammer felt parameters
        this->d1 = 2/(1+bH*Ts/(2*Mh));
        this->d2 = (-1+bH*Ts/(2*Mh))/(1+bH*Ts/(2*Mh));
        this->dF = (-powf(Ts,2)/Mh)/(1+bH*Ts/(2*Mh));

        // Hammer contact window definition
        this->g_meters = g_meters;

        // Fields initialized by the string
        x_contact = 0;
        Xs_contact = 0;
        Xs = 0;
        i = 0;
        g = 0;
        hammer_win = nullptr;
        hammer_mask = NULL;
        eta = NULL;
        Fh = NULL;
    }
    ~Hammer()
    {
        free(hammer_win);
        free(hammer_mask);
        free(eta);
        free(Fh);
    }
};

struct String
{
    // Hammer that hits this string
    Hammer* h;

    // String displacement over time and space
    double** y;

    // Sampling frequency and period
    int Fs;
    double Ts;

    // These values are given
    double f0; // Fund. frequency [Hz]
    double L; // Total length [m]
    double rho; // Linear density [kg/m]
    double S; // Cross-sectional area of the string [m]
    double E; // Young's modulus [N/m^2]
    double b1; // First damping coefficient
    double b2; // Second damping coefficient

    // These values are calculated
    double Ms; // Total mass [kg]
    double Te; // Tension [N]
    double c; // Propagation velocity [m/s]
    double r_gyr; // Radius of gyration of the string [m]
    double eps; // Eq. 2, stiffness parameter

    // Spatial sampling aliasing condition (number of maximum spatial steps)
    double gamma;
    int N;
    int len_x_axis;
    double* x_axis;

    // FD parameters
    double courant_num;
    double lambda;
    double mu;

    // PDE Coefficients (Chaigne's article)
    double D;
    double r;
    double a1;
    double a2;
    double a3;
    double a4;
    double a5;

    // Boundary parameters
    double zeta_b; // Normalized impedance of the bridge
    double zeta_l; // Normalized impedance of the left boundary

    // Bridge boundary coefficients (case m=0, m=1)
    double b_R1;
    double b_R2;
    double b_R3;
    double b_R4;
    double b_RF;

    // Left hand (hinged string end) boundary coefficients (case m;M-1, m=M)
    double b_L1;
    double b_L2;
    double b_L3;
    double b_L4;
    double b_LF;

    // Parameters for spatio-temporal simulation scheme
    int buffer_size; // 4 samples is the absolute minimum
    uint64_t n; // Sample counter for the string simulation
    uint8_t n_0;
    uint8_t n_1;
    uint8_t n_2;
    uint8_t n_3;

    // Parameters for the calculation of the sound
    int N_space_samples; // Must be even in order to be centered around something
    int Xs_sound;
    int left_boundary;
    int right_boundary;

    // Methods
    String(int Fs, double f0, double L, double rho, double S, double E, double b1, double b2, Hammer * h)
    {
        // Sampling frequency and period
        this->Fs = Fs;
        this->Ts = 1./Fs;

        // Hammer hitting this string
        this->h = h;

        // Assign the parameters
        this->f0 = f0;
        this->L = L;
        this->rho = rho;
        this->S = S;
        this->E = E;
        this->b1 = b1;
        this->b2 = b2;

        // Calculate the remaining physical parameters
        this->Ms = this->rho*this->L;
        this->Te = this->rho*powf(this->L,2)*4*powf(this->f0,2);
        this->c = sqrt(this->Te/this->rho);
        this->r_gyr = this->S/2;
        this->eps = powf(this->r_gyr,2) * ( (this->E*this->S) / (this->Te*powf(this->L,2)) );

        // Calculate the maximum number of spatial samples
        this->gamma = Fs/(2*this->f0); // Eq. 12
        this->N = floor( sqrt((-1+sqrt(1+16*eps*powf(gamma,2)))/(8*eps)) ); // Eq. 11
        this->len_x_axis = N;
        this->x_axis = zeros1D(N);
        for(int i = 1; i<len_x_axis; i++)
        {
            x_axis[i] = x_axis[i-1]+(this->L/this->N);
        }

        // Calculate the contact points on the string that are hit by the hammer
        this->h->Xs = this->L/this->N;
        this->h->x_contact = this->h->a*this->L;
        this->h->Xs_contact = round(this->h->x_contact/this->h->Xs);
        this->h->g = ceil(this->h->g_meters*this->N/this->L); //hammer_length in samples
        this->h->hammer_win = hanning(this->h->g);
        this->h->hammer_mask = zeros1D(this->len_x_axis);
        this->h->i = floorf(this->h->Xs_contact-(this->h->g/2)) + 1;
        memcpy(&this->h->hammer_mask[this->h->i], &this->h->hammer_win[0], this->h->g*sizeof(double*));

        // FD parameters
        courant_num = c*Ts/this->h->Xs;
        lambda = courant_num;
        mu = powf(eps,2)/(powf(c,2)*powf(this->h->Xs,2));

        // PDE Coefficients (Chaigne's article)
        this->D = 1 + b1*this->h->Xs +2*b2/Ts;
        this->r = c*Ts/this->h->Xs;
        this->a1 = (2 - 2*powf(r,2) + b2/Ts - 6*eps*powf(N,2)*powf(r,2))/D;
        this->a2 = (-1 + b1*Ts + 2*b2/Ts)/D;
        this->a3 = (powf(r,2) * (1+4*eps*powf(N,2)))/D;
        this->a4 = (b2/Ts - eps*powf(N,2)*powf(r,2))/D;
        this->a5 = (-b2/Ts)/D;

        // Boundary parameters
        this->zeta_b = 1e03; // Normalized impedance of the bridge
        this->zeta_l = 1e20; // Normalized impedance of the left boundary

        // Bridge boundary coefficients (case m=0, m=1)
        this->b_R1 = (2-2*powf(lambda,2)*mu-2*powf(lambda,2))/(1+b1*Ts+zeta_b*lambda);
        this->b_R2 = (4*powf(lambda,2)*mu+2*powf(lambda,2))/(1+b1*Ts+zeta_b*lambda);
        this->b_R3 = (-2*powf(lambda,2)*mu)/(1+b1*Ts+zeta_b*lambda);
        this->b_R4 = (-1-b1*Ts+zeta_b*lambda)/(1+b1*Ts+zeta_b*lambda);
        this->b_RF = (powf(Ts,2)/rho)/(1+b1*Ts+zeta_b*lambda);

        // Left hand (hinged string end) boundary coefficients (case m = M-1, m=M)
        this->b_L1 = (2-2*powf(lambda,2)*mu-2*powf(lambda,2))/(1+b1*Ts+zeta_l*lambda);
        this->b_L2 = (4*powf(lambda,2)*mu+2*powf(lambda,2))/(1+b1*Ts+zeta_l*lambda);
        this->b_L3 = (-2*powf(lambda,2)*mu)/(1+b1*Ts+zeta_l*lambda);
        this->b_L4 = (-1-b1*Ts+zeta_l*lambda)/(1+b1*Ts+zeta_l*lambda);
        this->b_LF = (powf(Ts,2)/rho)/(1+b1*Ts+zeta_l*lambda);

        // Parameters for the spatio-temporal simulation scheme
        //this->n = -1; // [CONSIDER DEPRECATING] This counter will be incremented at every temporal step
        this->buffer_size = 4; // This is the length of the circular temporal buffer.
                               // The finite-difference equation needs 3 previous time steps
                               // in order to calculate the current time step.
                               // Therefore, the minimum length of the buffer is 4.
        // The indices below will be updated at each time step.
        // In order to simulate a circular buffer, they will be
        // incremented and and only the two right-most bits will be
        // kept. This eliminates the need of a global counter and
        // should be more efficient than computing each the division
        // remainder between the counter and the buffer size.
        this->n_0 = 3; // Current time instant n
        this->n_1 = 2; // Previous time instant n-1
        this->n_2 = 1; // Previous time instant n-2
        this->n_3 = 0; // Previous time instant n-3

        // Array definition
        this->y = zeros2D(len_x_axis+2, buffer_size);
        this->h->eta = zeros1D(buffer_size); // Hammer displacement over time
        this->h->Fh = zeros1D(buffer_size); // Force imparted by the hammer on the string over time

        // Parameters for extrapolating the sound of the string
        this->N_space_samples = 13; // Must be even in order to be centered around something
        this->Xs_sound = this->N - this->h->Xs_contact;
        this->left_boundary = Xs_sound-(N_space_samples-1)/2;
        this->right_boundary = Xs_sound+(N_space_samples-1)/2;
    }
    ~String()
    {
        free(x_axis);
        for(int i = 0; i < len_x_axis+2; i++)
        {
            free(y[i]);
        }
        free(y);
    }
    void hit(double V_h0)
    {
        // Initial conditions: compute the displacement for the first three time samples

        // Time instant n=0
        n_0 = (n_0+1)&0x3;
        n_1 = (n_1+1)&0x3;
        n_2 = (n_2+1)&0x3;
        n_3 = (n_3+1)&0x3;

        /******************************* AHEM! *********************************
         * Resetting the entire string displacement to zero each time it's hit
         * by the hammer does not sound intuitively right! The problem is,
         * if I don't do it the second time I hit string, everything EXPLODES */
        for(int i = 0; i < len_x_axis; i++)
        {
            y[i][n_0] = 0.0f; // Eq. 14
        }
        h->eta[n_0] = 0.0f;
        h->Fh[n_0] = 0.0f;

        // Time instant n=1
        n_0 = (n_0+1)&0x3;
        n_1 = (n_1+1)&0x3;
        n_2 = (n_2+1)&0x3;
        n_3 = (n_3+1)&0x3;

        for (int i = 2; i < len_x_axis-3; i++)
        {
            y[i][n_0] = (y[i-1][n_1] + y[i+1][n_1])*0.5f;
        }
        h->eta[n_0] = V_h0 * Ts; // Eq. 15
        h->Fh[n_0] = h->K*powf((h->eta[n_0] - y[h->Xs_contact][n_0]), h->p); // Eq. 17

        // Time instant n=2
        n_0 = (n_0+1)&0x3;
        n_1 = (n_1+1)&0x3;
        n_2 = (n_2+1)&0x3;
        n_3 = (n_3+1)&0x3;

        for (int i = 2; i < len_x_axis-3; i++)
        {
            y[i][n_0] = y[i-1][n_1]
                    + y[i+1][n_1] -y[i][n_2]
                    + (powf(Ts,2.0f)*N*h->Fh[n_0]*h->hammer_mask[i])/h->Mh; // Eq. 18
        }
        h->eta[n_0] = h->d1*h->eta[n_1] + h->d2*h->eta[n_2] - (powf(Ts,2.0f)*h->Fh[n_1])/h->Mh; // Eq. 19
        h->Fh[n_0] = h->K*(powf(h->eta[n_0] - y[h->Xs_contact][n_0], h->p)); // Eq. 20
    }
    double get_next_sample()
    {
        // For n>=3, compute:
        n_0 = (n_0+1)&0x3;
        n_1 = (n_1+1)&0x3;
        n_2 = (n_2+1)&0x3;
        n_3 = (n_3+1)&0x3;

        // - the string displacement  y(i,n)
        //   (spatial sampling loop, Eq. 10)
        for (int i = 2; i< len_x_axis-3; i++)
        {
            y[i][n_0] = a1*y[i][n_1] + a2*y[i][n_2]
                    + a3*(y[i+1][n_1] + y[i-1][n_1])
                    + a4*(y[i+2][n_1] + y[i-2][n_1])
                    + a5*(y[i+1][n_2] + y[i-1][n_2] + y[i][n_3])
                    + (powf(Ts,2.0f)*N*h->Fh[n_1]*h->hammer_mask[i])/Ms;
        }

        // - boundary conditions (Chaigne, % Eq. 23)
        int end = len_x_axis+1;
        y[0][n_0] = -y[2][n_0]; // Left boundary
        y[end][n_0] = -y[end-2][n_0]; // b) Bridge boundary

        // - boundary conditions (Saitis, // Eq. 4.18 and Eq. 4.20)
        //   a) left boundary (frame) // 4.20
        //y[0][n] = b_L1*y[0][n-1] + b_L2*y[1][n-1] + b_L3*y[2][n-1]
        //    + b_L4*y[0][n-2] + b_LF*h->Fh[n-1]*h->hammer_mask[i];
        //   b) right boundary (bridge) // Eq. 4.18
        //int end = len_x_axis;
        //y[end][n] = b_R1*y[end][n-1] + b_R2*y[end-1][n-1]
        //    + b_R3*y[end-2][n-1] + b_R4*y[end][n-2] + b_RF*h->Fh[n-1]*h->hammer_mask[i];

        double current_sample = mean(this->y, 1, n_0, left_boundary, right_boundary); //y[left_boundary][n_0];

        // - the hammer displacement eta(n)
        h->eta[n_0] = h->d1*h->eta[n_1] + h->d2*h->eta[n_2] + h->dF*h->Fh[n_1]; // (Saitis, // 4.21)

        // - the hammer force Fh(n)
        // if the condition in Eq. 21 is met, the force term is removed
        if (h->eta[n_0] < y[h->Xs_contact][n_0]) // Eq. 21
            h->Fh[n_0] = 0.0f; // Hammer not in contact with string -> force is 0
        else
            h->Fh[n_0] = h->K*powf(h->eta[n_0]-y[h->Xs_contact][n_0], h->p); // Eq. 20

        return current_sample;
    }
    static drwav_uint64 save_to_wav(char* filename, double* sound, uint64_t duration_samples, bool normalize_output, bool destroy)
    {
        if(normalize_output)
        {
            normalize(sound, duration_samples);
        }

        drwav wav;
        drwav_data_format format;
        format.container = drwav_container_riff; // <-- drwav_container_riff = normal WAV files, drwav_container_w64 = Sony Wave64.
        format.format = DR_WAVE_FORMAT_IEEE_FLOAT; // <-- Any of the DR_WAVE_FORMAT_* codes.
        format.channels = 1;
        format.sampleRate = 48000;
        format.bitsPerSample = sizeof (double)*8;
        drwav_init_file_write(&wav, filename, &format, NULL);
        drwav_uint64 framesWritten = drwav_write_pcm_frames(&wav, duration_samples, sound);
        drwav_uninit(&wav);
        if(destroy)
        {
            free(sound);
        }

        return framesWritten;
    }
};
