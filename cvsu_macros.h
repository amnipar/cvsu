/**
 * @file cvsu_macros.h
 * @author Matti J. Eskelinen <matti.j.eskelinen@gmail.com>
 * @brief Macros for the cvsu module.
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

#ifndef CVSU_MACROS_H
#   define CVSU_MACROS_H

#include "cvsu_config.h"
#include "cvsu_types.h"
#include "cvsu_output.h"

#define TRY(func) result r

#define ERROR(e) do {\
    r = e;\
    goto finally; } while (0)

#define CHECK(res) do {\
    r = SUCCESS;\
    if ((res) != SUCCESS){\
        r = CAUGHT_ERROR;\
        goto finally;\
    } } while (0)

#define BREAK(expr) do {\
    r = (expr); break; } while (0)

#define TERMINATE(expr) do {\
    r = (expr); goto finally; } while (0)

#if (OUTPUT_LEVEL > OUTPUT_LEVEL_NONE)
#define FINALLY(func)\
finally:\
    report_result(r, func##_name)
#else
#define FINALLY(func)\
finally:
#endif

#define RETURN() return r

/**
 * Macro for handling NULL pointer situations.
 * Creates a result with error code and function name.
 * Requires 'result r' variable, 'finally:' label, and function name constant.
 */
#define CHECK_POINTER(ptr) do {\
    r = SUCCESS;\
    if (ptr == NULL) {\
        r = BAD_POINTER;\
        goto finally;\
    } } while (0)

#define CHECK_PARAM(expr) do {\
    r = SUCCESS;\
    if (!(expr)) {\
        r = BAD_PARAM;\
        goto finally;\
    } } while (0)

#define CHECK_TRUE(expr) do {\
  r == SUCCESS;\
  if ((expr) == 0) {\
    r = BAD_PARAM;\
    goto finally;\
  } } while (0)

#define CHECK_FALSE(expr) do {\
  r == SUCCESS;\
  if ((expr) != 0) {\
    r = BAD_PARAM;\
    goto finally;\
  } } while (0)

#define TRUNC(value, min, max)\
    (((value) < (min)) ? (min) : (((value) > (max)) ? (max) : (value)))

#define IS_TRUE(expr) ((expr) != 0)

#define IS_FALSE(expr) ((expr) == 0)

/******************************************************************************/
/* macros for image access by pointers                                        */

#define IMAGE_RECT_VARIABLES(image, type, rect)\
  const type *image##_data, *image##_pos;\
  uint32 image##_step, image##_stride;\
  image##_data = (type *)image->data;\
  image##_step = image->step;\
  image##_stride = image->stride - (rect.hstep * image->step) + 1;\
  image##_pos = image##_data + rect.offset

#define SINGLE_CONTINUOUS_IMAGE_VARIABLES(image, type)\
    type *image##_data, *image##_pos, *image##_end;\
    uint32 image##_step;\
    image##_data = (type *)image->data;\
    image##_end = image##_data + image->size;\
    image##_step = image->step;\
    image##_pos = image##_data + image->offset

#define CONTINUOUS_IMAGE_VARIABLES(source_type, target_type)\
    source_type *source_data, *source_pos, *source_end;\
    target_type *target_data, *target_pos;\
    uint32 source_step, target_step;\
    source_data = (source_type *)source->data;\
    source_end = source_data + source->size;\
    source_step = source->step;\
    target_data = (target_type *)target->data;\
    target_step = target->step;\
    source_pos = source_data + source->offset;\
    target_pos = target_data + target->offset

#define SINGLE_DISCONTINUOUS_IMAGE_VARIABLES(image, type)\
    type *image##_data, **image##_rows, *image##_pos;\
    uint32 image##_step;\
    image##_data = (type *)image->data;\
    image##_rows = (type **)image->rows;\
    image##_step = image->step;\
    image##_pos = image##_data + image->offset

#define DISCONTINUOUS_IMAGE_VARIABLES(source_type, target_type)\
    source_type *source_data, **source_rows, *source_pos;\
    target_type *target_data, **target_rows, *target_pos;\
    uint32 source_step, target_step;\
    source_data = (source_type *)source->data;\
    source_rows = (source_type **)source->rows;\
    source_step = source->step;\
    target_data = (target_type *)target->data;\
    target_rows = (target_type **)target->rows;\
    target_step = target->step;\
    source_pos = source_data + source->offset;\
    target_pos = target_data + target->offset

/* when applying a step, must process image row by row */
/* do not initialize step */
#define SINGLE_IMAGE_WITH_STEP_VARIABLES(image, type)\
    type *image##_data, **image##rows, *image##_pos;\
    uint32 image##_step, image##_row;\
    image##_data = (type *)image->data;\
    image##_rows = (type **)image->rows;\
    image##_pos = image##_data + image->offset

/* when applying a step, must process image row by row */
/* do not initialize step */
#define IMAGE_WITH_STEP_VARIABLES(source_type, target_type)\
    source_type *source_data, **source_rows, *source_pos;\
    target_type *target_data, **target_rows, *target_pos;\
    uint32 source_step, source_row, target_step, target_row;\
    source_data = (source_type *)source->data;\
    source_rows = (source_type **)source->rows;\
    target_data = (target_type *)target->data;\
    target_rows = (target_type **)target->rows;\
    source_pos = source_data + source->offset;\
    target_pos = target_data + target->offset

#define FOR_IMAGE_RECT_BEGIN(image, rect)\
  for (y = rect.vstep, image##_pos = image##_data + rect.offset; y--;) {\
    for (x = rect.hstep; x--; image##_pos += image##_step) {

#define FOR_IMAGE_RECT_END(image)\
    }\
    image##_pos += image##_stride;\
  }

#define FOR_CONTINUOUS_IMAGE(image)\
    for (image##_pos = image##_data + image->offset;\
         image##_pos < image##_end; image##_pos += image##_step)

#define FOR_CONTINUOUS_IMAGE_WITH_OFFSET(image, offset)\
    for (image##_pos = image##_data + offset;\
         image##_pos < image##_end; image##_pos += image##_step)

#define FOR_2_CONTINUOUS_IMAGES()\
    for (source_pos = source_data + source->offset,\
         target_pos = target_data + target->offset;\
         source_pos < source_end;\
         source_pos += source_step, target_pos += target_step)

#define FOR_2_CONTINUOUS_IMAGES_WITH_OFFSET(source_offset, target_offset)\
    for (source_pos = source_data + source_offset,\
         target_pos = target_data + target_offset;\
         source_pos < source_end;\
         source_pos += source_step, target_pos += target_step)

#define FOR_DISCONTINUOUS_IMAGE(image)\
    for (y = image->height; y--; )\
        for (image##_pos = image##_rows[y],\
             x = image->width; x--; image##_pos += image##_step)

#define FOR_DISCONTINUOUS_IMAGE_WITH_OFFSET(image, offset)\
    for (y = image->height; y--; )\
        for (image##_pos = image##_rows[y] + offset,\
             x = image->width; x--; image##_pos += image##_step)

#define FOR_DISCONTINUOUS_IMAGE_ROW(image)\
    for (image##_pos = image##_rows[0],x,\
         y = 0; y < image->height; y++,\
         image##_pos = image##_rows[y])

#define FOR_DISCONTINUOUS_IMAGE_COL(image)\
    for (image##_pos = image##_rows[0],y,\
         x = 0; x < image->width; x++,\
         image##_pos = image##_rows[0] + x * image##_step)

#define FOR_2_DISCONTINUOUS_IMAGES()\
    for (y = source->height; y--; )\
        for (source_pos = source_rows[y],\
             target_pos = target_rows[y],\
             x = source->width; x--; source_pos += source_step, target_pos += target_step)

#define FOR_2_DISCONTINUOUS_IMAGES_WITH_OFFSET(source_offset, target_offset)\
    for (y = source->height; y--; )\
        for (source_pos = source_rows[y] + source_offset,\
             target_pos = target_rows[y] + target_offset,\
             x = source->width; x--; source_pos += source_step, target_pos += target_step)

#define FOR_2_DISCONTINUOUS_IMAGE_ROWS()\
    for (source_pos = source_rows[0], target_pos = target_rows[0],x,\
         y = 0; y < source->height; y++,\
         source_pos = source_rows[y], target_pos = target_rows[y])

#define FOR_2_DISCONTINUOUS_IMAGE_COLS()\
    for (source_pos = source_rows[0], target_pos = target_rows[0],y,\
         x = 0; x < source->width; x++,\
         source_pos = source_rows[0] + x * source_step,\
         target_pos = target_rows[0] + x * target_step)

#define FOR_IMAGE_WITH_STEP(image, row_step, col_step)\
    for (image##_step = image->step * col_step,\
         image##_row = 0,\
         y = 0; y < image->height; y += row_step, image##_row += row_step)\
        for (image##_pos = image##_rows[image##_row],\
             x = 0; x < image->width; x += col_step, image##_pos += image##_step)

#define FOR_2_IMAGES_WITH_STEP(source_row_step, source_col_step, target_row_step, target_col_step)\
    for (source_step = source->step * source_col_step,\
         source_row = 0,\
         target_step = target->step * target_col_step,\
         target_row = 0,\
         y = 0; y < source->height; y += source_row_step,\
         source_row += source_row_step, target_row += target_row_step)\
        for (source_pos = source_rows[source_row],\
             target_pos = target_rows[target_row],\
             x = 0; x < source->width; x += source_col_step,\
             source_pos += source_step, target_pos += target_step)

#define FOR_2_IMAGES_WITH_STEP_REVERSE(source_row_step, source_col_step, target_row_step, target_col_step)\
    for (source_step = source->step * source_col_step,\
         source_row = source->height - source_row_step,\
         target_step = target->step * target_col_step,\
         target_row = target->height - target_row_step,\
         y = 0; y < source->height; y += source_row_step,\
         source_row -= source_row_step, target_row -= target_row_step)\
        for (source_pos = source_rows[source_row] + source->width - source_step,\
             target_pos = target_rows[target_row] + target->width - target_step,\
             x = 0; x < source->width; x += source_col_step,\
             source_pos -= source_step, target_pos -= target_step)

#define PIXEL_VALUE(image) *image##_pos
#define PIXEL_VALUE_PLUS(image, offset) *(image##_pos + (offset))
#define PIXEL_VALUE_MINUS(image, offset) *(image##_pos - (offset))

#define POINTER_TO_PIXEL(image) image##_pos

/******************************************************************************/
/* macros for integral image handling                                         */

#define INTEGRAL_IMAGE_1BOX_VARIABLES()\
    const I_1_t *I_1_data, *iA;\
    const I_2_t *I_2_data, *i2A;\
    uint32 B_inc, C_inc, D_inc;\
    integral_value N, sum, sumsqr;\

#define INTEGRAL_IMAGE_2BOX_VARIABLES()\
    const I_1_t *I_1_data, *iA1;\
    const I_2_t *I_2_data, *i2A1;\
    uint32 step, stride, B1_inc, C1_inc, D1_inc, A2_inc, B2_inc, C2_inc, D2_inc;\
    integral_value N, sum1, sum2, sumsqr1, sumsqr2, g;

#define INTEGRAL_IMAGE_INIT_1BOX(I, box_length, box_width)\
    I_1_data = (I_1_t *)(I)->I_1.data;\
    I_2_data = (I_2_t *)(I)->I_2.data;\
    B_inc = (box_length) * (I)->step;\
    C_inc = (box_width) * (I)->stride + (box_length) * (I)->step;\
    D_inc = (box_width) * (I)->stride;\
    N = (integral_value)((box_length) * (box_width))

#define INTEGRAL_IMAGE_INIT_HBOX(I, box_length, box_width)\
    I_1_data = (I_1_t *)(I)->I_1.data;\
    I_2_data = (I_2_t *)(I)->I_2.data;\
    step = (I)->step;\
    stride = (I)->stride;\
    B1_inc = (box_length) * step;\
    C1_inc = (box_width) * stride + (box_length) * step;\
    D1_inc = (box_width) * stride;\
    A2_inc = ((box_length) + 1) * step;\
    B2_inc = (2 * (box_length) + 1) * step;\
    C2_inc = (box_width) * stride + (2 * (box_length) + 1) * step;\
    D2_inc = (box_width) * stride + ((box_length) + 1) * step;\
    N = (integral_value)((box_length) * (box_width))

#define INTEGRAL_IMAGE_INIT_VBOX(I, box_length, box_width)\
    I_1_data = (I_1_t *)(I)->I_1.data;\
    I_2_data = (I_2_t *)(I)->I_2.data;\
    step = (I)->step;\
    stride = (I)->stride;\
    B1_inc = (box_width) * step;\
    C1_inc = (box_length) * stride + (box_width) * step;\
    D1_inc = (box_length) * stride;\
    A2_inc = ((box_length) + 1) * stride;\
    B2_inc = ((box_length) + 1) * stride + (box_width) * step;\
    C2_inc = (2 * (box_length) + 1) * stride + (box_width) * step;\
    D2_inc = (2 * (box_length) + 1) * stride;\
    N = (integral_value)((box_width) * (box_length))

#define INTEGRAL_IMAGE_SUM() (*(iA + C_inc) - *(iA + B_inc) - *(iA + D_inc) + *iA)
#define INTEGRAL_IMAGE_SUM_1() (*(iA1 + C1_inc) - *(iA1 + B1_inc) - *(iA1 + D1_inc) + *iA1)
#define INTEGRAL_IMAGE_SUM_2() (*(iA1 + C2_inc) - *(iA1 + B2_inc) - *(iA1 + D2_inc) + *(iA1 + A2_inc))
#define INTEGRAL_IMAGE_SUMSQR() (*(i2A + C_inc) - *(i2A + B_inc) - *(i2A + D_inc) + *i2A)
#define INTEGRAL_IMAGE_SUMSQR_1() (*(i2A1 + C1_inc) - *(i2A1 + B1_inc) - *(i2A1 + D1_inc) + *i2A1)
#define INTEGRAL_IMAGE_SUMSQR_2() (*(i2A1 + C2_inc) - *(i2A1 + B2_inc) - *(i2A1 + D2_inc) + *(i2A1 + A2_inc))

#endif /* CVSU_MACROS_H */
