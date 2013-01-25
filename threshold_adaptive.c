/**
* @file threshold_adaptive.c
* @author Matti J. Eskelinen (matti dot j dot eskelinen at jyu dot fi)
* @brief Simple program to demonstrate adaptive thresholding.
*
* Copyright (c) 2013, Matti Johannes Eskelinen
* All Rights Reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*   * Redistributions of source code must retain the above copyright
*     notice, this list of conditions and the following disclaimer.
*   * Redistributions in binary form must reproduce the above copyright
*     notice, this list of conditions and the following disclaimer in the
*     documentation and/or other materials provided with the distribution.
*   * Neither the name of the copyright holder nor the
*     names of its contributors may be used to endorse or promote products
*     derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDER BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
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

/*
 * Usage: threshold_adaptive radius multiplier alpha source_file target_file
 */

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

  radius = 7;
  multiplier = 3;
  alpha = 3;
  source_file = "smallLena.jpg";
  target_file = "thresholded.png";

  printf("load image...\n");
  CHECK(pixel_image_create_from_file(&src_image, source_file, p_U8, GREY));
  printf("create integral...\n");
  CHECK(integral_image_create(&integral, &src_image));
  printf("updating integral...\n");
  CHECK(integral_image_update(&integral));
  printf("thresholding...\n");
  CHECK(integral_image_threshold_feng(&integral, &tmp_image, TRUE, radius, multiplier, TRUE, alpha));
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
