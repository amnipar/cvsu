/**
 * @file quad_forest_segment.c
 * @author Matti J. Eskelinen <matti.j.eskelinen@gmail.com>
 * @brief Simple program to demonstrate quad forest segmentation.
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
#include "cvsu_quad_forest.h"

string main_name = "quad_forest_segment";

void print_usage()
{
  printf("quad_forest_segment\n");
  printf("Segments images using quad forests with range overlap measures.\n\n");
  printf("Usage:\n\n");
  printf("quad_forest_segment max min alpha toverlap soverlap source target\n");
  printf("  max: maximum size for trees; suggested value 16 (larger than min)\n");
  printf("  min: minimum size for tree;  suggested value 4 (smaller than max)\n");
  printf("  alpha: deviation multiplier for range generation, suggested value 3 (0..5]\n");
  printf("  toverlap: required overlap for trees, suggested value 0.5 (0..1)\n");
  printf("  soverlap: required overlap for segments, suggested value 0.5 (0..1)\n");
  printf("  source: source image file to process\n");
  printf("  target: target image file to generate\n\n");
}

int main(int argc, char *argv[])
{
  TRY();
  pixel_image src_image;
  pixel_image dst_image;
  quad_forest forest;
  uint32 max_size, min_size;
  integral_value alpha, tree_overlap, segment_overlap;
  string source_file, target_file;

  if (argc < 8) {
    printf("\nError: wrong number of parameters\n\n");
    print_usage();
    return 1;
  }
  else {
    int scan_result;

    scan_result = sscanf(argv[1], "%lu", &max_size);
    if (scan_result != 1) {
      printf("\nError: failed to parse parameter max\n\n");
      print_usage();
      return 1;
    }
    scan_result = sscanf(argv[2], "%lu", &min_size);
    if (scan_result != 1) {
      printf("\nError: failed to parse parameter min\n\n");
      print_usage();
      return 1;
    }
    scan_result = sscanf(argv[3], "%lf", &alpha);
    if (scan_result != 1) {
      printf("\nError: failed to parse parameter alpha\n\n");
      print_usage();
      return 1;
    }
    scan_result = sscanf(argv[4], "%lf", &tree_overlap);
    if (scan_result != 1) {
      printf("\nError: failed to parse parameter toverlap\n\n");
      print_usage();
      return 1;
    }
    scan_result = sscanf(argv[5], "%lf", &segment_overlap);
    if (scan_result != 1) {
      printf("\nError: failed to parse parameter soverlap\n\n");
      print_usage();
      return 1;
    }
    source_file = argv[6];
    target_file = argv[7];
    if (max_size < min_size) {
      printf("\nError: max may not be smaller than min\n\n");
      print_usage();
      return 1;
    }
    if (alpha <= 0 || alpha > 5) {
      printf("\nError: alpha must be in range (0..5]\n\n");
      print_usage();
      return 1;
    }
    if (tree_overlap <= 0 || tree_overlap >= 1) {
      printf("\nError: toverlap must be in range (0..1)\n\n");
      print_usage();
      return 1;
    }
    if (segment_overlap <= 0 || segment_overlap >= 1) {
      printf("\nError: soverlap must be in range (0..1)\n\n");
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
  printf("create forest...\n");
  CHECK(quad_forest_create(&forest, &src_image, max_size, min_size));
  printf("updating forest...\n");
  forest.token = 1;
  CHECK(quad_forest_update(&forest));
  printf("segmenting...\n");
  CHECK(quad_forest_segment_with_overlap(&forest, alpha, tree_overlap, segment_overlap));
  printf("found %lu segments!\n", forest.segments);
  printf("drawing result...\n");
  CHECK(quad_forest_draw_image(&forest, &dst_image, TRUE, TRUE));
  printf("writing result to file...\n");
  CHECK(pixel_image_write_to_file(&dst_image, target_file));
  printf("done!\n");

  FINALLY(main);
  quad_forest_destroy(&forest);
  pixel_image_destroy(&dst_image);
  pixel_image_destroy(&src_image);

  return 0;
}
