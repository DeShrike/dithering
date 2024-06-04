#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"
#include "image.h"

int dithering2x2[2][2] = {
   { 0, 2 },
   { 3, 1 }
};

int dithering3x3[3][3] = {
   { 0, 6, 3 },
   { 7, 5, 1 },
   { 4, 2, 8 }
};

int dithering4x4[4][4] = {
   { 0, 8, 2, 10 },
   { 12, 4, 14, 6 },
   { 3, 11, 1, 9 },
   { 15, 7, 13, 5 }
};

int dithering8x8[8][8] = {
   { 0, 32, 8, 40, 2, 34, 10, 42 },
   { 48, 16, 56, 24, 50, 18, 58, 26 },
   { 12, 44, 4, 36, 14, 46, 6, 38 },
   { 60, 28, 52, 20, 62, 30, 54, 22 },
   { 3, 35, 11, 43, 1, 33, 9, 41 },
   { 51, 19, 59, 27, 49, 17, 57, 25 },
   { 15, 47, 7, 39, 13, 45, 5, 37 },
   { 63, 31, 55, 23, 61, 29, 53, 21 }
};

Image ordered_dithering(Image input, int matrix_size, int matrix[matrix_size][matrix_size])
{
   Image out_image = alloc_image(input.width, input.height);

   int dx = 0;
   int dy = 0;
   int r, g, b;
   for (int y = 0; y < input.height; ++y)
   {
      for (int x = 0; x < input.width; ++x)
      {
         get_pixel(input, x, y, &r, &g, &b);
         int d = 255.0 / (matrix_size * matrix_size);

         int c = r <= (d * (matrix[dy][dx] + 1)) ? 0 : 255;

         set_pixel(out_image, x, y, c, c, c);
         dx = (dx + 1) % matrix_size;
      }

      dy = (dy + 1) % matrix_size;
      dx = 0;
   }

   return out_image;
}

Image ordered_color_dithering(Image input, int matrix_size, int matrix[matrix_size][matrix_size])
{
   Image out_image = alloc_image(input.width, input.height);

   int dx = 0;
   int dy = 0;
   int r, g, b;
   for (int y = 0; y < input.height; ++y)
   {
      for (int x = 0; x < input.width; ++x)
      {
         get_pixel(input, x, y, &r, &g, &b);

         r = r < 255 / (matrix[dy][dx] + 1) ? 0 : 255;
         g = g < 255 / (matrix[dy][dx] + 1) ? 0 : 255;
         b = b < 255 / (matrix[dy][dx] + 1) ? 0 : 255;

         set_pixel(out_image, x, y, r, g, b);
         dx = (dx + 1) % matrix_size;
      }

      dy = (dy + 1) % matrix_size;
   }

   return out_image;
}

Image floyd_steinberg_color_dithering(Image input)
{
   #define IX(x, y) ((x) + (y) * (input.width))

   Image out_image = alloc_image(input.width, input.height);

   float ratio1 = 7.0 / 16.0;
   float ratio2 = 3.0 / 16.0;
   float ratio3 = 5.0 / 16.0;
   float ratio4 = 1.0 / 16.0;
   
   float* valuesr = calloc(input.width * input.height, sizeof(float));
   float* valuesg = calloc(input.width * input.height, sizeof(float));
   float* valuesb = calloc(input.width * input.height, sizeof(float));
   int r, g, b;
   float errorr;
   float errorg;
   float errorb;

   for (int y = 0; y < input.height; ++y)
   {
      for (int x = 0; x < input.width; ++x)
      {
         get_pixel(input, x, y, &r, &g, &b);

         valuesr[IX(x, y)] += r;
         valuesg[IX(x, y)] += g;
         valuesb[IX(x, y)] += b;
         
         if (valuesr[IX(x, y)] < 255.0 / 2.0)
         {
            errorr = valuesr[IX(x, y)];
            valuesr[IX(x, y)] = 0.0;
         }
         else
         {
            errorr = valuesr[IX(x, y)] - 255.0;
            valuesr[IX(x, y)] = 255.0;
         }

         if (valuesg[IX(x, y)] < 255.0 / 2.0)
         {
            errorg = valuesg[IX(x, y)];
            valuesg[IX(x, y)] = 0.0;
         }
         else
         {
            errorg = valuesg[IX(x, y)] - 255.0;
            valuesg[IX(x, y)] = 255.0;
         }

         if (valuesb[IX(x, y)] < 255.0 / 2.0)
         {
            errorb = valuesb[IX(x, y)];
            valuesb[IX(x, y)] = 0.0;
         }
         else
         {
            errorb = valuesb[IX(x, y)] - 255.0;
            valuesb[IX(x, y)] = 255.0;
         }

         if (x < input.width - 1)
         {
            valuesr[IX(x + 1, y)] += ratio1 * errorr;
            valuesg[IX(x + 1, y)] += ratio1 * errorg;
            valuesb[IX(x + 1, y)] += ratio1 * errorb;
         }

         if (x > 0 && y < input.height - 1)
         {
            valuesr[IX(x - 1, y + 1)] += ratio2 * errorr;
            valuesg[IX(x - 1, y + 1)] += ratio2 * errorg;
            valuesb[IX(x - 1, y + 1)] += ratio2 * errorb;
         }

         if (y < input.height - 1)
         {
            valuesr[IX(x, y + 1)] += ratio3 * errorr;
            valuesg[IX(x, y + 1)] += ratio3 * errorg;
            valuesb[IX(x, y + 1)] += ratio3 * errorb;
         }

         if (x < input.width - 1 && y < input.height - 1)
         {
            valuesr[IX(x + 1, y + 1)] += ratio4 * errorr;
            valuesg[IX(x + 1, y + 1)] += ratio4 * errorg;
            valuesb[IX(x + 1, y + 1)] += ratio4 * errorb;
         }

         set_pixel(out_image, x, y, valuesr[IX(x, y)], valuesg[IX(x, y)], valuesb[IX(x, y)]);
      }
   }

   free(valuesr);
   free(valuesg);
   free(valuesb);
   return out_image;
}

Image floyd_steinberg_dithering(Image input)
{
   #define IX(x, y) ((x) + (y) * (input.width))

   Image out_image = alloc_image(input.width, input.height);

   float ratio1 = 7.0 / 16.0;
   float ratio2 = 3.0 / 16.0;
   float ratio3 = 5.0 / 16.0;
   float ratio4 = 1.0 / 16.0;
   
   float* values = calloc(input.width * input.height, sizeof(float));
   int r, g, b;
   float error;

   for (int y = 0; y < input.height; ++y)
   {
      for (int x = 0; x < input.width; ++x)
      {
         get_pixel(input, x, y, &r, &g, &b);

         values[IX(x, y)] += r;
         
         if (values[IX(x, y)] < 255.0 / 2.0)
         {
            error = values[IX(x, y)];
            values[IX(x, y)] = 0.0;
         }
         else
         {
            error = values[IX(x, y)] - 255.0;
            values[IX(x, y)] = 255.0;
         }

         if (x < input.width - 1)
         {
            values[IX(x + 1, y)] += ratio1 * error;
         }

         if (x > 0 && y < input.height - 1)
         {
            values[IX(x - 1, y + 1)] += ratio2 * error;
         }

         if (y < input.height - 1)
         {
            values[IX(x, y + 1)] += ratio3 * error;
         }

         if (x < input.width - 1 && y < input.height - 1)
         {
            values[IX(x + 1, y + 1)] += ratio4 * error;
         }

         set_pixel(out_image, x, y, values[IX(x, y)], values[IX(x, y)], values[IX(x, y)]);
      }
   }

   free(values);
   return out_image;
}

Image calc_luminance(Image input)
{
   Image out_image = alloc_image(input.width, input.height);

   int r, g, b;
   for (int x = 0; x < input.width; ++x)
   {
      for (int y = 0; y < input.height; ++y)
      {
         // int ix = x + (y * input.width);
         get_pixel(input, x, y, &r, &g, &b);
         float lum = to_luminance(r, g, b);

         set_pixel(out_image, x, y, lum * 255.0, lum * 255.0, lum * 255.0);
      }
   }

   return out_image;
}

void process(Image input, const char *input_file_path)
{
   const char *output_file_path;

   Image lum = calc_luminance(input);

   output_file_path = add_infix(input_file_path, ".lum");
   printf("Saving %s\n", output_file_path);
   write_png_file(output_file_path, lum);

   Image d2x2 = ordered_dithering(lum, 2, dithering2x2);

   output_file_path = add_infix(input_file_path, ".2x2");
   printf("Saving %s\n", output_file_path);
   write_png_file(output_file_path, d2x2);

   Image d3x3 = ordered_dithering(lum, 3, dithering3x3);

   output_file_path = add_infix(input_file_path, ".3x3");
   printf("Saving %s\n", output_file_path);
   write_png_file(output_file_path, d3x3);

   Image d4x4 = ordered_dithering(lum, 4, dithering4x4);

   output_file_path = add_infix(input_file_path, ".4x4");
   printf("Saving %s\n", output_file_path);
   write_png_file(output_file_path, d4x4);

   Image d8x8 = ordered_dithering(lum, 8, dithering8x8);

   output_file_path = add_infix(input_file_path, ".8x8");
   printf("Saving %s\n", output_file_path);
   write_png_file(output_file_path, d8x8);

   Image d2x2c = ordered_color_dithering(input, 2, dithering2x2);

   output_file_path = add_infix(input_file_path, ".2x2c");
   printf("Saving %s\n", output_file_path);
   write_png_file(output_file_path, d2x2c);

   Image fs = floyd_steinberg_dithering(lum);

   output_file_path = add_infix(input_file_path, ".fs");
   printf("Saving %s\n", output_file_path);
   write_png_file(output_file_path, fs);

   Image fsc = floyd_steinberg_color_dithering(input);

   output_file_path = add_infix(input_file_path, ".fsc");
   printf("Saving %s\n", output_file_path);
   write_png_file(output_file_path, fsc);

   free_image(fsc);
   free_image(fs);
   free_image(d2x2c);
   free_image(d8x8);
   free_image(d4x4);
   free_image(d3x3);
   free_image(d2x2);
   free_image(lum);
}

int main(int argc, char **argv)
{
   srand(time(NULL));

   const char *program = args_shift(&argc, &argv);

   if (argc <= 0)
   {
      fprintf(stderr, "Usage: %s <input.png>\n", program);
      fprintf(stderr, "ERROR: no input file is provided\n");
      return 1;
   }

   const char *input_file_path = args_shift(&argc, &argv);

   if (!is_png(input_file_path))
   {
      fprintf(stderr, "ERROR: %s is not a PNG file\n", input_file_path);
      return 1;
   }

   printf("Reading %s\n", input_file_path);
   Image input_image = read_png_file(input_file_path);
   printf("Dimensions: %d x %d\n", input_image.width, input_image.height);

   if (input_image.bit_depth != 8)
   {
      fprintf(stderr, "Only 8 bit images are supported\n");
      free_image(input_image);
      abort();
   }

   process(input_image, input_file_path);

   free_image(input_image);

   return 0;
}
