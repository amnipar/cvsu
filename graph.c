/**
 * @file graph.c
 * @author Matti J. Eskelinen <matti.j.eskelinen@gmail.com>
 * @brief Simple program to demonstrate graph-based image analysis operations.
 *
 * Copyright (c) 2014, Matti Johannes Eskelinen
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

#include "cvsu_macros.h"
#include "cvsu_output.h"
#include "cvsu_pixel_image.h"
#include "cvsu_typed_pointer.h"
#include "cvsu_graph.h"
#include "cvsu_set.h"
#include "cvsu_opencv.h"

#include <string.h>

string main_name = "graph";

void print_usage()
{
  PRINT0("graph\n");
  PRINT0("Performs various operations using image graph representations.\n\n");
  PRINT0("Usage:\n\n");
  PRINT0("graph mode stepx stepy source target\n");
  PRINT0("  mode: analysis mode [ connected | msf | contour ]\n");
  PRINT0("  stepx: horizontal step in pixels between nodes\n");
  PRINT0("  stepy: vertical step in pixels between nodes\n");
  PRINT0("  source: source image file to process\n");
  PRINT0("  target: target image file to generate\n\n");
}

enum mode_t {
  m_UNDEF = 0,
  m_CONNECTED,
  m_MSF,
  m_CONTOUR
};

int main(int argc, char *argv[])
{
  TRY();
  pixel_image src_image, tmp_image, dst_image;
  uint32 value_key, weight_key, set_key, stat_key;
  /*typed_pointer *tptr;*/
  /*attribute *attr;*/
  attribute *set_attr;
  graph g;
  uint32 stepx, stepy;
  string smode, source_file, target_file;
  enum mode_t mode;
  
  set_attr = NULL;

  if (argc < 6) {
    PRINT0("\nError: wrong number of parameters\n\n");
    print_usage();
    return 1;
  }
  else {
    int scan_result;

    mode = m_UNDEF;
    smode = argv[1];
    if (strcmp(smode, "connected") == 0) {
      mode = m_CONNECTED;
    }
    else
    if (strcmp(smode, "msf") == 0) {
      mode = m_MSF;
    }
    else
    if (strcmp(smode, "contour") == 0) {
      mode = m_CONTOUR;
    }
    else {
      PRINT1("\nError: unsupported mode (%s)\n\n", smode);
      print_usage();
      return 1;
    }

    scan_result = sscanf(argv[2], "%lu", &stepx);
    if (scan_result != 1) {
      PRINT0("\nError: failed to parse parameter stepx\n\n");
      print_usage();
      return 1;
    }
    scan_result = sscanf(argv[3], "%lu", &stepy);
    if (scan_result != 1) {
      PRINT0("\nError: failed to parse parameter stepy\n\n");
      print_usage();
      return 1;
    }
    source_file = argv[4];
    target_file = argv[5];
    if (stepx < 1) {
      PRINT0("\nError: stepx may not be smaller than 1\n\n");
      print_usage();
      return 1;
    }
    if (stepy < 1) {
      PRINT0("\nError: stepy may not be smaller than 1\n\n");
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

  PRINT0("load image...\n");
  CHECK(pixel_image_create_from_file(&src_image, source_file, p_U8, GREY));
  CHECK(pixel_image_create(&tmp_image, p_U8, RGB, src_image.width,
                           src_image.height, 3, 3 * src_image.width));
  CHECK(pixel_image_create(&dst_image, p_U8, RGB, 5 * src_image.width,
                           5 * src_image.height, 3, 3 * 5 * src_image.width));
  
  CHECK(convert_grey8_to_grey24(&src_image, &tmp_image));
  CHECK(pixel_image_replicate_pixels(&tmp_image, &dst_image, 5));
  
  value_key = 1;
  weight_key = 2;
  set_key = 3;
  stat_key = 4;
  /*tptr = typed_pointer_alloc();*/
  /*CHECK(typed_pointer_create(tptr, t_S32, 1, 1, (pointer)&def));*/
  /*
  PRINT0("create attribute...\n");
  attr = attribute_alloc();
  CHECK(attribute_create(attr, 1, tptr));
  */
  PRINT0("create graph...\n");
  CHECK(graph_create_from_image(&g, &src_image, 5, 5, stepx, stepy,
                                NEIGHBORHOOD_4, value_key, weight_key));
  
  /* run the requested algorithm on the graph */
  switch (mode) {
    case m_CONNECTED:
      PRINT0("finding connected components...\n");

      break;
    case m_MSF:
      PRINT0("finding minimum spanning forest...\n");
      {
        /*typed_pointer *tptr;*/
        /* add a set attribute containing statistics into each node */
        graph_for_attrs_in_each_node(&g, &disjoint_set_add_stat_attr, &set_key);
      }
      break;
    case m_CONTOUR:
      PRINT0("finding contours...\n");

      break;
    default:
      PRINT0("unknown mode\n");
      ERROR(BAD_PARAM);
  }
  
  /* draw nodes on image */
  PRINT0("drawing graph...\n");
  CHECK(graph_draw_nodes(&g, &dst_image, 1, 2, 5));
  
  /* write the resulting image to file */
  PRINT0("writing result to file...\n");
  CHECK(pixel_image_write_to_file(&dst_image, target_file));
  PRINT0("done!\n");

  FINALLY(main);
  PRINT0("destroy graph\n");
  graph_destroy(&g);
  /*
  attribute_free(attr);
  typed_pointer_free(tptr);
  */
  PRINT0("destroyed\n");
  pixel_image_destroy(&dst_image);
  pixel_image_destroy(&tmp_image);
  pixel_image_destroy(&src_image);
  if (set_attr != NULL) {
    attribute_destroy(set_attr);
  }
  return 0;
}
