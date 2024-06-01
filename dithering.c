#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"
#include "image.h"

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
