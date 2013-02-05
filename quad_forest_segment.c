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

/*
 * Usage: quad_forest_segment max_size min_size alpha tree_overlap
 *        segment_overlap source_file target_file
 */

int main(int argc, char *argv[])
{
  TRY();
  pixel_image src_image;
  pixel_image dst_image;
  quad_forest forest;
  uint32 max_size, min_size;
  integral_value alpha, tree_overlap, segment_overlap;
  string source_file, target_file;

  max_size = 16;
  min_size = 4;
  alpha = 3;
  tree_overlap = 0.5;
  segment_overlap = 0.5;
  source_file = "smallLena.jpg";
  target_file = "segmented.png";

  printf("load image...\n");
  CHECK(pixel_image_create_from_file(&src_image, source_file, p_U8, GREY));
  printf("create forest...\n");
  CHECK(quad_forest_create(&forest, &src_image, max_size, min_size));
  printf("updating forest...\n");
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
