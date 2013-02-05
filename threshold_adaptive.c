/**
* @file threshold_adaptive.c
* @author Matti J. Eskelinen <matti.j.eskelinen@gmail.com>
* @brief Simple program to demonstrate adaptive thresholding.
*
* Copyright (c) 2013, Matti Johannes Eskelinen
* All Rights Reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*   * Redistributions of source code must retain the above copyright notice,
*     this list of conditions and the following disclaimer.
*   * Redistributions in binary form must reproduce the above copyright notice,
*     this list of conditions and the following disclaimer in the documentation
*     and/or other materials provided with the distribution.
*   * Neither the name of the copyright holder nor the names of its
*     contributors may be used to endorse or promote products derived from this
*     software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDER BE LIABLE FOR ANY DIRECT,
* INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "cvsu_config.h"
#include "cvsu_macros.h"
#include "cvsu_pixel_image.h"
#include "cvsu_opencv.h"
#include "cvsu_integral.h"
#include "cvsu_connected_components.h"

string main_name = "threshold_adaptive";

void print_usage()
{
  printf("threshold_adaptive\n");
  printf("Segments images using Feng's improved Sauvola adaptive thresholding.\n\n");
  printf("Usage:\n\n");
  printf("threshold_adaptive radius multiplier source target\n");
  printf("  radius: size of neighborhood used for determining threshold (>= 1)\n");
  printf("  multiplier: size of the larger neighborhood is multiplier*radius (> 1)\n");
  printf("  source: source image file to process\n");
  printf("  target: target image file to generate\n\n");
}

int main(int argc, char *argv[])
{
  TRY();
  pixel_image src_image;
  pixel_image tmp_image;
  pixel_image dst_image;
  integral_image integral;
  connected_components components;
  uint32 radius;
  integral_value multiplier, alpha;
  string source_file, target_file;

  if (argc < 5) {
    printf("\nError: wrong number of parameters\n\n");
    print_usage();
    return 1;
  }
  else {
    int scan_result;

    scan_result = sscanf(argv[1], "%lu", &radius);
    if (scan_result != 1) {
      printf("\nError: failed to parse parameter radius\n\n");
      print_usage();
      return 1;
    }
    scan_result = sscanf(argv[2], "%lf", &multiplier);
    if (scan_result != 1) {
      printf("\nError: failed to parse parameter multiplier\n\n");
      print_usage();
      return 1;
    }
    source_file = argv[3];
    target_file = argv[4];
    if (radius < 1) {
      printf("\nError: radius may not be smaller than 1\n\n");
      print_usage();
      return 1;
    }
    if (multiplier <= 1) {
      printf("\nError: multiplier must be at least 1\n\n");
      print_usage();
      return 1;
    }

    {
      FILE *source;
      source = fopen(source_file, "r");
      if (source == NULL) {
        printf("\nError: the source file does not exist\n\n");
        print_usage();
        return 1;
      }
      fclose(source);
    }
  }

  alpha = 3;

  printf("load image...\n");
  CHECK(pixel_image_create_from_file(&src_image, source_file, p_U8, GREY));
  printf("create integral...\n");
  CHECK(integral_image_create(&integral, &src_image));
  printf("updating integral...\n");
  CHECK(integral_image_update(&integral));
  printf("thresholding...\n");
  CHECK(integral_image_threshold_feng(&integral, &tmp_image, TRUE,
                                      ((signed)radius), multiplier, TRUE, alpha));
  printf("creating connected components...\n");
  CHECK(connected_components_create(&components, &tmp_image));
  printf("updating connected components...\n");
  CHECK(connected_components_update(&components));
  printf("drawing result...\n");
  CHECK(connected_components_draw_image(&components, &dst_image));
  printf("writing result to file...\n");
  CHECK(pixel_image_write_to_file(&dst_image, target_file));
  printf("done!\n");

  FINALLY(main);
  connected_components_destroy(&components);
  integral_image_destroy(&integral);
  pixel_image_destroy(&dst_image);
  pixel_image_destroy(&tmp_image);
  pixel_image_destroy(&src_image);

  return 0;
}
