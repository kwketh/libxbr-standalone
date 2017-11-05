 /* fix_png.c - A tool to fix transparent pixel color in PNGs so they
                can be scaled/rotated/whatevs without fringe artifacts.
                
                by Christian Ruocco on 19 April 2017.
                Used original PNG loading/saving example code by 
                Guillaume Cottenceau then implemented my own filtering 
                algorithm in that. This code is released under the same 
                licence as his. See original copyright/licence terms 
                below. 
                
                This code is not remotely optimized, but it doesn't
                matter since it writes out a new PNG so it only needs
                to be run once per PNG you want fixed. It also removes
                unneeded headers inserted by art programs which I've 
                seen used a buttload of space on some images so your
                final PNG should a few K smaller. */
                
                
 /*
  * Copyright 2002-2010 Guillaume Cottenceau.
  *
  * This software may be freely redistributed under the terms
  * of the X11 license.
  *
  */
 
 #include <unistd.h>
 #include <stdlib.h>
 #include <stdio.h>
 #include <string.h>
 #include <stdarg.h>
 
 #define PNG_DEBUG 3
 #include <png.h>
 
 void abort_(const char * s, ...)
 {
         va_list args;
         va_start(args, s);
         vfprintf(stderr, s, args);
         fprintf(stderr, "\n");
         va_end(args);
         abort();
 }
 
 int x, y, threshold = 16;
 
 int width, height;
 png_byte color_type;
 png_byte bit_depth;
 
 png_structp png_ptr;
 png_infop info_ptr;
 int number_of_passes;
 png_bytep * row_pointers;
 
 void read_png_file(char* file_name)
 {
         char header[8];    // 8 is the maximum size that can be checked
 
         /* open file and test for it being a png */
         FILE *fp = fopen(file_name, "rb");
         if (!fp)
                 abort_("[read_png_file] File %s could not be opened for reading", file_name);
         fread(header, 1, 8, fp);
         //if (png_sig_cmp(header, 0, 8))
         //        abort_("[read_png_file] File %s is not recognized as a PNG file", file_name);
 
 
         /* initialize stuff */
         png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
 
         if (!png_ptr)
                 abort_("[read_png_file] png_create_read_struct failed");
 
         info_ptr = png_create_info_struct(png_ptr);
         if (!info_ptr)
                 abort_("[read_png_file] png_create_info_struct failed");
 
         if (setjmp(png_jmpbuf(png_ptr)))
                 abort_("[read_png_file] Error during init_io");
 
         png_init_io(png_ptr, fp);
         png_set_sig_bytes(png_ptr, 8);
 
         png_read_info(png_ptr, info_ptr);
 
         width = png_get_image_width(png_ptr, info_ptr);
         height = png_get_image_height(png_ptr, info_ptr);
         color_type = png_get_color_type(png_ptr, info_ptr);
         bit_depth = png_get_bit_depth(png_ptr, info_ptr);
 
         number_of_passes = png_set_interlace_handling(png_ptr);
         png_read_update_info(png_ptr, info_ptr);
 
 
         /* read file */
         if (setjmp(png_jmpbuf(png_ptr)))
                 abort_("[read_png_file] Error during read_image");
 
         row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * height);
         for (y=0; y<height; y++)
                 row_pointers[y] = (png_byte*) malloc(png_get_rowbytes(png_ptr,info_ptr));
 
         png_read_image(png_ptr, row_pointers);
 
         fclose(fp);
 }
 
 
 void write_png_file(char* file_name)
 {
         /* create file */
         FILE *fp = fopen(file_name, "wb");
         if (!fp)
                 abort_("[write_png_file] File %s could not be opened for writing", file_name);
 
 
         /* initialize stuff */
         png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
 
         if (!png_ptr)
                 abort_("[write_png_file] png_create_write_struct failed");
 
         info_ptr = png_create_info_struct(png_ptr);
         if (!info_ptr)
                 abort_("[write_png_file] png_create_info_struct failed");
 
         if (setjmp(png_jmpbuf(png_ptr)))
                 abort_("[write_png_file] Error during init_io");
 
         png_init_io(png_ptr, fp);
 
 
         /* write header */
         if (setjmp(png_jmpbuf(png_ptr)))
                 abort_("[write_png_file] Error during writing header");
 
         png_set_IHDR(png_ptr, info_ptr, width, height,
                      bit_depth, color_type, PNG_INTERLACE_NONE,
                      PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
 
         png_write_info(png_ptr, info_ptr);
 
 
         /* write bytes */
         if (setjmp(png_jmpbuf(png_ptr)))
                 abort_("[write_png_file] Error during writing bytes");
 
         png_write_image(png_ptr, row_pointers);
 
 
         /* end write */
         if (setjmp(png_jmpbuf(png_ptr)))
                 abort_("[write_png_file] Error during end of write");
 
         png_write_end(png_ptr, NULL);
 
         /* cleanup heap allocation */
         for (y=0; y<height; y++)
                 free(row_pointers[y]);
         free(row_pointers);
 
         fclose(fp);
 }
 
 
 unsigned int get_pixel(int x, int y)
 {
         if (y < 0 || y >= height ||
             x < 0 || x >= width)
             return 0;
         
         png_byte *pixel = row_pointers[y] + x * 4;
         
         return (pixel[0] << 0) | (pixel[1] << 8) | (pixel[2] << 16) | (pixel[3]<<24);
 }
 
 void set_pixel(int x, int y, unsigned int color)
 {
         if (y < 0 || y >= height ||
             x < 0 || x >= width)
             return;
         
         png_byte *pixel = row_pointers[y] + x * 4;
         
         pixel[0] = color >> 0;
         pixel[1] = color >> 8;
         pixel[2] = color >> 16;
         pixel[3] = color >> 24;
 }
 
 void process_file(void)
 {
         if (png_get_color_type(png_ptr, info_ptr) == PNG_COLOR_TYPE_RGB)
                 abort_("[process_file] input file is PNG_COLOR_TYPE_RGB but must be PNG_COLOR_TYPE_RGBA "
                        "(lacks the alpha channel)");
 
         if (png_get_color_type(png_ptr, info_ptr) != PNG_COLOR_TYPE_RGBA)
                 abort_("[process_file] color_type of input file must be PNG_COLOR_TYPE_RGBA (%d) (is %d)",
                        PNG_COLOR_TYPE_RGBA, png_get_color_type(png_ptr, info_ptr));
 
 
         
         for (y=0; y<height; y++) 
         {
                 for (x=0; x<width; x++) 
                 {
                         int red = 0;
                         int green = 0;
                         int blue = 0;
                         int alpha = 0;
                         
                         unsigned color;
                         png_byte *cc = (png_byte*)&color;
                         
                         color = get_pixel(x, y);
                         
                         /* We consider any alpha value <=16 to
                            be zero for "noisy" images. */
                         if (cc[3] > threshold)
                                 continue;
                         
                         int i,j;
                         for(i=-1;i<=1;i++)
                         for(j=-1;j<=1;j++)
                         {
                                 if (i==0 && j==0)
                                         continue;
                                         
                                 color = get_pixel(x+i, y+j);
                                 red += cc[0] * cc[3];
                                 green += cc[1] * cc[3];
                                 blue += cc[2] * cc[3];
                                 alpha += cc[3];
                         }
                         
                         if (alpha == 0)
                                 continue;
                                 
                         red /= alpha;
                         green /= alpha;
                         blue /= alpha;
                         
                         
                         color = get_pixel(x, y);
                         
                         cc[0] = red;
                         cc[1] = green;
                         cc[2] = blue;
                                 
                         set_pixel(x, y, color);
                 }
         }
 }
 
 
 int main(int argc, char **argv)
 {
         if (argc < 3 || argc > 4)
                 abort_("Usage: program_name <file_in> <file_out> [<threshold=16>]");
 
         if (argc == 4)
                 threshold = atoi(argv[3]);

         read_png_file(argv[1]);
         process_file();
         write_png_file(argv[2]);
 
         return 0;
 }
 
 