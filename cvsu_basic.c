/**
 * @file cvsu_basic.c
 * @author Matti Eskelinen (matti dot j dot eskelinen at jyu dot fi)
 * @brief Basic types and operations for the cvsu module.
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
#include "cvsu_basic.h"

#include <math.h>
#include <limits.h>

/******************************************************************************/
/* constants for storing the function names                                   */
/* used in error reporting macros                                             */

string pixel_image_init_name = "pixel_image_init";
string pixel_image_create_name = "pixel_image_create";
string pixel_image_destroy_name = "pixel_image_destroy";
string pixel_image_create_roi_name = "pixel_image_create_roi";
string pixel_image_clone_name = "pixel_image_clone";
string pixel_image_copy_name = "pixel_image_copy";
string pixel_image_clear_name = "pixel_image_clear";
string integral_image_create_name = "integral_image_create";
string integral_image_destroy_name = "integral_image_destroy";
string integral_image_clone_name = "integral_image_clone";
string integral_image_copy_name = "integral_image_copy";
string integral_image_update_name = "integral_image_update";
string normalize_name = "normalize";
string normalize_byte_name = "normalize_byte";
string normalize_char_name = "normalize_char";
string normalize_word_name = "normalize_word";
string normalize_long_name = "normalize_long";
string normalize_double_name = "normalize_double";
string scale_down_name = "scale_down";
string scale_up_name = "scale_up";
string convert_grey8_to_grey24_name = "convert_grey8_to_grey24";
string convert_rgb24_to_grey8_name = "convert_rgb24_to_grey8";

/******************************************************************************/
/* constants for lookup tables                                                */

bool tables_initialized = false;
double pixel_squared[256];

/******************************************************************************/

void init_tables()
{
    uint32 i;
    if (!tables_initialized) {
        for (i = 0; i < 256; i++) {
            pixel_squared[i] = (double)(i * i);
        }
        tables_initialized = true;
    }
}

/******************************************************************************/
/* private function for initializing the pixel_image structure                */

result pixel_image_init(
    pixel_image *target,
    pointer data,
    pixel_type type,
    pixel_format format,
    uint32 dx,
    uint32 dy,
    uint32 width,
    uint32 height,
    uint32 offset,
    uint32 step,
    uint32 stride,
    uint32 size)
{
    TRY();
    uint32 i;
#if IMAGE_ACCESS_METHOD == IMAGE_ACCESS_BY_POINTER
    byte **row;
    size_t pixel_size;
#endif

    init_tables();

    CHECK_POINTER(target);

    /* store data in image so we have the pointer for freeing the data */
    /* in case other parameters are invalid */
    target->data = data;

    CHECK_POINTER(target->data);
    CHECK_PARAM((dx + width) * step <= stride);
    CHECK_PARAM((dy + height) * stride <= size);

    target->parent = NULL;
    target->type = type;
    target->format = format;
    target->dx = dx;
    target->dy = dy;
    target->width = width;
    target->height = height;
    target->offset = offset;
    target->step = step;
    target->stride = stride;
    target->size = size;

#if IMAGE_ACCESS_METHOD == IMAGE_ACCESS_BY_INDEX
    CHECK(memory_allocate((data_pointer *)&target->rows, height, sizeof(uint32)));
    for (i = 0; i < target->height; i++) {
        target->rows[i] = (target->dy + i) * target->stride + target->dx * target->step + target->offset;
    }
#elif IMAGE_ACCESS_METHOD == IMAGE_ACCESS_BY_POINTER
    CHECK(memory_allocate((data_pointer *)&target->rows, height, sizeof(data_pointer *)));
    switch (type) {
    case p_U8:
        pixel_size = sizeof(uint8);
        break;
    case p_S8:
        pixel_size = sizeof(sint8);
        break;
    case p_U16:
        pixel_size = sizeof(uint16);
        break;
    case p_S32:
        pixel_size = sizeof(sint32);
        break;
    case p_F64:
        pixel_size = sizeof(real64);
        break;
    default:
        ERROR(BAD_TYPE);
    }
    for (i = 0, row = (data_pointer *)target->rows; i < target->height; i++, row++) {
        *row = (data_pointer)target->data + ((target->dy + i) * target->stride + target->dx * target->step + target->offset) * pixel_size;
    }
#endif

    FINALLY(pixel_image_init);
    RETURN();
}

/******************************************************************************/

result pixel_image_create(
    pixel_image *target,
    pixel_type type,
    pixel_format format,
    uint32 width,
    uint32 height,
    uint32 step,
    uint32 stride
    )
{
    TRY();
    data_pointer data;
    uint32 size;

    CHECK_POINTER(target);

    size = height * stride;
    switch (type) {
    case p_U8:
        CHECK(memory_allocate(&data, size, sizeof(uint8)));
        break;
    case p_S8:
        CHECK(memory_allocate(&data, size, sizeof(sint8)));
        break;
    case p_U16:
        CHECK(memory_allocate(&data, size, sizeof(uint16)));
        break;
    case p_S32:
        CHECK(memory_allocate(&data, size, sizeof(sint32)));
        break;
    case p_F64:
        CHECK(memory_allocate(&data, size, sizeof(real64)));
        break;
    default:
        ERROR(BAD_TYPE);
    }

    CHECK(pixel_image_init(target, data, type, format, 0, 0, width, height, 0, step, stride, size));

    FINALLY(pixel_image_create);
    RETURN();
}

/******************************************************************************/

result pixel_image_destroy(
    pixel_image *target
    )
{
    TRY();

    CHECK_POINTER(target);

    /* don't delete if target has a parent, that's parent's responsibility */
    if (target->parent == NULL) {
        CHECK(memory_deallocate((data_pointer *)&target->data));
    }
    CHECK(memory_deallocate((data_pointer *)&target->rows));

    FINALLY(pixel_image_destroy);
    RETURN();
}

/******************************************************************************/

result pixel_image_create_roi(
    pixel_image *target,
    pixel_image *source,
    uint32 dx,
    uint32 dy,
    uint32 width,
    uint32 height
    )
{
    TRY();

    CHECK_POINTER(source);

    /* set data as NULL, in case create_image fails to set the pointer */
    target->data = NULL;
    CHECK(pixel_image_init(target, source->data, source->type, source->format,
                           dx, dy, width, height,
                           source->offset, source->step, source->stride, source->size));

    FINALLY(pixel_image_create_roi);
    /* set parent here, as create_image has to set it as NULL */
    target->parent = source;
    target->format = source->format;
    RETURN();
}

/******************************************************************************/

result pixel_image_clone(
    pixel_image *target,
    pixel_image *source
    )
{
    TRY();
    CHECK_POINTER(source);

    CHECK(pixel_image_create(target, source->type, source->format, source->width, source->height, source->step, source->stride));

    FINALLY(pixel_image_clone);
    RETURN();
}

/******************************************************************************/

result pixel_image_copy(
    pixel_image *target,
    const pixel_image *source
    )
{
    TRY();

    CHECK_POINTER(source);
    CHECK_POINTER(target);
    CHECK_POINTER(source->data);
    CHECK_POINTER(target->data);
    CHECK_PARAM(source->type == target->type);
    CHECK_PARAM(source->format == target->format);
    CHECK_PARAM(source->width == target->width);
    CHECK_PARAM(source->height == target->height);
    CHECK_PARAM(source->step == target->step);

    if (pixel_image_is_continuous(source) && pixel_image_is_continuous(target)) {
        size_t pixel_size;
        switch (source->type) {
        case p_U8:
            pixel_size = sizeof(uint8);
            break;
        case p_S8:
            pixel_size = sizeof(sint8);
            break;
        case p_U16:
            pixel_size = sizeof(uint16);
            break;
        case p_S32:
            pixel_size = sizeof(sint32);
            break;
        case p_F64:
            pixel_size = sizeof(real64);
            break;
        default:
            ERROR(BAD_TYPE);
        }
        memory_copy(target->data, source->data, source->size, pixel_size);
    }
    else {
        uint32 x, y;
        switch (source->type) {
        case p_U8:
            {
                DISCONTINUOUS_IMAGE_VARIABLES(byte, byte);
                FOR_2_DISCONTINUOUS_IMAGE_ROWS()
                    memory_copy((data_pointer)POINTER_TO_PIXEL(target),
                                (data_pointer)POINTER_TO_PIXEL(source),
                                source->width * source_step, sizeof(byte));
            }
            break;
        case p_S8:
            {
                DISCONTINUOUS_IMAGE_VARIABLES(char, char);
                FOR_2_DISCONTINUOUS_IMAGE_ROWS()
                    memory_copy((data_pointer)POINTER_TO_PIXEL(target),
                                (data_pointer)POINTER_TO_PIXEL(source),
                                source->width * source_step, sizeof(char));
            }
            break;
        case p_U16:
            {
                DISCONTINUOUS_IMAGE_VARIABLES(word, word);
                FOR_2_DISCONTINUOUS_IMAGE_ROWS()
                    memory_copy((data_pointer)POINTER_TO_PIXEL(target),
                                (data_pointer)POINTER_TO_PIXEL(source),
                                source->width * source_step, sizeof(word));
            }
            break;
        case p_S32:
            {
                DISCONTINUOUS_IMAGE_VARIABLES(long, long);
                FOR_2_DISCONTINUOUS_IMAGE_ROWS()
                    memory_copy((data_pointer)POINTER_TO_PIXEL(target),
                                (data_pointer)POINTER_TO_PIXEL(source),
                                source->width * source_step, sizeof(long));
            }
            break;
        case p_F64:
            {
                DISCONTINUOUS_IMAGE_VARIABLES(double, double);
                FOR_2_DISCONTINUOUS_IMAGE_ROWS()
                    memory_copy((data_pointer)POINTER_TO_PIXEL(target),
                                (data_pointer)POINTER_TO_PIXEL(source),
                                source->width * source_step, sizeof(double));
            }
            break;
        default:
            ERROR(BAD_TYPE);
        }
    }

    FINALLY(pixel_image_copy);
    RETURN();
}

/******************************************************************************/

result pixel_image_clear(
    pixel_image *target
    )
{
    TRY();

    CHECK_POINTER(target);
    CHECK_POINTER(target->data);

    if (pixel_image_is_continuous(target)) {
        size_t pixel_size;
        switch (target->type) {
        case p_U8:
            pixel_size = sizeof(uint8);
            break;
        case p_S8:
            pixel_size = sizeof(sint8);
            break;
        case p_U16:
            pixel_size = sizeof(uint16);
            break;
        case p_S32:
            pixel_size = sizeof(sint32);
            break;
        case p_F64:
            pixel_size = sizeof(real64);
            break;
        default:
            ERROR(BAD_TYPE);
        }
        memory_clear((data_pointer)target->data, target->size, pixel_size);
    }
    else {
        uint32 x, y;
        switch (target->type) {
        case p_U8:
            {
                SINGLE_DISCONTINUOUS_IMAGE_VARIABLES(target, byte);
                FOR_DISCONTINUOUS_IMAGE_ROW(target)
                    memory_clear((data_pointer)POINTER_TO_PIXEL(target),
                                 target->width * target_step, sizeof(byte));
            }
            break;
        case p_S8:
            {
                SINGLE_DISCONTINUOUS_IMAGE_VARIABLES(target, char);
                FOR_DISCONTINUOUS_IMAGE_ROW(target)
                    memory_clear((data_pointer)POINTER_TO_PIXEL(target),
                                 target->width * target_step, sizeof(char));
            }
            break;
        case p_U16:
            {
                SINGLE_DISCONTINUOUS_IMAGE_VARIABLES(target, word);
                FOR_DISCONTINUOUS_IMAGE_ROW(target)
                    memory_clear((data_pointer)POINTER_TO_PIXEL(target),
                                 target->width * target_step, sizeof(word));
            }
            break;
        case p_S32:
            {
                SINGLE_DISCONTINUOUS_IMAGE_VARIABLES(target, long);
                FOR_DISCONTINUOUS_IMAGE_ROW(target)
                    memory_clear((data_pointer)POINTER_TO_PIXEL(target),
                                 target->width * target_step, sizeof(long));
            }
            break;
        case p_F64:
            {
                SINGLE_DISCONTINUOUS_IMAGE_VARIABLES(target, double);
                FOR_DISCONTINUOUS_IMAGE_ROW(target)
                    memory_clear((data_pointer)POINTER_TO_PIXEL(target),
                                 target->width * target_step, sizeof(double));
            }
            break;
        default:
            ERROR(BAD_TYPE);
        }
    }

    FINALLY(pixel_image_clear);
    RETURN();
}

/******************************************************************************/

bool pixel_image_is_continuous(const pixel_image *image)
{
    if (image == NULL) {
        return false;
    }
    if (image->width * image->step != image->stride) {
        return false;
    }
    if (image->dx > 0 || image->dy > 0) {
        return false;
    }
    return true;
}

/******************************************************************************/

result integral_image_create(
    integral_image *target,
    pixel_image *source
    )
{
    TRY();

    CHECK_POINTER(target);
    CHECK_POINTER(source);
    CHECK_POINTER(source->data);
    CHECK_PARAM(source->type == p_U8);

    target->original = source;
    target->width = source->width;
    target->height = source->height;
    target->step = source->step;
    target->stride = target->width * target->step;
    target->I_1.data = NULL;
    target->I_2.data = NULL;

    CHECK(pixel_image_create(&target->I_1, p_S32, GREY,
            target->width, target->height, target->step, target->stride));

    CHECK(pixel_image_create(&target->I_2, p_F64, GREY,
            target->width, target->height, target->step, target->stride));

    FINALLY(integral_image_create);
    RETURN();
}

/******************************************************************************/

result integral_image_destroy(
    integral_image *target
    )
{
    TRY();

    CHECK_POINTER(target);

    CHECK(memory_deallocate((data_pointer *)&target->I_1.data));
    CHECK(memory_deallocate((data_pointer *)&target->I_2.data));

    FINALLY(integral_image_destroy);
    RETURN();
}

/******************************************************************************/

result integral_image_clone(
    integral_image *target,
    integral_image *source
    )
{
    TRY();

    CHECK_POINTER(target);
    CHECK_POINTER(source);

    /* clone will use the same original image */
    target->original = source->original;
    target->width = source->width;
    target->height = source->height;
    target->step = source->step;

    CHECK(pixel_image_clone(&target->I_1, &source->I_1));
    CHECK(pixel_image_clone(&target->I_2, &source->I_2));

    FINALLY(integral_image_clone);
    RETURN();
}

/******************************************************************************/

result integral_image_copy(
    integral_image *target,
    integral_image *source
    )
{
    TRY();

    CHECK_POINTER(target);
    CHECK_POINTER(source);
    CHECK_PARAM(target != source);
    CHECK_PARAM(source->width == target->width);
    CHECK_PARAM(source->height == target->height);
    CHECK_PARAM(source->step == target->step);

    if (target->original != source->original) {
        CHECK(pixel_image_copy(target->original, source->original));
    }
    CHECK(pixel_image_copy(&target->I_1, &source->I_1));
    CHECK(pixel_image_copy(&target->I_2, &source->I_2));

    FINALLY(integral_image_copy);
    RETURN();
}

/******************************************************************************/
/* private macros for generating the integral_image_update function           */

#if (IMAGE_ACCESS_METHOD == IMAGE_ACCESS_BY_INDEX)

#define INTEGRAL_IMAGE_UPDATE_DEFINE_VARIABLES()\
    I_1_t *I_1_data;\
    I_2_t *I_2_data;\
    uint32 current_pos

#define INTEGRAL_IMAGE_SET_POS(offset)\
    current_pos = (offset)

#define INTEGRAL_IMAGE_ADVANCE_POS(offset)\
    current_pos += (offset)

#define I_1_GET_VALUE() (I_1_data[current_pos])
#define I_1_SET_VALUE(value) I_1_data[current_pos] = (value)
#define I_1_GET_VALUE_WITH_OFFSET(offset) (I_1_data[current_pos - (offset)])
#define I_2_GET_VALUE() (I_2_data[current_pos])
#define I_2_SET_VALUE(value) I_2_data[current_pos] = (value)
#define I_2_GET_VALUE_WITH_OFFSET(offset) (I_2_data[current_pos - (offset)])

#elif (IMAGE_ACCESS_METHOD == IMAGE_ACCESS_BY_POINTER)

#define INTEGRAL_IMAGE_UPDATE_DEFINE_VARIABLES()\
    I_1_t *I_1_data, *I_1_pos;\
    I_2_t *I_2_data, *I_2_pos

#define INTEGRAL_IMAGE_SET_POS(offset)\
    I_1_pos = I_1_data + (offset);\
    I_2_pos = I_2_data + (offset)

#define INTEGRAL_IMAGE_ADVANCE_POS(offset)\
    I_1_pos += (offset);\
    I_2_pos += (offset)

#define I_1_GET_VALUE() (*I_1_pos)
#define I_1_SET_VALUE(value) *I_1_pos = (value)
#define I_1_GET_VALUE_WITH_OFFSET(offset) (*(I_1_pos - (offset)))
#define I_2_GET_VALUE() (*I_2_pos)
#define I_2_SET_VALUE(value) *I_2_pos = (value)
#define I_2_GET_VALUE_WITH_OFFSET(offset) (*(I_2_pos - (offset)))

#else
#error "Image access method not defined"
#endif

/******************************************************************************/

result integral_image_update(
    integral_image *target
    )
{
    TRY();
    pixel_image *source;

    CHECK_POINTER(target);
    CHECK_POINTER(target->original);
    CHECK_POINTER(target->I_1.data);
    CHECK_POINTER(target->I_2.data);

    source = target->original;
    {
        INTEGRAL_IMAGE_UPDATE_DEFINE_VARIABLES();
        uint32 intensity, width, height, x, y, h, v, d;
        SINGLE_DISCONTINUOUS_IMAGE_VARIABLES(source, byte);

        I_1_data = (I_1_t *)target->I_1.data;
        I_2_data = (I_2_t *)target->I_2.data;
        width = target->width;
        height = target->height;
        /* horizontal offset for integral images */
        h = 1;
        /* vertical offset for integral images */
        v = width;
        /* diagonal offset for integral images */
        d = width + 1;
        /* initialize top left corner of integral images */
        {
            source_pos = source_rows[0];
            INTEGRAL_IMAGE_SET_POS(0);
            intensity = PIXEL_VALUE(source);
            I_1_SET_VALUE(intensity);
            I_2_SET_VALUE(pixel_squared[intensity]);

            /* initialize topmost row of integral images */
            for (x = width - 1; x--; ) {
                source_pos += source_step;
                intensity = PIXEL_VALUE(source);
                INTEGRAL_IMAGE_ADVANCE_POS(h);
                I_1_SET_VALUE(I_1_GET_VALUE_WITH_OFFSET(h) +
                              intensity);
                I_2_SET_VALUE(I_2_GET_VALUE_WITH_OFFSET(h) +
                              pixel_squared[intensity]);
            }
        }
        /* initialize leftmost column of integral images */
        {
            INTEGRAL_IMAGE_SET_POS(0);
            for (y = 1; y < height; y++) {
                source_pos = source_rows[y];
                intensity = PIXEL_VALUE(source);
                INTEGRAL_IMAGE_ADVANCE_POS(v);
                I_1_SET_VALUE(I_1_GET_VALUE_WITH_OFFSET(v) +
                              intensity);
                I_2_SET_VALUE(I_2_GET_VALUE_WITH_OFFSET(v) +
                              pixel_squared[intensity]);
            }
        }
        /* initialize rest of integral images */
        /* add value of this pixel and integrals from top and left */
        /* subtract integral from top left diagonal */
        {
            INTEGRAL_IMAGE_SET_POS(width);
            for (y = 1; y < height; y++) {
                source_pos = source_rows[y];
                for (x = width - 1; x--; ) {
                    source_pos += source_step;
                    intensity = PIXEL_VALUE(source);
                    INTEGRAL_IMAGE_ADVANCE_POS(h);
                    I_1_SET_VALUE((I_1_GET_VALUE_WITH_OFFSET(v) -
                                   I_1_GET_VALUE_WITH_OFFSET(d)) +
                                   I_1_GET_VALUE_WITH_OFFSET(h) +
                                   intensity);
                    I_2_SET_VALUE((I_2_GET_VALUE_WITH_OFFSET(v) -
                                   I_2_GET_VALUE_WITH_OFFSET(d)) +
                                   I_2_GET_VALUE_WITH_OFFSET(h) +
                                   pixel_squared[intensity]);
                }
                /* skip one col to reach the beginning of next row */
                INTEGRAL_IMAGE_ADVANCE_POS(h);
            }
        }
    }

    FINALLY(integral_image_update);
    RETURN();
}

/******************************************************************************/

result normalize(pixel_image *source, pixel_image *target)
{
    TRY();
    CHECK_POINTER(source);
    CHECK_POINTER(target);
    CHECK_PARAM(source->width == target->width);
    CHECK_PARAM(source->height == target->height);
    CHECK_PARAM(target->type == p_U8);

    switch (source->type) {
    case p_U8:
        CHECK(normalize_byte(source, target, 0, 0, 0));
        break;
    case p_S8:
        CHECK(normalize_char(source, target, 0, 0, 0));
        break;
    case p_U16:
        CHECK(normalize_word(source, target, 0, 0, 0));
        break;
    case p_S32:
        CHECK(normalize_long(source, target, 0, 0, 0));
        break;
    case p_F64:
        CHECK(normalize_double(source, target, 0, 0, 0));
        break;
    default:
        ERROR(BAD_TYPE);
    }

    FINALLY(normalize);
    RETURN();
}

/******************************************************************************/
/* this set of private macros for generating the normalize functions is a     */
/* compromise between overall amount of copy-pasting and amount of generic    */
/* image processing code that is copy-pasted                                  */

#define NORMALIZE_FUNCTION_BEGIN(type)\
result normalize_##type(\
    pixel_image *source,\
    pixel_image *target,\
    type min,\
    type max,\
    type mean\
    )\
{\
    type value;\
    double factor;\
    int temp;

#define NORMALIZE_FUNCTION_END(type) }\

#if IMAGE_ACCESS_METHOD == IMAGE_ACCESS_BY_INDEX
#define CHECK_MINMAX()\
    {\
        value = source_data[source_pos];\
        if (value < min) {\
            min = value;\
        }\
        else if (value > max) {\
            max = value;\
        }\
    }
#elif IMAGE_ACCESS_METHOD == IMAGE_ACCESS_BY_POINTER
#define CHECK_MINMAX()\
    {\
        value = *source_pos;\
        if (value < min) {\
            min = value;\
        }\
        else if (value > max) {\
            max = value;\
        }\
    }
#else
#error "Image access method not defined"
#endif

#if IMAGE_ACCESS_METHOD == IMAGE_ACCESS_BY_INDEX
#define NORMALIZE_VALUE()\
    {\
        temp = (int)(factor * (double)(source_data[source_pos] - min));\
        target_data[target_pos] = (byte)((temp < 0) ? 0 : ((temp > 255) ? 255 : temp));\
    }
#elif IMAGE_ACCESS_METHOD == IMAGE_ACCESS_BY_POINTER
#define NORMALIZE_VALUE()\
    {\
        temp = (int)(factor * (double)(*source_pos - min));\
        *target_pos = (byte)((temp < 0) ? 0 : ((temp > 255) ? 255 : temp));\
    }
#else
#error "Image access method not defined"
#endif

/******************************************************************************/
/* generate normalize functions                                               */

NORMALIZE_FUNCTION_BEGIN(byte)
    TRY();

    CHECK_POINTER(source);
    CHECK_POINTER(target);
    CHECK_PARAM(source->type == p_U8);
    CHECK_PARAM(target->type == p_U8);
    CHECK_PARAM((min <= mean) && (mean <= max));

    if (pixel_image_is_continuous(source) && pixel_image_is_continuous(target)) {
        CONTINUOUS_IMAGE_VARIABLES(byte, byte);
        if (min == 0 && max == 0) {
            min = PIXEL_VALUE(source);
            max = min;

            FOR_CONTINUOUS_IMAGE(source)
                CHECK_MINMAX()
        }
        factor = 256.0 / (double)(max - min);

        FOR_2_CONTINUOUS_IMAGES()
            NORMALIZE_VALUE()
    }
    else {
        uint32 x, y;
        DISCONTINUOUS_IMAGE_VARIABLES(byte, byte);
        if (min == 0 && max == 0) {
            min = PIXEL_VALUE(source);
            max = min;

            FOR_DISCONTINUOUS_IMAGE(source)
                CHECK_MINMAX()
        }
        factor = 256.0 / (double)(max - min);

        FOR_2_DISCONTINUOUS_IMAGES()
            NORMALIZE_VALUE()
    }
    FINALLY(normalize_byte);
    RETURN();
NORMALIZE_FUNCTION_END()

/******************************************************************************/

NORMALIZE_FUNCTION_BEGIN(char)
    TRY();

    CHECK_POINTER(source);
    CHECK_POINTER(target);
    CHECK_PARAM(source->type == p_S8);
    CHECK_PARAM(target->type == p_U8);
    CHECK_PARAM((min <= mean) && (mean <= max));

    if (pixel_image_is_continuous(source) && pixel_image_is_continuous(target)) {
        CONTINUOUS_IMAGE_VARIABLES(char, byte);
        if (min == 0 && max == 0) {
            min = PIXEL_VALUE(source);
            max = min;

            FOR_CONTINUOUS_IMAGE(source)
                CHECK_MINMAX()
        }
        factor = 256.0 / (double)(max - min);

        FOR_2_CONTINUOUS_IMAGES()
            NORMALIZE_VALUE()
    }
    else {
        uint32 x, y;
        DISCONTINUOUS_IMAGE_VARIABLES(char, byte);
        if (min == 0 && max == 0) {
            min = PIXEL_VALUE(source);
            max = min;

            FOR_DISCONTINUOUS_IMAGE(source)
                CHECK_MINMAX()
        }
        factor = 256.0 / (double)(max - min);

        FOR_2_DISCONTINUOUS_IMAGES()
            NORMALIZE_VALUE()
    }

    FINALLY(normalize_char);
    RETURN();
NORMALIZE_FUNCTION_END()

/******************************************************************************/

NORMALIZE_FUNCTION_BEGIN(word)
    TRY();

    CHECK_POINTER(source);
    CHECK_POINTER(target);
    CHECK_PARAM(source->type == p_U16);
    CHECK_PARAM(target->type == p_U8);
    CHECK_PARAM((min <= mean) && (mean <= max));

    if (pixel_image_is_continuous(source) && pixel_image_is_continuous(target)) {
        CONTINUOUS_IMAGE_VARIABLES(word, byte);
        if (min == 0 && max == 0) {
            min = PIXEL_VALUE(source);
            max = min;

            FOR_CONTINUOUS_IMAGE(source)
                CHECK_MINMAX()
        }
        factor = 256.0 / (double)(max - min);

        FOR_2_CONTINUOUS_IMAGES()
            NORMALIZE_VALUE()
    }
    else {
        uint32 x, y;
        DISCONTINUOUS_IMAGE_VARIABLES(word, byte);
        if (min == 0 && max == 0) {
            min = PIXEL_VALUE(source);
            max = min;

            FOR_DISCONTINUOUS_IMAGE(source)
                CHECK_MINMAX()
        }
        factor = 256.0 / (double)(max - min);

        FOR_2_DISCONTINUOUS_IMAGES()
            NORMALIZE_VALUE()
    }

    FINALLY(normalize_word);
    RETURN();
NORMALIZE_FUNCTION_END()

/******************************************************************************/

NORMALIZE_FUNCTION_BEGIN(long)
    TRY();

    CHECK_POINTER(source);
    CHECK_POINTER(target);
    CHECK_PARAM(source->type == p_S32);
    CHECK_PARAM(target->type == p_U8);
    CHECK_PARAM((min <= mean) && (mean <= max));

    if (pixel_image_is_continuous(source) && pixel_image_is_continuous(target)) {
        CONTINUOUS_IMAGE_VARIABLES(long, byte);
        if (min == 0 && max == 0) {
            min = PIXEL_VALUE(source);
            max = min;

            FOR_CONTINUOUS_IMAGE(source)
                CHECK_MINMAX()
        }
        factor = 256.0 / (double)(max - min);

        FOR_2_CONTINUOUS_IMAGES()
            NORMALIZE_VALUE()
    }
    else {
        uint32 x, y;
        DISCONTINUOUS_IMAGE_VARIABLES(long, byte);
        if (min == 0 && max == 0) {
            min = PIXEL_VALUE(source);
            max = min;

            FOR_DISCONTINUOUS_IMAGE(source)
                CHECK_MINMAX()
        }
        factor = 256.0 / (double)(max - min);

        FOR_2_DISCONTINUOUS_IMAGES()
            NORMALIZE_VALUE()
    }

    FINALLY(normalize_long);
    RETURN();
NORMALIZE_FUNCTION_END()

/******************************************************************************/

NORMALIZE_FUNCTION_BEGIN(double)
    TRY();

    CHECK_POINTER(source);
    CHECK_POINTER(target);
    CHECK_PARAM(source->type == p_F64);
    CHECK_PARAM(target->type == p_U8);
    CHECK_PARAM((min <= mean) && (mean <= max));

    if (pixel_image_is_continuous(source) && pixel_image_is_continuous(target)) {
        CONTINUOUS_IMAGE_VARIABLES(double, byte);
        if (min == 0 && max == 0) {
            min = PIXEL_VALUE(source);
            max = min;

            FOR_CONTINUOUS_IMAGE(source)
                CHECK_MINMAX()
        }
        factor = 256.0 / (double)(max - min);

        FOR_2_CONTINUOUS_IMAGES()
            NORMALIZE_VALUE()
    }
    else {
        uint32 x, y;
        DISCONTINUOUS_IMAGE_VARIABLES(double, byte);
        if (min == 0 && max == 0) {
            min = PIXEL_VALUE(source);
            max = min;

            FOR_DISCONTINUOUS_IMAGE(source)
                CHECK_MINMAX()
        }
        factor = 256.0 / (double)(max - min);

        FOR_2_DISCONTINUOUS_IMAGES()
            NORMALIZE_VALUE()
    }

    FINALLY(normalize_double);
    RETURN();
NORMALIZE_FUNCTION_END()

/******************************************************************************/

result convert_grey8_to_grey24(
    const pixel_image *source,
    pixel_image *target
    )
{
    TRY();

    CHECK_POINTER(source);
    CHECK_POINTER(target);
    CHECK_POINTER(source->data);
    CHECK_POINTER(target->data);
    CHECK_PARAM(source->type == p_U8);
    CHECK_PARAM(target->type == p_U8);
    CHECK_PARAM(source->step == 1);
    CHECK_PARAM(target->step == 3);
    CHECK_PARAM(source->format == GREY);
    CHECK_PARAM(target->format == RGB);
    CHECK_PARAM(source->width == target->width);
    CHECK_PARAM(source->height == target->height);

    if (pixel_image_is_continuous(source) && pixel_image_is_continuous(target)) {
        CONTINUOUS_IMAGE_VARIABLES(byte, byte);
        FOR_2_CONTINUOUS_IMAGES()
        {
            PIXEL_VALUE(target)
                    = PIXEL_VALUE_PLUS(target, 1)
                    = PIXEL_VALUE_PLUS(target, 2)
                    = PIXEL_VALUE(source);

        }
    }
    else {
        uint32 x, y;
        DISCONTINUOUS_IMAGE_VARIABLES(byte, byte);
        FOR_2_DISCONTINUOUS_IMAGES()
        {
            PIXEL_VALUE(target)
                    = PIXEL_VALUE_PLUS(target, 1)
                    = PIXEL_VALUE_PLUS(target, 2)
                    = PIXEL_VALUE(source);

        }
    }

    FINALLY(convert_grey8_to_grey24);
    RETURN();
}

/******************************************************************************/

result convert_rgb24_to_grey8(
    const pixel_image *source,
    pixel_image *target
    )
{
    TRY();
    sint32 value;

    CHECK_POINTER(source);
    CHECK_POINTER(target);
    CHECK_POINTER(source->data);
    CHECK_POINTER(target->data);
    CHECK_PARAM(source->type == p_U8);
    CHECK_PARAM(target->type == p_U8);
    CHECK_PARAM(source->step == 3);
    CHECK_PARAM(target->step == 1);
    CHECK_PARAM(source->format == RGB);
    CHECK_PARAM(target->format == GREY);
    CHECK_PARAM(source->width == target->width);
    CHECK_PARAM(source->height == target->height);

    if (pixel_image_is_continuous(source) && pixel_image_is_continuous(target)) {
        CONTINUOUS_IMAGE_VARIABLES(byte, byte);
        FOR_2_CONTINUOUS_IMAGES()
        {
            value = (int)(0.30 * PIXEL_VALUE(source) +
                          0.59 * PIXEL_VALUE_PLUS(source, 1) +
                          0.11 * PIXEL_VALUE_PLUS(source, 2));
            value = TRUNC(value, 0, 255);
            PIXEL_VALUE(target) = (byte)value;
        }
    }
    else {
        uint32 x, y;
        DISCONTINUOUS_IMAGE_VARIABLES(byte, byte);
        FOR_2_DISCONTINUOUS_IMAGES()
        {
            value = (int)(0.30 * PIXEL_VALUE(source) +
                          0.59 * PIXEL_VALUE_PLUS(source, 1) +
                          0.11 * PIXEL_VALUE_PLUS(source, 2));
            value = TRUNC(value, 0, 255);
            PIXEL_VALUE(target) = (byte)value;
        }
    }

    FINALLY(convert_rgb24_to_grey8);
    RETURN();
}

/******************************************************************************/

result scale_down(
    const pixel_image *source,
    pixel_image *target
    )
{
    TRY();

    CHECK_POINTER(source);
    CHECK_POINTER(target);
    CHECK_POINTER(source->data);
    CHECK_POINTER(target->data);
    CHECK_PARAM(source->type == p_U8);
    CHECK_PARAM(target->type == p_U8);
    CHECK_PARAM(2 * target->width >= source->width);
    CHECK_PARAM(2 * target->height >= source->height);

    {
        uint32 x, y;
        IMAGE_WITH_STEP_VARIABLES(byte, byte);
        FOR_2_IMAGES_WITH_STEP(2, 2, 1, 1)
        {
            PIXEL_VALUE(target) = PIXEL_VALUE(source);
        }
    }

    FINALLY(scale_down);
    RETURN();
}

/******************************************************************************/

result scale_up(
    const pixel_image *source,
    pixel_image *target
    )
{
    TRY();

    CHECK_POINTER(source);
    CHECK_POINTER(target);
    CHECK_POINTER(source->data);
    CHECK_POINTER(target->data);
    CHECK_PARAM(source->type == p_U8);
    CHECK_PARAM(target->type == p_U8);
    CHECK_PARAM(target->width >= 2 * source->width);
    CHECK_PARAM(target->height >= 2 * source->height);

    {
        uint32 x, y, offset_1, offset_2, offset_3;
        IMAGE_WITH_STEP_VARIABLES(byte, byte);
        offset_1 = target->step;
        offset_2 = target->stride;
        offset_3 = target->stride + target->step;
        FOR_2_IMAGES_WITH_STEP_REVERSE(1, 1, 2, 2)
        {
            PIXEL_VALUE(target) =
            PIXEL_VALUE_PLUS(target, offset_1) =
            PIXEL_VALUE_PLUS(target, offset_2) =
            PIXEL_VALUE_PLUS(target, offset_3) =
            PIXEL_VALUE(source);
        }
    }

    FINALLY(scale_up);
    RETURN();
}

/******************************************************************************/
