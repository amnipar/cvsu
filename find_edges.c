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

/*
 * Usage: find_edges width length source_file target_file
 */

int main (int argc, char *argv[])
{
  TRY();
  pixel_image src_image;
  pixel_image tmp_image;
  edge_image edges;
  uint32 width, length;
  string source_file, target_file;

  width = 12;
  length = 6;
  source_file = "smallLena.jpg";
  target_file = "edges.png";

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
