#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"

static char temp_str[100];

bool is_png(const char *filename)
{
   int ix = strrchr(filename, '.') - filename;
   return ix > 0 && strcmp(filename + ix, ".png") == 0;
}

char* add_infix(const char *filename, const char *infix)
{
   strcpy(temp_str, filename);
   int ix = strrchr(filename, '.') - filename;
   strcpy(temp_str + ix, infix);
   strcat(temp_str, ".png");
   return temp_str;
}

char *args_shift(int *argc, char ***argv)
{
    assert(*argc > 0);
    char *result = **argv;
    (*argc) -= 1;
    (*argv) += 1;
    return result;
}

float to_luminance(int r, int g, int b)
{
   float rr = r / 255.0;
   float gg = g / 255.0;
   float bb = b / 255.0;
   return 0.2126 * rr + 0.7152 * gg + 0.0722 * bb;
}

float min(float a, float b)
{
   return a < b ? a : b;
}

float max(float a, float b)
{
   return a > b ? a : b;
}

float min3(float a, float b, float c)
{
   return min(a, min(b, c));
}

float max3(float a, float b, float c)
{
   return max(a, max(b, c));
}
