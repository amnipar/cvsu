/**
 * @file main.c
 * @author Matti J. Eskelinen (matti dot j dot eskelinen at jyu dot fi)
 * @brief Simple program to test image operations.
 *
 * Copyright (c) 2011, Matti Johannes Eskelinen
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
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

#include "cvsu_config.h"
#include "cvsu_macros.h"
#include "cvsu_memory.h"
#include "cvsu_basic.h"
#include "cvsu_integral.h"
#include "cvsu_filter.h"
#include "cvsu_scale.h"
#include "cvsu_edges.h"
#include "cvsu_list.h"
#include "cvsu_simple_scene.h"
#include "cvsu_image_tree.h"

const uint32 ImageWidth = 320;
const uint32 ImageHeight = 320;

string main_name = "main";
string ifn = "if.bin";
string ofn = "of.bin";

/*
Usage: cv-sks [p #]
Without parameters, writes to output image the result of edge detection
using box filters and deviation.
With parameter p, writes to output image the result of edge detection
using sobel operator in scale space.
With additional parameter # (number 0-4) shows the image pyramid level
with that number
*/
int main (int argc, char *argv[])
{
    TRY();
    size_t size;
    pixel_image input_image;
    pixel_image output_image;
    pixel_image temp_image;
    image_pyramid pyramid;
    integral_image integral;
    simple_scene scene;
    image_tree_forest forest;
    /*line scene_lines[100];*/
    /*uint32 line_count;*/
    FILE* ifh;
    FILE* ofh;
    bool use_scene = false;
    bool use_pyramid = false;
    bool show_scale = false;
    long scale_to_show = 0;

    if (argc > 1) {
        if (*argv[1] == 'p') {
            use_pyramid = true;
            if (argc > 2) {
                show_scale = true;
                scale_to_show = atoi(argv[2]);
                if (scale_to_show < 0) scale_to_show = 0;
                if (scale_to_show > 4) scale_to_show = 4;
            }
        }
        else if (*argv[1] == 's') {
            use_scene = true;
        }
    }

    size = (ImageWidth * ImageHeight);

    CHECK(pixel_image_create(&input_image, p_U8, GREY, ImageWidth, ImageHeight, 1, ImageWidth));
    CHECK(pixel_image_create(&output_image, p_U8, GREY, ImageWidth, ImageHeight, 1, ImageWidth));
    CHECK(pixel_image_create(&temp_image, p_S32, GREY, ImageWidth, ImageHeight, 1, ImageWidth));
    CHECK(image_pyramid_create(&pyramid, &input_image, 5));
    CHECK(integral_image_create(&integral, &input_image));
    CHECK(simple_scene_create(&scene, &input_image));
    CHECK(image_tree_forest_create(&forest, &input_image, 32, 32));

    ifh = fopen(ifn, "rb");
    ofh = fopen(ofn, "wb");

    if (ifh != NULL && ofh != NULL) {
        size_t readcount = fread(input_image.data, sizeof(byte), size, ifh);
        if (readcount == size) {
            /*uint32 i;*/
            struct timeval start;
            struct timeval finish;
            double timediff;

            /*
            gettimeofday(&start, NULL);
            if (use_pyramid) {
                for (i = 0; i < 1000; i++) {
                    CHECK(edges_x_sobel_scale(&pyramid, &temp_image, &output_image, 12));
                }
            }
            else if (use_scene) {
                pixel_image_copy(&output_image, &input_image);
                for (i = 0; i < 1000; i++) {
                    CHECK(simple_scene_update(&scene));
                    CHECK(edge_image_overlay_to_grey8(&scene.curr_edges, &output_image));
                }
                line_count = 0;
                CHECK(simple_scene_pack_lines_to_array(&scene, scene_lines, 100, &line_count));
                printf("Lines found: %lu\n", line_count);
            }
            else {
                for (i = 0; i < 1000; i++) {
                    CHECK(edges_x_box_deviation(&integral, &temp_image, &output_image, 8, 16));
                }
            }
            gettimeofday(&finish, NULL);
            timediff = (double)(finish.tv_sec - start.tv_sec) + (double)(finish.tv_usec - start.tv_usec) / 1000000.0;
            printf("Time taken: %f\n", timediff);
            */
            gettimeofday(&start, NULL);
            CHECK(simple_scene_update(&scene));
            gettimeofday(&finish, NULL);
            timediff = (double)(finish.tv_sec - start.tv_sec) + (double)(finish.tv_usec - start.tv_usec) / 1000000.0;
            printf("Scene, time taken: %f\n", timediff);
            gettimeofday(&start, NULL);
            CHECK(image_tree_forest_update(&forest));
            gettimeofday(&finish, NULL);
            timediff = (double)(finish.tv_sec - start.tv_sec) + (double)(finish.tv_usec - start.tv_usec) / 1000000.0;
            printf("Forest, time taken: %f\n", timediff);

            if (show_scale) {
                fwrite(pyramid.levels[scale_to_show].data, sizeof(byte), size, ofh);
            }
            else {
                fwrite(output_image.data, sizeof(byte), size, ofh);
            }
        }
        else {
            printf("Read error\n");
        }
        fclose(ifh);
        fclose(ofh);
    }
    else {
        printf("File open error\n");
    }

    FINALLY(main);
    pixel_image_destroy(&input_image);
    pixel_image_destroy(&output_image);
    pixel_image_destroy(&temp_image);
    image_pyramid_destroy(&pyramid);
    integral_image_destroy(&integral);
    simple_scene_destroy(&scene);

    return 0;
}
