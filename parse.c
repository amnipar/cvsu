/**
 * @file parse.c
 * @author Matti J. Eskelinen <matti.j.eskelinen@gmail.com>
 * @brief Simple program to demonstrate image parsing operations.
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

#include "cvsu_macros.h"
#include "cvsu_output.h"
#include "cvsu_pixel_image.h"
#include "cvsu_quad_forest.h"
#include "cvsu_parsing.h"
#include "cvsu_opencv.h"

#include <string.h>

string main_name = "parse";

void print_usage()
{
  PRINT0("parse\n");
  PRINT0("Parses images using quad forests and propagation.\n\n");
  PRINT0("Usage:\n\n");
  PRINT0("parse mode max min rounds source target\n");
  PRINT0("  mode: parsing mode [ stat | nstat | overlap | strength | full ]\n");
  PRINT0("  max: maximum size for trees; suggested value 16 (larger than min)\n");
  PRINT0("  min: minimum size for tree;  suggested value 4 (smaller than max)\n");
  PRINT0("  rounds: number of propagation rounds (0..5]\n");
  PRINT0("  weight: use this weight for calculation of neighborhood stats, 0 for no weighting\n");
  PRINT0("  source: source image file to process\n");
  PRINT0("  target: target image file to generate\n\n");
}

enum mode_t {
  m_UNDEF = 0,
  m_STAT,
  m_NSTAT,
  m_OVERLAP,
  m_STRENGTH,
  m_FULL
};

int main(int argc, char *argv[])
{
  TRY();
  pixel_image src_image;
  pixel_image dst_image;
  quad_forest forest;
  uint32 max_size, min_size, rounds;
  integral_value weight;
  truth_value use_weighted;
  string smode, source_file, target_file;
  enum mode_t mode;
  /*list lines;*/

  if (argc < 8) {
    PRINT0("\nError: wrong number of parameters\n\n");
    print_usage();
    return 1;
  }
  else {
    int scan_result;

    mode = m_UNDEF;
    smode = argv[1];
    if (strcmp(smode, "stat") == 0) {
      mode = m_STAT;
    }
    else
    if (strcmp(smode, "nstat") == 0) {
      mode = m_NSTAT;
    }
    else
    if (strcmp(smode, "overlap") == 0) {
      mode = m_OVERLAP;
    }
    else
    if (strcmp(smode, "strength") == 0) {
      mode = m_STRENGTH;
    }
    else
    if (strcmp(smode, "full") == 0) {
      mode = m_FULL;
    }
    else {
      PRINT1("\nError: unsupported mode (%s)\n\n", smode);
      print_usage();
      return 1;
    }

    scan_result = sscanf(argv[2], "%lu", &max_size);
    if (scan_result != 1) {
      PRINT0("\nError: failed to parse parameter max\n\n");
      print_usage();
      return 1;
    }
    scan_result = sscanf(argv[3], "%lu", &min_size);
    if (scan_result != 1) {
      PRINT0("\nError: failed to parse parameter min\n\n");
      print_usage();
      return 1;
    }
    scan_result = sscanf(argv[4], "%lu", &rounds);
    if (scan_result != 1) {
      PRINT0("\nError: failed to parse parameter rounds\n\n");
      print_usage();
      return 1;
    }
    scan_result = sscanf(argv[4], "%lu", &rounds);
    if (scan_result != 1) {
      PRINT0("\nError: failed to parse parameter rounds\n\n");
      print_usage();
      return 1;
    }
    scan_result = sscanf(argv[5], "%lf", &weight);
    if (scan_result != 1) {
      PRINT0("\nError: failed to parse parameter weight\n\n");
      print_usage();
      return 1;
    }
    source_file = argv[6];
    target_file = argv[7];
    if (max_size < min_size) {
      PRINT0("\nError: max may not be smaller than min\n\n");
      print_usage();
      return 1;
    }
    if (rounds <= 0 || rounds > 5) {
      PRINT0("\nError: rounds must be in range (0..5]\n\n");
      print_usage();
      return 1;
    }
    if (weight < 0 || weight > 10) {
      PRINT0("\nError: weight must be in range [0..10]\n\n");
    }
    use_weighted = FALSE;
    if (weight > 0.0000001) {
      use_weighted = TRUE;
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
  CHECK(pixel_image_create(&dst_image, p_U8, RGB, src_image.width, src_image.height,
                           3, 3 * src_image.width));
  CHECK(convert_grey8_to_grey24(&src_image, &dst_image));
  PRINT0("create forest...\n");
  CHECK(quad_forest_create(&forest, &src_image, max_size, min_size));
  PRINT0("updating forest...\n");
  CHECK(quad_forest_update(&forest));
  forest.token = 1;
  switch (mode) {
    case m_STAT:
      PRINT0("parsing stat...\n");
      /*CHECK(quad_forest_calculate_accumulated_stats(&forest, rounds));*/
      PRINT0("drawing image...\n");
      CHECK(quad_forest_visualize_neighborhood_stats(&forest, &dst_image, v_STAT));
      break;
    case m_NSTAT:
      PRINT0("parsing nstat...\n");
      CHECK(quad_forest_calculate_neighborhood_stats(&forest));
      PRINT0("drawing image...\n");
      CHECK(quad_forest_visualize_neighborhood_stats(&forest, &dst_image, v_NSTAT));
      break;
    case m_OVERLAP:
      PRINT0("parsing overlap...\n");
      CHECK(quad_forest_calculate_neighborhood_stats(&forest));
      PRINT0("drawing image...\n");
      CHECK(quad_forest_visualize_neighborhood_stats(&forest, &dst_image, v_OVERLAP));
      break;
    case m_STRENGTH:
      PRINT0("parsing strength...\n");
      CHECK(quad_forest_calculate_neighborhood_stats(&forest));
      PRINT0("drawing image...\n");
      CHECK(quad_forest_visualize_neighborhood_stats(&forest, &dst_image, v_STRENGTH));
      break;
    case m_FULL:
      PRINT0("parsing...\n");
      CHECK(quad_forest_parse(&forest, rounds, FALSE));
      PRINT0("drawing image...\n");
      CHECK(quad_forest_visualize_parse_result(&forest, &dst_image));
      break;
    default:
      PRINT0("unknown mode\n");
      ERROR(BAD_PARAM);
  }
  /*
  PRINT0("drawing boundaries...\n");
  CHECK(list_create(&lines, 100, sizeof(line), 1));
  CHECK(quad_forest_find_accumulated_boundaries(&forest, rounds, &lines));
  CHECK(pixel_image_draw_lines(&dst_image, &lines));
  */
  /*CHECK(quad_forest_draw_image(&forest, &dst_image, FALSE, FALSE));*/
  PRINT0("writing result to file...\n");
  CHECK(pixel_image_write_to_file(&dst_image, target_file));
  PRINT0("done!\n");

  FINALLY(main);
  PRINT0("destroy forest\n");
  quad_forest_destroy(&forest);
  PRINT0("destroyed\n");
  pixel_image_destroy(&dst_image);
  pixel_image_destroy(&src_image);
  /*list_destroy(&lines);*/
  return 0;
}
