#include "array_helpers.h"

void normalize(double* array, int size)
{
    double max = 0.0;
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

void print1D(double* array, int size)
{
    for (int i = 0; i < size; i++)
    {
        printf("array[%i]: %f\n", i, array[i]);
    }
}

void sum(double* array_1, double* array_2, int size)
{
    for (int i = 0; i < size; i++)
    {
        array_1[i] += array_2[i];
    }
}
