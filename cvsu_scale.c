/**
 * @file cvsu_scale.c
 * @author Matti J. Eskelinen <matti.j.eskelinen@gmail.com>
 * @brief Scale-space handling and operations for the cvsu module.
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
 */

#include "cvsu_config.h"
#include "cvsu_macros.h"
#include "cvsu_memory.h"
#include "cvsu_scale.h"
#include "cvsu_filter.h"

#include <math.h>

string image_pyramid_create_name = "image_pyramid_create";
string image_pyramid_destroy_name = "image_pyramid_destroy";
string image_pyramid_clone_name = "image_pyramid_clone";
string image_pyramid_copy_name = "image_pyramid_copy";
string image_pyramid_down_name = "image_pyramid_down";
string image_pyramid_up_name = "image_pyramid_up";
string image_pyramid_max_name = "image_pyramid_max";
string image_pyramid_min_name = "image_pyramid_min";
string edges_x_sobel_scale_name = "edges_x_sobel_scale";

result image_pyramid_create(
    image_pyramid *target,
    pixel_image *source,
    uint32 levels
    )
{
    TRY();
    uint32 i, minsize;

    /* make sure the state is set to invalid in all situations */
    target->state = INVALID;

    CHECK_POINTER(target);
    CHECK_POINTER(source);
    CHECK_POINTER(source->data);
    CHECK_PARAM(source->type == p_U8);

    /* original image size must be large enough to fit the levels */
    minsize = (uint32)pow(2.0, (double)levels);
    CHECK_PARAM(source->width > minsize);
    CHECK_PARAM(source->height > minsize);

    target->source = source;
    target->level_count = levels;
    target->width = source->width;
    target->height = source->height;
    target->step = source->step;
    target->stride = source->step * source->width;

    /* allocate space for storing the images for the levels */
    CHECK(memory_allocate((data_pointer *)&target->levels, levels, sizeof(pixel_image)));
    CHECK(memory_allocate((data_pointer *)&target->roi, levels, sizeof(pixel_image)));

    /* allocate images for the levels */
    for (i = 0; i < levels; i++) {
        CHECK(pixel_image_create(&target->levels[i], source->type, source->format, target->width, target->height, target->step, target->stride));
        CHECK(pixel_image_create_roi(&target->roi[i], &target->levels[i], 0, 0, target->width, target->height));
    }

    /* set state to init only after all steps are completed successfully */
    target->state = INIT;
    FINALLY(image_pyramid_create);
    RETURN();
}

result image_pyramid_destroy(
    image_pyramid *target
    )
{
    TRY();
    uint32 i;

    CHECK_POINTER(target);

    /* free allocated pyramid levels - level 0 is the original */
    for (i = 0; i < target->level_count; i++) {
        if (target->levels[i].data != NULL) {
            CHECK(pixel_image_destroy(&target->levels[i]));
        }
    }
    CHECK(memory_deallocate((data_pointer *)&target->levels));
    CHECK(memory_deallocate((data_pointer *)&target->roi));

    FINALLY(image_pyramid_destroy);
    /* make sure the state is set to invalid in all situations */
    target->state = INVALID;
    /* set the source image to NULL, it is not destroyed here */
    target->source = NULL;
    RETURN();
}

result image_pyramid_clone(
    image_pyramid *target,
    image_pyramid *source
    )
{
    TRY();
    CHECK_POINTER(source);

    CHECK(image_pyramid_create(target, source->source, source->level_count));

    FINALLY(image_pyramid_clone);
    RETURN();
}

result image_pyramid_copy(
    image_pyramid *target,
    image_pyramid *source
    )
{
    TRY();
    uint32 i;

    CHECK_POINTER(source);
    CHECK_POINTER(target);
    CHECK_POINTER(source->levels);
    CHECK_POINTER(target->levels);
    CHECK_PARAM(source->level_count == target->level_count);
    CHECK_PARAM(source->state != INVALID);
    CHECK_PARAM(target->state != INVALID);

    CHECK(pixel_image_copy(target->source, source->source));
    for (i = 0; i < target->level_count; i++) {
        CHECK(pixel_image_copy(&target->levels[i], &source->levels[i]));
    }

    FINALLY(image_pyramid_copy);
    RETURN();
}

result image_pyramid_down(
    image_pyramid *target
    )
{
    TRY();
    uint32 i, new_width, new_height;

    CHECK_POINTER(target);
    CHECK_POINTER(target->levels);
    CHECK_POINTER(target->roi);

    /* pyramid can be in any valid state in this function */
    CHECK_PARAM(target->state == INIT || target->state == DOWN || target->state == UP);

    new_width = target->width;
    new_height = target->height;

    /* copy data from source image to the first level */
    CHECK(pixel_image_copy(&target->levels[0], target->source));
    for (i = 1; i < target->level_count; i++) {
        target->levels[i].width = new_width;
        target->levels[i].height = new_height;
        CHECK(smooth_binomial(&target->levels[i - 1], &target->levels[i], 2));
        new_width = (uint32)(new_width / 2);
        new_height = (uint32)(new_height / 2);
        target->roi[i].width = new_width;
        target->roi[i].height = new_height;
        CHECK(scale_down(&target->levels[i], &target->roi[i]));
        target->levels[i].width = new_width;
        target->levels[i].height = new_height;
    }

    target->state = DOWN;
    FINALLY(image_pyramid_down);
    RETURN();
}

result image_pyramid_up(
    image_pyramid *target
    )
{
    TRY();
    uint32 i, j, new_width, new_height;

    CHECK_POINTER(target);

    /* if pyramid is not in DOWN state, it may not be scaled up */
    CHECK_PARAM(target->state == DOWN);

    for (i = 0; i < target->level_count; i++) {
        new_width = target->levels[i].width;
        new_height = target->levels[i].height;
        j = 0;
        /* scale up until the image has the same size as original */
        /* notice that level 0 will not be scaled */
        for (j = 0; j < i; j++) {
            new_width = new_width * 2;
            new_height = new_height * 2;
            target->levels[i].width = new_width;
            target->levels[i].height = new_height;
            CHECK(scale_up(&target->roi[i], &target->levels[i]));
            target->roi[i].width = new_width;
            target->roi[i].height = new_height;
        }
    }

    target->state = UP;
    FINALLY(image_pyramid_up);
    RETURN();
}

result image_pyramid_max(
    image_pyramid *pyramid,
    pixel_image *target
    )
{
    TRY();
    byte max, value;

    CHECK_POINTER(pyramid);
    CHECK_POINTER(target);
    CHECK_POINTER(target->data);
    CHECK_PARAM(pyramid->state == UP);
    CHECK_PARAM(target->type == p_U8);
    CHECK_PARAM(pyramid->width == target->width);
    CHECK_PARAM(pyramid->height == target->height);

    if (pixel_image_is_continuous(target)) {
        byte *target_data;
        uint32 i, pos, size;
        target_data = (byte *)target->data;
        /* pyramid images are always continuous */
        /* access by index is easiest with multiple levels */
        size = pyramid->height * pyramid->width;
        for (pos = 0; pos < size; pos++) {
            value = 0;
            max = 0;
            for (i = 0; i < pyramid->level_count; i++) {
                value = ((byte *)(*pyramid).levels[i].data)[pos];
                if (value > max) {
                    max = value;
                }
            }
            target_data[pos] = max;
        }
    }
    else {
        uint32 i, x, y, pos;
        SINGLE_DISCONTINUOUS_IMAGE_VARIABLES(target, byte);
        pos = 0;
        for (y = 0; y < pyramid->height; y++) {
            target_pos = target_rows[y];
            for (x = pyramid->width; x--; pos++, target_pos += target_step) {
                value = 0;
                max = 0;
                for (i = 0; i < pyramid->level_count; i++) {
                    value = ((byte *)(*pyramid).levels[i].data)[pos];
                    if (value > max) {
                        max = value;
                    }
                }
                PIXEL_VALUE(target) = max;
            }
        }
    }

    FINALLY(image_pyramid_max);
    RETURN();
}

result image_pyramid_min(
    image_pyramid *pyramid,
    pixel_image *target
    )
{
    TRY();
    byte min, value;

    CHECK_POINTER(pyramid);
    CHECK_POINTER(target);
    CHECK_POINTER(target->data);
    CHECK_PARAM(pyramid->state == UP);
    CHECK_PARAM(target->type == p_U8);
    CHECK_PARAM(pyramid->width == target->width);
    CHECK_PARAM(pyramid->height == target->height);

    if (pixel_image_is_continuous(target)) {
        byte *target_data;
        uint32 i, pos, size;
        target_data = (byte *)target->data;
        /* pyramid images are always continuous */
        /* access by index is easiest with multiple levels */
        size = pyramid->height * pyramid->width;
        for (pos = 0; pos < size; pos++) {
            value = 0;
            min = 255;
            for (i = 0; i < pyramid->level_count; i++) {
                value = ((byte *)(*pyramid).levels[i].data)[pos];
                if (value < min) min = value;
            }
            target_data[pos] = min;
        }
    }
    else {
        uint32 i, x, y, pos;
        SINGLE_DISCONTINUOUS_IMAGE_VARIABLES(target, byte);
        pos = 0;
        for (y = 0; y < pyramid->height; y++) {
            for (target_pos = target_rows[y],
                 x = pyramid->width; x--; pos++, target_pos += target_step) {
                value = 0;
                min = 255;
                for (i = 0; i < pyramid->level_count; i++) {
                    value = ((byte *)(*pyramid).levels[i].data)[pos];
                    if (value < min) {
                        min = value;
                    }
                }
                PIXEL_VALUE(target) = min;
            }
        }
    }

    FINALLY(image_pyramid_min);
    RETURN();
}

result edges_x_sobel_scale(
    image_pyramid *pyramid,
    pixel_image *temp,
    pixel_image *target,
    byte t
    )
{
    TRY();
    uint32 i;

    CHECK_POINTER(pyramid);
    CHECK_POINTER(temp);
    CHECK_POINTER(temp->data);
    CHECK_POINTER(target);
    CHECK_POINTER(target->data);
    CHECK_PARAM(pyramid->state == INIT || pyramid->state == DOWN || pyramid->state == UP);
    CHECK_PARAM(pyramid->width == temp->width);
    CHECK_PARAM(pyramid->height == temp->height);
    CHECK_PARAM(pyramid->width == target->width);
    CHECK_PARAM(pyramid->height == target->height);

    CHECK(image_pyramid_down(pyramid));

    for (i = 0; i < pyramid->level_count; i++) {
        temp->width = pyramid->levels[i].width;
        temp->height = pyramid->levels[i].height;
        CHECK(abs_sobel_x(&pyramid->levels[i], temp));
        CHECK(extrema_x(temp, temp));
        CHECK(normalize(temp, &pyramid->levels[i]));
    }
    CHECK(image_pyramid_up(pyramid));
    CHECK(image_pyramid_min(pyramid, target));
    CHECK(threshold(target, target, t));

    FINALLY(edges_x_sobel_scale);
    temp->width = pyramid->width;
    temp->height = pyramid->height;
    RETURN();
}
