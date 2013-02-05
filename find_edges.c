/**
 * @file find_edges.c
 * @author Matti J. Eskelinen <matti.j.eskelinen@gmail.com>
 * @brief Simple program to demonstrate edge detection.
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

#include "cvsu_config.h"
#include "cvsu_macros.h"
#include "cvsu_pixel_image.h"
#include "cvsu_opencv.h"
#include "cvsu_filter.h"
#include "cvsu_edges.h"

string main_name = "find_edges";

void print_usage()
{
  printf("find_edges\n");
  printf("Finds edges by calculating edge responses with box filters.\n\n");
  printf("Usage:\n\n");
  printf("find_edges width length source target\n");
  printf("  width: width of boxes used in calculating edge response (>= 1)\n");
  printf("  length: length of boxes used in calculating edge response (>= 1)\n");
  printf("  source: source image file to process\n");
  printf("  target: target image file to generate\n\n");
}

int main (int argc, char *argv[])
{
  TRY();
  pixel_image src_image;
  pixel_image tmp_image;
  edge_image edges;
  uint32 width, length;
  string source_file, target_file;

  if (argc < 5) {
    printf("\nError: wrong number of parameters\n\n");
    print_usage();
    return 1;
  }
  else {
    int scan_result;

    scan_result = sscanf(argv[1], "%lu", &width);
    if (scan_result != 1) {
      printf("\nError: failed to parse parameter width\n\n");
      print_usage();
      return 1;
    }
    scan_result = sscanf(argv[2], "%lu", &length);
    if (scan_result != 1) {
      printf("\nError: failed to parse parameter length\n\n");
      print_usage();
      return 1;
    }
    source_file = argv[3];
    target_file = argv[4];
    if (width < 1) {
      printf("\nError: width may not be smaller than 1\n\n");
      print_usage();
      return 1;
    }
    if (length < 1) {
      printf("\nError: length must not be smaller than 1\n\n");
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

  printf("load image...\n");
  CHECK(pixel_image_create_from_file(&src_image, source_file, p_U8, GREY));
  printf("...done\n");

  printf("smooth image..\n");
  CHECK(pixel_image_clone(&tmp_image, &src_image));
  CHECK(smooth_binomial(&src_image, &tmp_image, 2));
  printf("...done\n");

  printf("create edge image...\n");
  CHECK(edge_image_create(&edges, &tmp_image, width, width, width, width, width, length));
  printf("...done\n");

  CHECK(edge_image_update(&edges));
  CHECK(edge_image_overlay_to_grey8(&edges, &tmp_image));
  printf("write image...\n");
  CHECK(pixel_image_write_to_file(&tmp_image, target_file));
  printf("...done\n");

  FINALLY(main);
  pixel_image_destroy(&src_image);
  pixel_image_destroy(&tmp_image);
  edge_image_destroy(&edges);

  return 0;
}
