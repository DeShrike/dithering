#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include "png.h"
#include "image.h"

Image alloc_image(int width, int height)
{
	Image result;
	result.width = width;
	result.height = height;
	result.color_type = PNG_COLOR_TYPE_RGBA;
	result.bit_depth = 8;
	result.row_pointers = NULL;

	result.row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * result.height);
	for (int y = 0; y < result.height; y++)
	{
		result.row_pointers[y] = (png_byte*)malloc( result.width * 4 );
	}

	return result;
}

void free_image(Image image)
{
	for (int y = 0; y < image.height; y++)
	{
		free(image.row_pointers[y]);
	}

	free(image.row_pointers);
	image.row_pointers = NULL;
}

void read_png_version_info()
{
    fprintf(stderr, "Compiled with libpng %s; using libpng %s.\n", PNG_LIBPNG_VER_STRING, png_libpng_ver);
}

Image read_png_file(const char *filename)
{
	Image result = { 0 };

	FILE *fp = fopen(filename, "rb");
	if (fp == NULL)
	{
		fprintf(stderr, "Could not open file %s: %s\n", filename, strerror(errno));
		abort();
	}

	png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png) abort();

	png_infop info = png_create_info_struct(png);
	if (!info) abort();

	if (setjmp(png_jmpbuf(png))) abort();

	png_init_io(png, fp);

	png_read_info(png, info);

	result.width      = png_get_image_width(png, info);
	result.height     = png_get_image_height(png, info);
	result.color_type = png_get_color_type(png, info);
	result.bit_depth  = png_get_bit_depth(png, info);

	// Read any color_type into 8bit depth, RGBA format.
	// See http://www.libpng.org/pub/png/libpng-manual.txt

	if (result.bit_depth == 16)
	{
    	png_set_strip_16(png);
	}

	if (result.color_type == PNG_COLOR_TYPE_PALETTE)
	{
    	png_set_palette_to_rgb(png);
	}

	// PNG_COLOR_TYPE_GRAY_ALPHA is always 8 or 16bit depth.
	if (result.color_type == PNG_COLOR_TYPE_GRAY && result.bit_depth < 8)
	{
	    png_set_expand_gray_1_2_4_to_8(png);
	}

	if (png_get_valid(png, info, PNG_INFO_tRNS))
	{
		png_set_tRNS_to_alpha(png);
	}

	// These color_type don't have an alpha channel then fill it with 0xff.
	if (result.color_type == PNG_COLOR_TYPE_RGB ||
        result.color_type == PNG_COLOR_TYPE_GRAY ||
        result.color_type == PNG_COLOR_TYPE_PALETTE)
	{
    	png_set_filler(png, 0xFF, PNG_FILLER_AFTER);
	}

	if (result.color_type == PNG_COLOR_TYPE_GRAY ||
        result.color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
	{
	    png_set_gray_to_rgb(png);
	}

	png_read_update_info(png, info);

 	if (result.row_pointers) abort();

	result.row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * result.height);

	// printf("Row bytes: %zu\n", png_get_rowbytes(png, info));
	for (int y = 0; y < result.height; y++)
	{
		result.row_pointers[y] = (png_byte*)malloc(png_get_rowbytes(png, info));
	}

  	png_read_image(png, result.row_pointers);

  	fclose(fp);

  	png_destroy_read_struct(&png, &info, NULL);

  	return result;
}

void write_png_file(const char *filename, Image image)
{
	FILE *fp = fopen(filename, "wb");
	if (fp == NULL)
	{
		fprintf(stderr, "Could not open file %s: %s\n", filename, strerror(errno));
		abort();
	}

	png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png) abort();

	png_infop info = png_create_info_struct(png);
	if (!info) abort();

	if (setjmp(png_jmpbuf(png))) abort();

	png_init_io(png, fp);

	// Output is 8bit depth, RGBA format.
	png_set_IHDR(
		png,
		info,
		image.width, image.height,
		8,
		PNG_COLOR_TYPE_RGBA,
		PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_DEFAULT,
		PNG_FILTER_TYPE_DEFAULT
	);
	png_write_info(png, info);

	// To remove the alpha channel for PNG_COLOR_TYPE_RGB format,
	// Use png_set_filler().
	// png_set_filler(png, 0, PNG_FILLER_AFTER);

	if (!image.row_pointers) abort();

	png_write_image(png, image.row_pointers);
	png_write_end(png, NULL);

	fclose(fp);

	png_destroy_write_struct(&png, &info);
}

void set_pixel(Image image, int x, int y, int r, int g, int b)
{
   if (x < 0 || x >= image.width || y < 0 || y >= image.height)
   {
      return;
   }

   image.row_pointers[y][x * 4 + 0] = r;
   image.row_pointers[y][x * 4 + 1] = g;
   image.row_pointers[y][x * 4 + 2] = b;
   image.row_pointers[y][x * 4 + 3] = 255;
}

void get_pixel(Image image, int x, int y, int* r, int* g, int* b)
{
   if (x < 0 || x >= image.width || y < 0 || y >= image.height)
   {
      *r = 0;
      *g = 0;
      *b = 0;
      return;
   }

   *r = image.row_pointers[y][x * 4 + 0];
   *g = image.row_pointers[y][x * 4 + 1];
   *b = image.row_pointers[y][x * 4 + 2];
}
