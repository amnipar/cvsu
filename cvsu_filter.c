/**
 * @file cvsu_filter.c
 * @author Matti Eskelinen (matti dot j dot eskelinen at jyu dot fi)
 * @brief Basic image filters for the cvsu module.
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
#include "cvsu_filter.h"

/******************************************************************************/
/* constants for storing the function names                                   */
/* used in error reporting macros                                             */

string threshold_name = "threshold";
string smooth_binomial_name = "smooth_binomial";
string sobel_x_name = "sobel_x";
string abs_sobel_x_name = "abs_sobel_x";
string sobel_y_name = "sobel_y";
string abs_sobel_y_name = "abs_sobel_y";
string extrema_x_name = "extrema_x";
string extrema_y_name = "extrema_y";

/******************************************************************************/

result threshold(
    const pixel_image *source,
    pixel_image *target,
    byte t
    )
{
    TRY();
    byte value;

    CHECK_POINTER(source);
    CHECK_POINTER(target);
    CHECK_POINTER(source->data);
    CHECK_POINTER(target->data);
    CHECK_PARAM(source->type == p_U8);
    CHECK_PARAM(target->type == p_U8);
    CHECK_PARAM(source->width == target->width);
    CHECK_PARAM(source->height == target->height);

    if (pixel_image_is_continuous(source) && pixel_image_is_continuous(target)) {
        CONTINUOUS_IMAGE_VARIABLES(byte, byte);
        FOR_2_CONTINUOUS_IMAGES()
        {
            value = PIXEL_VALUE(source);
            if (value >= t) {
                PIXEL_VALUE(target) = 255;
            }
            else {
                PIXEL_VALUE(target) = 0;
            }
        }
    }
    else {
        uint32 x, y;
        DISCONTINUOUS_IMAGE_VARIABLES(byte, byte);
        FOR_2_DISCONTINUOUS_IMAGES()
        {
            value = PIXEL_VALUE(source);
            if (value >= t) {
                PIXEL_VALUE(target) = 255;
            }
            else {
                PIXEL_VALUE(target) = 0;
            }
        }
    }

    FINALLY(threshold);
    RETURN();
}

/******************************************************************************/

result smooth_binomial(
    const pixel_image *source,
    pixel_image *target,
    uint32 passes
    )
{
    TRY();

    CHECK_POINTER(source);
    CHECK_POINTER(target);
    CHECK_POINTER(source->data);
    CHECK_POINTER(target->data);
    CHECK_PARAM(source->type == p_U8);
    CHECK_PARAM(target->type == p_U8);
    CHECK_PARAM(source->width == target->width);
    CHECK_PARAM(source->height == target->height);

    /* copy source image data for 'pass 0' result */
    CHECK(pixel_image_copy(target, source));

    {
        uint32 x, y, pass, target_stride;
        byte prev, curr, next;
        SINGLE_DISCONTINUOUS_IMAGE_VARIABLES(target, byte);
        target_stride = target->stride;

        /* smooth out the 'pass 0' result with binomial filter */
        for (pass = 0; pass < passes; pass++) {
            /* filter row by row */
            FOR_DISCONTINUOUS_IMAGE_ROW(target)
            {
                prev = PIXEL_VALUE(target);
                target_pos += target_step;
                curr = PIXEL_VALUE(target);
                next = PIXEL_VALUE_PLUS(target, target_step);
                /* skipping first and last column */
                for (x = target->width - 2; x--; ) {
                    /*
                    apply binomial mask [ 1/4 1/2 1/4 ]
                    */
                    PIXEL_VALUE(target) = (byte)((prev >> 2) + (curr >> 1) + (next >> 2));
                    prev = curr;
                    curr = next;
                    target_pos += target_step;
                    next = PIXEL_VALUE_PLUS(target, target_step);
                }
            }
            /* filter column by column */
            FOR_DISCONTINUOUS_IMAGE_COL(target)
            {
                prev = PIXEL_VALUE(target);
                target_pos += target_stride;
                curr = PIXEL_VALUE(target);
                next = PIXEL_VALUE_PLUS(target, target_stride);
                /* skipping first and last row */
                for (y = target->height - 2; y--; ) {
                    /*
                    apply binomial mask [ 1/4 1/2 1/4 ]
                    */
                    PIXEL_VALUE(target) = (byte)((prev >> 2) + (curr >> 1) + (next >> 2));
                    prev = curr;
                    curr = next;
                    target_pos += target_stride;
                    next = PIXEL_VALUE_PLUS(target, target_stride);
                }
            }
        }
    }

    FINALLY(smooth_binomial);
    RETURN();
}

/******************************************************************************/

result sobel_x(
    const pixel_image *source,
    pixel_image *target
    )
{
    TRY();
    sint32 sobeloffsets[9];
    /* mask values used in convolution */
    /* sint32 sobelmaskx[9] = { -1, 0, 1, -2, 0, 2, -1, 0, 1 }; */
    sint32 i, j;

    CHECK_POINTER(source);
    CHECK_POINTER(target);
    CHECK_POINTER(source->data);
    CHECK_POINTER(target->data);
    CHECK_PARAM(source->type == p_U8);
    CHECK_PARAM(target->type == p_S32);
    CHECK_PARAM(source->width == target->width);
    CHECK_PARAM(source->height == target->height);

    /* generate offsets for the sobel mask */
    for (i = -1; i <= 1; i++) {
        for (j = -1; j <= 1; j++) {
            sobeloffsets[((unsigned)(j + 1) * 3 + (unsigned)(i + 1))] =
                    j * (signed)source->stride + i * (signed)source->step;
        }
    }

    pixel_image_clear(target);
    {
        uint32 x, y, x_end, y_end;
        long value;
        DISCONTINUOUS_IMAGE_VARIABLES(byte, long);
        y_end = source->height - 1;
        x_end = source->width - 2;

        for (y = 1; y < y_end; y++) {
            source_pos = source_rows[y] + source_step;
            target_pos = target_rows[y] + target_step;
            for (x = x_end; x--; source_pos += source_step, target_pos += target_step) {
                /* flatten the sobel mask operation to optimize */
                value  = ((PIXEL_VALUE_PLUS(source, sobeloffsets[2]))
                        - (PIXEL_VALUE_PLUS(source, sobeloffsets[0])));
                value += ((PIXEL_VALUE_PLUS(source, sobeloffsets[5]) * 2)
                        - (PIXEL_VALUE_PLUS(source, sobeloffsets[3]) * 2));
                value += ((PIXEL_VALUE_PLUS(source, sobeloffsets[8]))
                        - (PIXEL_VALUE_PLUS(source, sobeloffsets[6])));
                /* the unoptimized code using a loop: */
                /*
                value = 0;
                for (i = 9; i--; ) {
                    value += (PIXEL_VALUE_PLUS(source, sobeloffsets[i]) * sobelmaskx[i]);
                }
                */
                PIXEL_VALUE(target) = value;
            }
        }
    }

    FINALLY(sobel_x);
    RETURN();
}

/******************************************************************************/

result abs_sobel_x(
    const pixel_image *source,
    pixel_image *target
    )
{
    TRY();
    sint32 sobeloffsets[9];
    /* mask values used in convolution */
    /* sint32 sobelmaskx[9] = { -1, 0, 1, -2, 0, 2, -1, 0, 1 }; */
    sint32 i, j;

    CHECK_POINTER(source);
    CHECK_POINTER(target);
    CHECK_POINTER(source->data);
    CHECK_POINTER(target->data);
    CHECK_PARAM(source->type == p_U8);
    CHECK_PARAM(target->type == p_S32);
    CHECK_PARAM(source->width == target->width);
    CHECK_PARAM(source->height == target->height);

    for (i = -1; i <= 1; i++) {
        for (j = -1; j <= 1; j++) {
            sobeloffsets[(unsigned)(j + 1) * 3 + (unsigned)(i + 1)] =
                    j * (signed)source->stride + i * (signed)source->step;
        }
    }

    pixel_image_clear(target);
    {
        uint32 x, y, x_end, y_end;
        long value;
        DISCONTINUOUS_IMAGE_VARIABLES(byte, long);
        y_end = source->height - 1;
        x_end = source->width - 2;

        for (y = 1; y < y_end; y++) {
            source_pos = source_rows[y] + source_step;
            target_pos = target_rows[y] + target_step;
            for (x = x_end; x--; source_pos += source_step, target_pos += target_step) {
                /* flatten the sobel mask operation to optimize */
                value  = ((PIXEL_VALUE_PLUS(source, sobeloffsets[2]))
                        - (PIXEL_VALUE_PLUS(source, sobeloffsets[0])));
                value += ((PIXEL_VALUE_PLUS(source, sobeloffsets[5]) * 2)
                        - (PIXEL_VALUE_PLUS(source, sobeloffsets[3]) * 2));
                value += ((PIXEL_VALUE_PLUS(source, sobeloffsets[8]))
                        - (PIXEL_VALUE_PLUS(source, sobeloffsets[6])));
                /* the unoptimized code using a loop: */
                /*
                value = 0;
                for (i = 9; i--; ) {
                    value += (PIXEL_VALUE_PLUS(source, sobeloffsets[i]) * sobelmaskx[i]);
                }
                */
                value = (value < 0) ? -value : value;
                PIXEL_VALUE(target) = value;
            }
        }
    }

    FINALLY(abs_sobel_x);
    RETURN();
}

/******************************************************************************/

result sobel_y(
    const pixel_image *source,
    pixel_image *target
    )
{
    TRY();
    sint32 sobeloffsets[9];
    /* mask values used in convolution */
    /* sint32 sobelmaskx[9] = { -1, -2, -1, 0, 0, 0, 1, 2, 1 }; */
    sint32 i, j;

    CHECK_POINTER(source);
    CHECK_POINTER(target);
    CHECK_POINTER(source->data);
    CHECK_POINTER(target->data);
    CHECK_PARAM(source->type == p_U8);
    CHECK_PARAM(target->type == p_S32);
    CHECK_PARAM(source->width == target->width);
    CHECK_PARAM(source->height == target->height);

    /* generate offsets for the sobel mask */
    for (i = -1; i <= 1; i++) {
        for (j = -1; j <= 1; j++) {
            sobeloffsets[(unsigned)(j + 1) * 3 + (unsigned)(i + 1)] =
                    j * (signed)source->stride + i * (signed)source->step;
        }
    }

    pixel_image_clear(target);
    {
        uint32 x, y, x_end, y_end;
        long value;
        DISCONTINUOUS_IMAGE_VARIABLES(byte, long);
        y_end = source->height - 1;
        x_end = source->width - 2;

        for (y = 1; y < y_end; y++) {
            source_pos = source_rows[y] + source_step;
            target_pos = target_rows[y] + target_step;
            for (x = x_end; x--; source_pos += source_step, target_pos += target_step) {
                /* flatten the sobel mask operation to optimize */
                value  = ((PIXEL_VALUE_PLUS(source, sobeloffsets[6]))
                        - (PIXEL_VALUE_PLUS(source, sobeloffsets[0])));
                value += ((PIXEL_VALUE_PLUS(source, sobeloffsets[7]) * 2)
                        - (PIXEL_VALUE_PLUS(source, sobeloffsets[1]) * 2));
                value += ((PIXEL_VALUE_PLUS(source, sobeloffsets[8]))
                        - (PIXEL_VALUE_PLUS(source, sobeloffsets[2])));
                /* the unoptimized code using a loop: */
                /*
                value = 0;
                for (i = 9; i--; ) {
                    value += (PIXEL_VALUE_PLUS(source, sobeloffsets[i]) * sobelmaskx[i]);
                }
                */
                PIXEL_VALUE(target) = value;
            }
        }
    }

    FINALLY(sobel_x);
    RETURN();
}

/******************************************************************************/

result abs_sobel_y(
    const pixel_image *source,
    pixel_image *target
    )
{
    TRY();
    sint32 sobeloffsets[9];
    /* mask values used in convolution */
    /* sint32 sobelmaskx[9] = { -1, -2, -1, 0, 0, 0, 1, 2, 1 }; */
    sint32 i, j;

    CHECK_POINTER(source);
    CHECK_POINTER(target);
    CHECK_POINTER(source->data);
    CHECK_POINTER(target->data);
    CHECK_PARAM(source->type == p_U8);
    CHECK_PARAM(target->type == p_S32);
    CHECK_PARAM(source->width == target->width);
    CHECK_PARAM(source->height == target->height);

    for (i = -1; i <= 1; i++) {
        for (j = -1; j <= 1; j++) {
            sobeloffsets[(unsigned)(j + 1) * 3 + (unsigned)(i + 1)] =
                    j * (signed)source->stride + i * (signed)source->step;
        }
    }

    pixel_image_clear(target);
    {
        uint32 x, y, x_end, y_end;
        long value;
        DISCONTINUOUS_IMAGE_VARIABLES(byte, long);
        y_end = source->height - 1;
        x_end = source->width - 2;

        for (y = 1; y < y_end; y++) {
            source_pos = source_rows[y] + source_step;
            target_pos = target_rows[y] + target_step;
            for (x = x_end; x--; source_pos += source_step, target_pos += target_step) {
                /* flatten the sobel mask operation to optimize */
                value  = ((PIXEL_VALUE_PLUS(source, sobeloffsets[6]))
                        - (PIXEL_VALUE_PLUS(source, sobeloffsets[0])));
                value += ((PIXEL_VALUE_PLUS(source, sobeloffsets[7]) * 2)
                        - (PIXEL_VALUE_PLUS(source, sobeloffsets[1]) * 2));
                value += ((PIXEL_VALUE_PLUS(source, sobeloffsets[8]))
                        - (PIXEL_VALUE_PLUS(source, sobeloffsets[2])));
                /* the unoptimized code using a loop: */
                /*
                  value = 0;
                for (i = 9; i--; ) {
                    value += (PIXEL_VALUE_PLUS(source, sobeloffsets[i]) * sobelmaskx[i]);
                }
                */
                value = (value < 0) ? -value : value;
                PIXEL_VALUE(target) = value;
            }
        }
    }

    FINALLY(abs_sobel_y);
    RETURN();
}

/******************************************************************************/

result extrema_x(
    const pixel_image *source,
    pixel_image *target
    )
{
    TRY();

    CHECK_POINTER(source);
    CHECK_POINTER(target);
    CHECK_POINTER(source->data);
    CHECK_POINTER(target->data);
    CHECK_PARAM(source->type == p_S32);
    CHECK_PARAM(target->type == p_S32);
    CHECK_PARAM(source->width == target->width);
    CHECK_PARAM(source->height == target->height);

    {
        uint32 x, y, x_end, y_end;
        long value, prev;
        bool is_rising, is_falling;
        DISCONTINUOUS_IMAGE_VARIABLES(long, long);
        y_end = source->height;
        x_end = source->width - 2;

        for (y = y_end; y--; ) {
            /* keep target one step behind source */
            source_pos = source_rows[y] + 2 * source_step;
            /* this is because target is always updated by previous value */
            target_pos = target_rows[y] + 1 * target_step;
            /* initialize direction and prev value */
            value = PIXEL_VALUE_MINUS(source, 1);
            prev = PIXEL_VALUE_MINUS(source, 2);
            if (value < prev) {
                is_falling = true;
                is_rising = false;
            }
            else {
                is_falling = false;
                if (value > prev) {
                    is_rising = true;
                }
                else {
                    is_rising = false;
                }
            }
            prev = value;
            /* now find extrema row by row */
            for (x = x_end; x--; source_pos += source_step, target_pos += target_step) {
                value = PIXEL_VALUE(source);
                if (value < prev) {
                    if (is_rising) {
                        PIXEL_VALUE(target) = value;
                        is_rising = false;
                    }
                    else {
                        PIXEL_VALUE(target) = 0;
                    }
                    is_falling = true;
                }
                else if (value > prev) {
                    if (is_falling) {
                        PIXEL_VALUE(target) = value;
                        is_falling = false;
                    }
                    else {
                        PIXEL_VALUE(target) = 0;
                    }
                    is_rising = true;
                }
                else {
                    PIXEL_VALUE(target) = 0;
                }
                prev = value;
            }
        }
    }

    FINALLY(extrema_x);
    RETURN();
}

/******************************************************************************/

result extrema_y(
    const pixel_image *source,
    pixel_image *target
    )
{
    TRY();

    CHECK_POINTER(source);
    CHECK_POINTER(target);
    CHECK_POINTER(source->data);
    CHECK_POINTER(target->data);
    CHECK_PARAM(source->type == p_S32);
    CHECK_PARAM(target->type == p_S32);
    CHECK_PARAM(source->width == target->width);
    CHECK_PARAM(source->height == target->height);

    {
        uint32 x, y, x_end, y_end, source_stride, target_stride;
        long value, prev;
        bool is_rising, is_falling;
        DISCONTINUOUS_IMAGE_VARIABLES(long, long);
        x_end = source->width;
        y_end = source->height - 2;
        source_stride = source->stride;
        target_stride = target->stride;

        for (x = x_end; x--; ) {
            /* keep target one step behind source */
            source_pos = source_rows[2] + x * source_step;
            /* this is because target is always updated by previous value */
            target_pos = target_rows[1] + x * target_step;
            /* initialize direction and prev value */
            value = PIXEL_VALUE_MINUS(source, 1 * source_stride);
            prev = PIXEL_VALUE_MINUS(source, 2 * source_stride);
            if (value < prev) {
                is_falling = true;
                is_rising = false;
            }
            else {
                is_falling = false;
                if (value > prev) {
                    is_rising = true;
                }
                else {
                    is_rising = false;
                }
            }
            prev = value;
            /* now find extrema col by col */
            for (y = y_end; y--; source_pos += source_stride, target_pos += target_stride) {
                value = PIXEL_VALUE(source);
                if (value < prev) {
                    if (is_rising) {
                        PIXEL_VALUE(target) = value;
                        is_rising = false;
                    }
                    else {
                        PIXEL_VALUE(target) = 0;
                    }
                    is_falling = true;
                }
                else if (value > prev) {
                    if (is_falling) {
                        PIXEL_VALUE(target) = value;
                        is_falling = false;
                    }
                    else {
                        PIXEL_VALUE(target) = 0;
                    }
                    is_rising = true;
                }
                else {
                    PIXEL_VALUE(target) = 0;
                }
                prev = value;
            }
        }
    }

    FINALLY(extrema_y);
    RETURN();
}

/******************************************************************************/
