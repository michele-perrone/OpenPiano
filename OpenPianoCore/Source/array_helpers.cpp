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

#include "array_helpers.h"

void normalize(float* array, int size)
{
    float max = 0.0;
    for (int n = 0; n < size; n++)
    {
        if(array[n] > max)
        {
            max = array[n];
        }
    }
    for (int n = 0; n < size; n++)
    {
        array[n] /= max;
    }
}

double* hanning(int length)
{
    double* output = (double*)malloc(sizeof (double) * length);
    for (int i = 0; i < length; i++)
    {
        output[i] = 0.5 * (1 - cos(2*M_PI*i/(length-1)));
    }
    return output;
}

double** zeros2D(int rows, int columns)
{

    double** output = (double**)malloc(rows * sizeof(double*));
    for (int i = 0; i < rows; i++)
        output[i] = (double*)malloc(columns * sizeof(double));

    for (int i = 0; i < rows; i++)
        for (int j = 0; j < columns; j++)
            output[i][j] = 0;

    return output;
}

double* zeros1D(int size)
{

    double* output = (double*)malloc(size * sizeof(double));
    for (int i = 0; i < size; i++)
        output[i] = 0;

    return output;
}


double mean(double** array, int dim, int idx, int start, int stop)
{
    // This function computes the mean of a 2D array over a single dimension.

    // dim == 0 -> Compute the mean of the elements along one ROW
    // dim == 1 -> Compute the mean of the elements along one COLUMN

    double sum = 0;

    if(dim == 0)
    {
        for (int i = start; i < stop; i++)
        {
            sum += array[idx][i];
        }
    }
    else if(dim == 1)
    {
        for (int i = start; i < stop; i++)
        {
            sum += array[i][idx];
        }
    }

    return sum/(stop-start);
}

double mean_abs(double** array, int dim, int idx, int start, int stop)
{
    // This function computes the mean of the absolute values of a 2D array
    // over a single dimension.

    // dim == 0 -> Compute the mean of the elements along one ROW
    // dim == 1 -> Compute the mean of the elements along one COLUMN

    double sum = 0;

    if(dim == 0)
    {
        for (int i = start; i < stop; i++)
        {
            sum += fabs(array[idx][i]);
        }
    }
    else if(dim == 1)
    {
        for (int i = start; i < stop; i++)
        {
            sum += fabs(array[i][idx]);
        }
    }

    return sum/(stop-start);
}

void print1D(double* array, int size)
{
    for (int i = 0; i < size; i++)
    {
        printf("array[%i]: %f\n", i, array[i]);
    }
}

void mix(float* dest, float* array_1, float* array_2, int size)
{
    for (int i = 0; i < size; i++)
    {
        dest[i] = array_1[i] + array_2[i];
    }
}
