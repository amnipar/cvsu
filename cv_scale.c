/**
 * @file cv_scale.c
 * @author Matti Eskelinen (matti dot j dot eskelinen at jyu dot fi)
 * @brief Scale-space handling and operations for the cv module.
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

#include "cv_scale.h"
#include "cv_filter.h"

#include <math.h>
/* for malloc; TODO: remove malloc, and this include after that */
#include <stdlib.h>

/*
create_image_pyramid
Allocates an array for storing an image pyramid
First level of the pyramid is given, other levels are allocated
Pyramid itself is not generated, only the data structure is initialized
Use pyramid_down to generate the pyramid by downscaling the src image
TODO: should deep-copy the source image instead of using a pointer to it
*/
result create_image_pyramid(image_pyramid *dst, pixel_image *src, long levels)
{
    long i;
    result r;
    dst->state = INVALID;

    if (dst == NULL || src == NULL || src->data == NULL) {
        return BAD_POINTER;
    }
    if (src->type != U8) {
        return BAD_TYPE;
    }
    long minsize = (long)pow(2.0, (double)levels);
    if ((levels <= 0) || (src->width < minsize) || (src->height < minsize)) {
        return BAD_PARAM;
    }
    dst->original = src;
    dst->level_count = levels;
    dst->width = src->width;
    dst->height = src->height;
    dst->step = src->step;

    /* TODO: remove malloc */
    dst->levels = (pixel_image *)malloc((size_t)levels * sizeof(pixel_image));
    if (dst->levels == NULL) {
        return BAD_POINTER;
    }
    dst->levels[0] = *src;
    for (i = 1; i < levels; i++) {
        (*dst).levels[i].data = NULL;
    }
    for (i = 1; i < levels; i++) {
        r = allocate_byte_image(&dst->levels[i], dst->width, dst->height, dst->step);
        if (r != SUCCESS) {
            return r;
        }
    }

    dst->state = INIT;
    return SUCCESS;
}

result destroy_image_pyramid(image_pyramid *dst)
{
    long i;
    if (dst == NULL) {
        return BAD_POINTER;
    }
    /*
    free the data in original image if not yet freed elsewhere
    */
    if (dst->original != NULL && dst->original->data != NULL) {
        destroy_image(dst->original);
        //free(pyramid->original->data);
        //pyramid->original->data = NULL;
    }
    /*
    free allocated pyramid levels - level 0 is the original
    */
    for (i = 1; i < dst->level_count; i++) {
        if ((*dst).levels[i].data != NULL) {
            destroy_image(&dst->levels[i]);
            //free((*pyramid).levels[i].data);
            //(*pyramid).levels[i].data = NULL;
        }
    }

    dst->state = INVALID;
    return SUCCESS;
}

result clone_image_pyramid(image_pyramid *dst, image_pyramid *src)
{
    /* TODO: implement */
    return SUCCESS;
}

result copy_image_pyramid(image_pyramid *dst, image_pyramid *src)
{
    /* TODO: implement */
    return SUCCESS;
}

/*
pyramid
Creates an image pyramid
Takes the image in position 0, smoothes with binomial filter, and scales down
*/
result pyramid_down(image_pyramid *dst)
{
    long i, new_width, new_height;
    result r;

    if (dst == NULL) {
        return BAD_POINTER;
    }
    /*
 pyramid can be in any valid state in this function
 */
    if (dst->state != INIT && dst->state != DOWN && dst->state != UP) {
        return BAD_PARAM;
    }

    new_width = dst->width;
    new_height = dst->height;

    for (i = 1; i < dst->level_count; i++) {
        (*dst).levels[i].width = new_width;
        (*dst).levels[i].height = new_height;
        r = smooth_binomial(&dst->levels[i - 1], &dst->levels[i], 2);
        if (r != SUCCESS) {
            return r;
        }
        r = scale_down(&dst->levels[i], &dst->levels[i]);
        new_width = (long)(new_width / 2);
        new_height = (long)(new_height / 2);
        (*dst).levels[i].width = new_width;
        (*dst).levels[i].height = new_height;
        if (r != SUCCESS) {
            return r;
        }
    }

    dst->state = DOWN;
    return SUCCESS;
}

/*
pyramid_up
Upscales an image pyramid back to original size
*/
result pyramid_up(image_pyramid *dst)
{
    long i, j, new_width, new_height;

    if (dst == NULL) {
        return BAD_POINTER;
    }
    /*
 if pyramid is not in DOWN state, it may not be scaled up
 */
    if (dst->state != DOWN) {
        return BAD_PARAM;
    }

    for (i = 0; i < dst->level_count; i++) {
        new_width = (*dst).levels[i].width;
        new_height = (*dst).levels[i].height;
        j = 0;
        /*
        scale up until the image has the same size as original
        level 0 will not be scaled
        */
        for (j = 0; j < i; j++) {
            scale_up(&dst->levels[i], &dst->levels[i]);
            new_width = new_width * 2;
            new_height = new_height * 2;
            (*dst).levels[i].width = new_width;
            (*dst).levels[i].height = new_height;
        }
    }

    dst->state = UP;
    return SUCCESS;
}

/*
max_scale
Creates an image that contains the maximal values occurring in scale space
*/
result pyramid_max(image_pyramid *pyramid, pixel_image *dst)
{
    byte *dst_data;
    long i, pos, width, height, size;
    byte max, value;

    if (pyramid == NULL || dst == NULL) {
        return BAD_POINTER;
    }
    if (pyramid->state != UP) {
        return BAD_PARAM;
    }
    if (dst->data == NULL) {
        return BAD_POINTER;
    }
    if (dst->type != U8) {
        return BAD_TYPE;
    }
    if (pyramid->width != dst->width || pyramid->height != dst->height) {
        return BAD_SIZE;
    }

    dst_data = (byte *)dst->data;
    width = pyramid->width;
    height = pyramid->height;

    size = width * height;
    for (pos = 0; pos < size; pos++) {
        value = 0;
        max = 0;
        for (i = 0; i < pyramid->level_count; i++) {
            value = ((byte *)(*pyramid).levels[i].data)[pos];
            if (value > max) max = value;
        }
        dst_data[pos] = max;
    }

    return SUCCESS;
}

/*
min_scale
Creates an image that contains the minimal values occurring in scale space
*/
result pyramid_min(image_pyramid *pyramid, pixel_image *dst)
{
    byte *dst_data;
    long i, pos, width, height, size;
    byte min, value;

    if (pyramid == NULL || dst == NULL) {
        return BAD_POINTER;
    }
    if (dst->data == NULL) {
        return BAD_POINTER;
    }
    if (pyramid->state != UP) {
        return BAD_PARAM;
    }
    if (dst->type != U8) {
        return BAD_TYPE;
    }
    if (pyramid->width != dst->width || pyramid->height != dst->height) {
        return BAD_SIZE;
    }

    dst_data = (byte *)dst->data;
    width = pyramid->width;
    height = pyramid->height;

    size = width * height;
    for (pos = 0; pos < size; pos++) {
        value = 0;
        min = 255;
        for (i = 0; i < pyramid->level_count; i++) {
            value = ((byte *)(*pyramid).levels[i].data)[pos];
            if (value < min) min = value;
        }
        dst_data[pos] = min;
    }

    return SUCCESS;
}

/*
edges_x_sobel_scale
Finds edges in horizontal direction using sobel operator in scale space
Applies sobel operator to each scale, taking the absolute value
Scales the pyramid images up to normal scale
Finds the minimal values across scales
Thresholds the images to reveal edge locations
Parameters:
pyramid - initialized image pyramid containing source image
temp - 32-bit integer (S32) image for temporary data
dst - 8-bit (U8) image for final result
t - threshold value for final result
*/
result edges_x_sobel_scale(image_pyramid *pyramid, pixel_image *temp, pixel_image *dst, byte t)
{
    long i;
    result r;

    if (pyramid == NULL || temp == NULL || dst == NULL) {
        return BAD_POINTER;
    }
    if (pyramid->state != INIT && pyramid->state != DOWN && pyramid->state != UP) {
        return BAD_PARAM;
    }
    if (temp->data == NULL || dst->data == NULL) {
        return BAD_POINTER;
    }
    if (pyramid->width != temp->width || pyramid->height != temp->height) {
        return BAD_SIZE;
    }

    r = SUCCESS;

    r = pyramid_down(pyramid);
    if (r != SUCCESS) {
        return r;
    }
    for (i = 0; i < pyramid->level_count; i++) {
        temp->width = (*pyramid).levels[i].width;
        temp->height = (*pyramid).levels[i].height;
        r = abs_sobel_x(&pyramid->levels[i], temp);
        if (r != SUCCESS) {
            goto finalize;
        }
        r = normalize(temp, &pyramid->levels[i]);
        if (r != SUCCESS) {
            goto finalize;
        }
    }
    r = pyramid_up(pyramid);
    if (r != SUCCESS) {
        goto finalize;
    }
    r = pyramid_min(pyramid, dst);
    if (r != SUCCESS) {
        goto finalize;
    }
    r = threshold(dst, dst, t);
    if (r != SUCCESS) {
        goto finalize;
    }

finalize:
    temp->width = pyramid->width;
    temp->height = pyramid->height;
    return r;
}
