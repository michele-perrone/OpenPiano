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

#ifndef ARRAY_HELPERS_H
#define ARRAY_HELPERS_H

#endif // ARRAY_HELPERS_H

/* ******************************************************************************** *
 * Various functions that help with the creation and manipulation of C-style arrays *
 * ******************************************************************************** */


#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__) || defined (_WIN64)
   #define _USE_MATH_DEFINES
#endif
#include <math.h>
#include <stdio.h>

double* zeros1D(int size);
double** zeros2D(int rows, int columns);
double* hanning(int length);
double mean(double **array, int dim, int idx, int start, int stop);
double mean_abs(double** array, int dim, int idx, int start, int stop);
void print1D(double* array, int size);
void normalize(float *array, int size);
void mix(float *dest, float *array_1, float *array_2, int size);

#ifdef __cplusplus
}
#endif
