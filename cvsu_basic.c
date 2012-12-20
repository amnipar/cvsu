/**
 * @file cvsu_basic.c
 * @author Matti J. Eskelinen <matti.j.eskelinen@gmail.com>
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

#include <stdlib.h>
#include <math.h>
#include <limits.h>

/******************************************************************************/
/* constants for storing the function names                                   */
/* used in error reporting macros                                             */

string pixel_image_init_name = "pixel_image_init";
string pixel_image_alloc_name = "pixel_image_alloc";
string pixel_image_free_name = "pixel_image_free";
string pixel_image_create_name = "pixel_image_create";
string pixel_image_create_from_data_name = "pixel_image_create_from_data";
string pixel_image_destroy_name = "pixel_image_destroy";
string pixel_image_nullify_name = "pixel_image_nullify";
string pixel_image_convert_name = "pixel_image_convert";
string pixel_image_create_roi_name = "pixel_image_create_roi";
string pixel_image_clone_name = "pixel_image_clone";
string pixel_image_copy_name = "pixel_image_copy";
string pixel_image_clear_name = "pixel_image_clear";
string pixel_image_read_name = "pixel_image_read";
string pixel_image_write_name = "pixel_image_write";
string normalize_name = "normalize";
string normalize_byte_name = "normalize_byte";
string normalize_char_name = "normalize_char";
string normalize_word_name = "normalize_word";
string normalize_long_name = "normalize_long";
string normalize_float_name = "normalize_float";
string normalize_double_name = "normalize_double";
string scale_down_name = "scale_down";
string scale_up_name = "scale_up";
string convert_grey8_to_grey24_name = "convert_grey8_to_grey24";
string convert_grey8_to_yuv24_name = "convert_grey8_to_yuv24";
string convert_rgb24_to_grey8_name = "convert_rgb24_to_grey8";
string convert_rgb24_to_yuv24_name = "convert_rgb24_to_yuv24";
string convert_yuv24_to_rgb24_name = "convert_yuv24_to_rgb24";
string convert_yuv24_to_grey8_name = "convert_yuv24_to_grey8";
string pick_1_channel_from_3_channels_name = "pick_1_channel_from_3_channels";

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
    case p_S16:
        pixel_size = sizeof(sint16);
        break;
    case p_U32:
        pixel_size = sizeof(uint32);
        break;
    case p_S32:
        pixel_size = sizeof(sint32);
        break;
    case p_F32:
        pixel_size = sizeof(real32);
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

pixel_image *pixel_image_alloc()
{
    TRY();
    pixel_image *ptr;

    CHECK(memory_allocate((data_pointer *)&ptr, 1, sizeof(pixel_image)));
    CHECK(pixel_image_nullify(ptr));

    FINALLY(pixel_image_alloc);
    return ptr;
}

/******************************************************************************/

void pixel_image_free(
    pixel_image *ptr
    )
{
    TRY();

    r = SUCCESS;

    if (ptr != NULL) {
        CHECK(pixel_image_destroy(ptr));
        CHECK(memory_deallocate((data_pointer *)&ptr));
    }
    FINALLY(pixel_image_free);
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
    case p_S16:
        CHECK(memory_allocate(&data, size, sizeof(sint16)));
        break;
    case p_U32:
        CHECK(memory_allocate(&data, size, sizeof(uint32)));
        break;
    case p_S32:
        CHECK(memory_allocate(&data, size, sizeof(sint32)));
        break;
    case p_F32:
        CHECK(memory_allocate(&data, size, sizeof(real32)));
        break;
    case p_F64:
        CHECK(memory_allocate(&data, size, sizeof(real64)));
        break;
    default:
        ERROR(BAD_TYPE);
    }

    CHECK(pixel_image_init(target, data, type, format, 0, 0, width, height, 0, step, stride, size));
    target->own_data = 1;

    FINALLY(pixel_image_create);
    RETURN();
}

/******************************************************************************/

result pixel_image_create_from_data(
    pixel_image *target,
    data_pointer data,
    pixel_type type,
    pixel_format format,
    uint32 width,
    uint32 height,
    uint32 step,
    uint32 stride
    )
{
    TRY();
    uint32 size;

    CHECK_POINTER(target);
    CHECK_POINTER(data);

    size = height * stride;

    CHECK(pixel_image_init(target, data, type, format, 0, 0, width, height, 0, step, stride, size));
    target->own_data = 0;

    FINALLY(pixel_image_create_from_data);
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
    if (target->parent == NULL && target->own_data != 0) {
        CHECK(memory_deallocate((data_pointer *)&target->data));
    }
    CHECK(memory_deallocate((data_pointer *)&target->rows));
    CHECK(pixel_image_nullify(target));

    FINALLY(pixel_image_destroy);
    RETURN();
}

/******************************************************************************/

result pixel_image_nullify(
    pixel_image *target
    )
{
    TRY();

    CHECK_POINTER(target);

    target->parent = NULL;
    target->data = NULL;
    target->rows = NULL;
    target->own_data = 0;
    target->type = p_NONE;
    target->format = NONE;
    target->dx = 0;
    target->dy = 0;
    target->width = 0;
    target->height = 0;
    target->offset = 0;
    target->step = 0;
    target->stride = 0;
    target->size = 0;

    FINALLY(pixel_image_nullify);
    RETURN();
}

/******************************************************************************/

result pixel_image_convert(
    pixel_image *source,
    pixel_image *target
    )
{
    pixel_image *temp;
    TRY();

    temp = NULL;

    CHECK_POINTER(source);
    CHECK_POINTER(target);

    CHECK_PARAM(source->width == target->width);
    CHECK_PARAM(source->height == target->height);
    /*printf("t:%d->%d,f:%d->%d\n",source->type, target->type, source->format, target->format);*/

    if (source->type == target->type && source->format == target->format) {
        CHECK(pixel_image_copy(target, source));
    }
    else {
        /* first convert the pixel type of the source image */
        if (source->type != target->type) {
            ERROR(NOT_IMPLEMENTED);
        }
        /* then convert the format */
        switch (source->format) {
        case GREY:
            if (target->format == RGB) {
                CHECK(convert_grey8_to_grey24(source, target));
            }
            else
            if (target->format == YUV) {
                CHECK(convert_grey8_to_yuv24(source, target));
            }
            else {
                ERROR(NOT_IMPLEMENTED);
            }
            break;
        case RGB:
          if (target->format == GREY) {
              CHECK(convert_rgb24_to_grey8(source, target));
          }
          else
          if (target->format == YUV) {
              CHECK(convert_rgb24_to_yuv24(source, target));
          }
          else {
              ERROR(NOT_IMPLEMENTED);
          }
          break;
        case YUV:
          if (target->format == GREY) {
              CHECK(convert_yuv24_to_grey8(source, target));
          }
          else
          if (target->format == RGB) {
              CHECK(convert_yuv24_to_rgb24(source, target));
          }
          else {
              ERROR(NOT_IMPLEMENTED);
          }
          break;
        default:
            ERROR(NOT_IMPLEMENTED);
        }
    }

    FINALLY(pixel_image_convert);
    if (temp != NULL) {
        pixel_image_free(temp);
    }
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
    const pixel_image *source
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
        case p_S16:
            pixel_size = sizeof(sint16);
            break;
        case p_U32:
            pixel_size = sizeof(uint32);
            break;
        case p_S32:
            pixel_size = sizeof(sint32);
            break;
        case p_F32:
            pixel_size = sizeof(real32);
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
                DISCONTINUOUS_IMAGE_VARIABLES(sint8, sint8);
                FOR_2_DISCONTINUOUS_IMAGE_ROWS()
                    memory_copy((data_pointer)POINTER_TO_PIXEL(target),
                                (data_pointer)POINTER_TO_PIXEL(source),
                                source->width * source_step, sizeof(sint8));
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
        case p_S16:
            {
                DISCONTINUOUS_IMAGE_VARIABLES(sint16, sint16);
                FOR_2_DISCONTINUOUS_IMAGE_ROWS()
                    memory_copy((data_pointer)POINTER_TO_PIXEL(target),
                                (data_pointer)POINTER_TO_PIXEL(source),
                                source->width * source_step, sizeof(sint16));
            }
            break;
        case p_U32:
            {
                DISCONTINUOUS_IMAGE_VARIABLES(dword, dword);
                FOR_2_DISCONTINUOUS_IMAGE_ROWS()
                    memory_copy((data_pointer)POINTER_TO_PIXEL(target),
                                (data_pointer)POINTER_TO_PIXEL(source),
                                source->width * source_step, sizeof(dword));
            }
            break;
        case p_S32:
            {
                DISCONTINUOUS_IMAGE_VARIABLES(sint32, sint32);
                FOR_2_DISCONTINUOUS_IMAGE_ROWS()
                    memory_copy((data_pointer)POINTER_TO_PIXEL(target),
                                (data_pointer)POINTER_TO_PIXEL(source),
                                source->width * source_step, sizeof(sint32));
            }
            break;
        case p_F32:
            {
                DISCONTINUOUS_IMAGE_VARIABLES(real32, real32);
                FOR_2_DISCONTINUOUS_IMAGE_ROWS()
                    memory_copy((data_pointer)POINTER_TO_PIXEL(target),
                                (data_pointer)POINTER_TO_PIXEL(source),
                                source->width * source_step, sizeof(real32));
            }
            break;
        case p_F64:
            {
                DISCONTINUOUS_IMAGE_VARIABLES(real64, real64);
                FOR_2_DISCONTINUOUS_IMAGE_ROWS()
                    memory_copy((data_pointer)POINTER_TO_PIXEL(target),
                                (data_pointer)POINTER_TO_PIXEL(source),
                                source->width * source_step, sizeof(real64));
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
        case p_S16:
            pixel_size = sizeof(sint16);
            break;
        case p_U32:
            pixel_size = sizeof(uint32);
            break;
        case p_S32:
            pixel_size = sizeof(sint32);
            break;
        case p_F32:
            pixel_size = sizeof(real32);
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
        case p_S16:
            {
                SINGLE_DISCONTINUOUS_IMAGE_VARIABLES(target, sint16);
                FOR_DISCONTINUOUS_IMAGE_ROW(target)
                    memory_clear((data_pointer)POINTER_TO_PIXEL(target),
                                 target->width * target_step, sizeof(sint16));
            }
            break;
        case p_U32:
            {
                SINGLE_DISCONTINUOUS_IMAGE_VARIABLES(target, dword);
                FOR_DISCONTINUOUS_IMAGE_ROW(target)
                    memory_clear((data_pointer)POINTER_TO_PIXEL(target),
                                 target->width * target_step, sizeof(dword));
            }
            break;
        case p_S32:
            {
                SINGLE_DISCONTINUOUS_IMAGE_VARIABLES(target, sint32);
                FOR_DISCONTINUOUS_IMAGE_ROW(target)
                    memory_clear((data_pointer)POINTER_TO_PIXEL(target),
                                 target->width * target_step, sizeof(sint32));
            }
            break;
        case p_F32:
            {
                SINGLE_DISCONTINUOUS_IMAGE_VARIABLES(target, real32);
                FOR_DISCONTINUOUS_IMAGE_ROW(target)
                    memory_clear((data_pointer)POINTER_TO_PIXEL(target),
                                 target->width * target_step, sizeof(real32));
            }
            break;
        case p_F64:
            {
                SINGLE_DISCONTINUOUS_IMAGE_VARIABLES(target, real64);
                FOR_DISCONTINUOUS_IMAGE_ROW(target)
                    memory_clear((data_pointer)POINTER_TO_PIXEL(target),
                                 target->width * target_step, sizeof(real64));
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

void skip_whitespace(FILE *file)
{
  char c;

  c = getc(file);
  while (c == ' ' || c == '\t' || c == '\v' || c == '\f' || c == '\n' || c == '\r') {
    c = getc(file);
  }
  ungetc(c, file);
}

void skip_comment(FILE *file)
{
  char c;
  c = getc(file);

  if (c == '#') {
    while (c != '\n' && c != '\r') {
      c = getc(file);
    }
  }
  ungetc(c, file);
}

uint32 read_format(FILE *file)
{
  int read_count;
  char c;

  read_count = fscanf(file, "P%c", &c);
  if (read_count < 1) {
    return 0;
  }
  switch (c) {
    case '1': return 1;
    case '2': return 2;
    case '3': return 3;
    case '4': return 4;
    case '5': return 5;
    case '6': return 6;
    default: return NONE;
  }
}

int read_number(FILE *file)
{
  int read_count, i;

  read_count = fscanf(file, "%d", &i);
  if (read_count != 1) {
    return -1;
  }
  return i;
}

result pixel_image_read
(
  pixel_image *target,
  string source
)
{
  TRY();
  FILE* file;
  int read_count, value;
  uint32 number, width, height, maxval;
  pixel_format format;
  pixel_type type;

  CHECK_POINTER(target);
  CHECK_POINTER(source);

  printf("Starting to read image '%s'...\n", source);
  file = fopen(source, "rb");
  if (file == NULL) {
    printf("Error: opening pnm file failed\n");
    ERROR(INPUT_ERROR);
  }

  number = read_format(file);
  if (number == 0) {
    printf("Error: reading format of pnm failed\n");
    ERROR(INPUT_ERROR);
  }

  if (number == 1 || number == 4) {
    format = MONO;
  }
  else
  if (number == 2 || number == 5) {
    format = GREY;
  }
  else {
    format = RGB;
  }

  skip_whitespace(file);
  skip_comment(file);
  skip_whitespace(file);

  value = read_number(file);
  if (value < 0) {
    printf("Error: reading width of pnm failed\n");
    ERROR(INPUT_ERROR);
  }
  width = (unsigned)value;

  skip_whitespace(file);
  skip_comment(file);
  skip_whitespace(file);

  value = read_number(file);
  if (value < 0) {
    printf("Error: reading height of pnm failed\n");
    ERROR(INPUT_ERROR);
  }
  height = (unsigned)value;

  skip_whitespace(file);
  skip_comment(file);
  skip_whitespace(file);

  if (format == MONO) {
    maxval = 1;
    type = p_U8;
  }
  else {
    value = read_number(file);
    if (value < 0) {
      printf("Error: reading maxval of pnm failed\n");
      ERROR(INPUT_ERROR);
    }
    maxval = (unsigned)value;

    if (maxval < 256) {
      type = p_U8;
    }
    else
    if (maxval < 65536) {
      type = p_U16;
    }
    else {
      type = p_U32;
    }

    skip_whitespace(file);
    skip_comment(file);
    skip_whitespace(file);
  }

  if (format == RGB) {
    CHECK(pixel_image_create(target, type, format, width, height, 3, 3 * width));
  }
  else {
    CHECK(pixel_image_create(target, type, format, width, height, 1, width));
  }

  /* ASCII types must be read value by value and stored in the array */
  if (number < 4) {
    if (type == p_U8) {
      byte *target_data;
      uint32 pos, size;

      target_data = (byte *)target->data;
      size = target->size;
      pos = 0;
      while (pos < size) {
        value = read_number(file);
        if (value < 0) {
          printf("Error: reading pnm image data failed at position %lu/%lu\n", pos, size-1);
          ERROR(INPUT_ERROR);
        }
        if (value > 255) {
          printf("Error: too large pnm image value at position %lu/%lu\n", pos, size-1);
          ERROR(INPUT_ERROR);
        }

        target_data[pos] = (byte)value;
        pos++;
        skip_whitespace(file);
      }
    }
    else
    if (type == p_U16) {
      uint16 *target_data;
      uint32 pos, size;

      target_data = (uint16 *)target->data;
      size = target->size;
      pos = 0;
      while (pos < size) {
        value = read_number(file);
        if (value < 0) {
          printf("Error: reading pnm image data failed at position %lu/%lu\n", pos, size-1);
          ERROR(INPUT_ERROR);
        }
        if (value > 65535) {
          printf("Error: too large pnm image value at position %lu/%lu\n", pos, size-1);
          ERROR(INPUT_ERROR);
        }

        target_data[pos] = (uint16)value;
        pos++;
        skip_whitespace(file);
      }
    }
    else {
      uint32 *target_data, pos, size;

      target_data = (uint32 *)target->data;
      size = target->size;
      pos = 0;
      while (pos < size) {
        value = read_number(file);
        if (value < 0) {
          printf("Error: reading pnm image data failed at position %lu/%lu\n", pos, size-1);
          ERROR(INPUT_ERROR);
        }

        target_data[pos] = (uint32)value;
        pos++;
        skip_whitespace(file);
      }
    }
  }
  /* binary types can be read in one chunk... */
  else {
    read_count = fread(target->data, sizeof(byte), target->size, file);
    if (read_count != target->size) {
      printf("Error: reading pnm image data failed\n");
      ERROR(INPUT_ERROR);
    }
  }

  printf("Successfully read type %lu image of size (%lu x %lu)\n", number, width, height);

  FINALLY(pixel_image_read);
  fclose(file);
  RETURN();
}

/******************************************************************************/

result pixel_image_write
(
  pixel_image *source,
  string target,
  uint32 ascii
)
{
  TRY();
  FILE* file;
  int write_count, number, maxval;

  CHECK_POINTER(target);
  CHECK_POINTER(source);
  CHECK_PARAM(source->type == p_U8 || source->type == p_U16);
  CHECK_PARAM(source->format == MONO || source->format == GREY || source->format == RGB);

  printf("Starting to write image '%s'...\n", target);
  file = fopen(target, "wb");
  if (file == NULL) {
    printf("Error: opening pnm file failed\n");
    ERROR(INPUT_ERROR);
  }

  if (ascii == 1) {
    if (source->format == MONO) {
      number = 1;
    }
    else
    if (source->format == GREY) {
      number = 2;
    }
    else {
      number = 3;
    }
  }
  else {
    if (source->format == MONO) {
      number = 4;
    }
    else
    if (source->format == GREY) {
      number = 5;
    }
    else {
      number = 6;
    }
  }

  if (source->type == p_U8) {
    maxval = 255;
  }
  else {
    maxval = 65535;
  }

  fprintf(file, "P%d\n# Created by cvsu\n", number);
  if (number == 1 || number == 4) {
    fprintf(file, "%lu %lu", source->width, source->height);
  }
  else {
    fprintf(file, "%lu %lu %d\n", source->width, source->height, maxval);
  }

  if (number < 4) {
    int x, y, width, height, stride, pos;
    width = source->width * source->step;
    height = source->height;
    stride = source->stride;
    if (maxval = 255) {
      byte *source_data;
      source_data = (byte *)source->data;
      for (y = 0; y < height; y++) {
        pos = y * stride;
        for (x = width; --x; pos++) {
          if (x > 1) {
            fprintf(file, "%d ", source_data[pos]);
          }
          else {
            fprintf(file, "%d\n", source_data[pos]);
          }
        }
      }
    }
    else {
      uint16 *source_data;
      source_data = (uint16 *)source->data;
      for (y = 0; y < height; y++) {
        pos = y * stride;
        for (x = width; --x; pos++) {
          if (x > 1) {
            fprintf(file, "%d ", source_data[pos]);
          }
          else {
            fprintf(file, "%d\n", source_data[pos]);
          }
        }
      }
    }
  }
  else {
    write_count = fwrite(source->data, sizeof(byte), source->size, file);
    if (write_count != source->size) {
      printf("Error writing pnm file data\n");
      ERROR(INPUT_ERROR);
    }
  }

  FINALLY(pixel_image_write);
  fclose(file);
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
    case p_F32:
        CHECK(normalize_float(source, target, 0, 0, 0));
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
    uint32 i;\
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
        for (i = 0; i < source->step; i++) {
            if (min == 0 && max == 0) {
                min = PIXEL_VALUE(source);
                max = min;

                FOR_CONTINUOUS_IMAGE_WITH_OFFSET(source, i)
                    CHECK_MINMAX()
            }
            factor = 256.0 / (double)(max - min);

            FOR_2_CONTINUOUS_IMAGES_WITH_OFFSET(i, i)
                NORMALIZE_VALUE()
        }
    }
    else {
        uint32 x, y;
        DISCONTINUOUS_IMAGE_VARIABLES(byte, byte);
        for (i = 0; i < source->step; i++) {
            if (min == 0 && max == 0) {
                min = PIXEL_VALUE(source);
                max = min;

                FOR_DISCONTINUOUS_IMAGE_WITH_OFFSET(source, i)
                    CHECK_MINMAX()
            }
            factor = 256.0 / (double)(max - min);

            FOR_2_DISCONTINUOUS_IMAGES_WITH_OFFSET(i, i)
                NORMALIZE_VALUE()
        }
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
        for (i = 0; i < source->step; i++) {
            if (min == 0 && max == 0) {
                min = PIXEL_VALUE(source);
                max = min;

                FOR_CONTINUOUS_IMAGE_WITH_OFFSET(source, i)
                    CHECK_MINMAX()
            }
            factor = 256.0 / (double)(max - min);

            FOR_2_CONTINUOUS_IMAGES_WITH_OFFSET(i, i)
                NORMALIZE_VALUE()
        }
    }
    else {
        uint32 x, y;
        DISCONTINUOUS_IMAGE_VARIABLES(char, byte);
        for (i = 0; i < source->step; i++) {
            if (min == 0 && max == 0) {
                min = PIXEL_VALUE(source);
                max = min;

                FOR_DISCONTINUOUS_IMAGE_WITH_OFFSET(source, i)
                    CHECK_MINMAX()
            }
            factor = 256.0 / (double)(max - min);

            FOR_2_DISCONTINUOUS_IMAGES_WITH_OFFSET(i, i)
                NORMALIZE_VALUE()
        }
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
        for (i = 0; i < source->step; i++) {
            if (min == 0 && max == 0) {
                min = PIXEL_VALUE(source);
                max = min;

                FOR_CONTINUOUS_IMAGE_WITH_OFFSET(source, i)
                    CHECK_MINMAX()
            }
            factor = 256.0 / (double)(max - min);

            FOR_2_CONTINUOUS_IMAGES_WITH_OFFSET(i, i)
                NORMALIZE_VALUE()
        }
    }
    else {
        uint32 x, y;
        DISCONTINUOUS_IMAGE_VARIABLES(word, byte);
        for (i = 0; i < source->step; i++) {
            if (min == 0 && max == 0) {
                min = PIXEL_VALUE(source);
                max = min;

                FOR_DISCONTINUOUS_IMAGE_WITH_OFFSET(source, i)
                    CHECK_MINMAX()
            }
            factor = 256.0 / (double)(max - min);

            FOR_2_DISCONTINUOUS_IMAGES_WITH_OFFSET(i, i)
                NORMALIZE_VALUE()
        }
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
        for (i = 0; i < source->step; i++) {
            if (min == 0 && max == 0) {
                min = PIXEL_VALUE(source);
                max = min;

                FOR_CONTINUOUS_IMAGE_WITH_OFFSET(source, i)
                    CHECK_MINMAX()
            }
            factor = 256.0 / (double)(max - min);

            FOR_2_CONTINUOUS_IMAGES_WITH_OFFSET(i, i)
                NORMALIZE_VALUE()
        }
    }
    else {
        uint32 x, y;
        DISCONTINUOUS_IMAGE_VARIABLES(long, byte);
        for (i = 0; i < source->step; i++) {
            if (min == 0 && max == 0) {
                min = PIXEL_VALUE(source);
                max = min;

                FOR_DISCONTINUOUS_IMAGE_WITH_OFFSET(source, i)
                    CHECK_MINMAX()
            }
            factor = 256.0 / (double)(max - min);

            FOR_2_DISCONTINUOUS_IMAGES_WITH_OFFSET(i, i)
                NORMALIZE_VALUE()
        }
    }

    FINALLY(normalize_long);
    RETURN();
NORMALIZE_FUNCTION_END()

/******************************************************************************/

NORMALIZE_FUNCTION_BEGIN(float)
    TRY();

    CHECK_POINTER(source);
    CHECK_POINTER(target);
    CHECK_PARAM(source->type == p_F32);
    CHECK_PARAM(target->type == p_U8);
    CHECK_PARAM((min <= mean) && (mean <= max));

    if (pixel_image_is_continuous(source) && pixel_image_is_continuous(target)) {
        CONTINUOUS_IMAGE_VARIABLES(float, byte);
        for (i = 0; i < source->step; i++) {
            if (min == 0 && max == 0) {
                min = PIXEL_VALUE(source);
                max = min;

                FOR_CONTINUOUS_IMAGE_WITH_OFFSET(source, i)
                    CHECK_MINMAX()
            }
            factor = 256.0 / (float)(max - min);

            FOR_2_CONTINUOUS_IMAGES_WITH_OFFSET(i, i)
                NORMALIZE_VALUE()
        }
    }
    else {
        uint32 x, y;
        DISCONTINUOUS_IMAGE_VARIABLES(float, byte);
        for (i = 0; i < source->step; i++) {
            if (min == 0 && max == 0) {
                min = PIXEL_VALUE(source);
                max = min;

                FOR_DISCONTINUOUS_IMAGE_WITH_OFFSET(source, i)
                    CHECK_MINMAX()
            }
            factor = 256.0 / (float)(max - min);

            FOR_2_DISCONTINUOUS_IMAGES_WITH_OFFSET(i, i)
                NORMALIZE_VALUE()
        }
    }

    FINALLY(normalize_float);
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
        for (i = 0; i < source->step; i++) {
            if (min == 0 && max == 0) {
                min = PIXEL_VALUE(source);
                max = min;

                FOR_CONTINUOUS_IMAGE_WITH_OFFSET(source, i)
                    CHECK_MINMAX()
            }
            factor = 256.0 / (double)(max - min);

            FOR_2_CONTINUOUS_IMAGES_WITH_OFFSET(i, i)
                NORMALIZE_VALUE()
        }
    }
    else {
        uint32 x, y;
        DISCONTINUOUS_IMAGE_VARIABLES(double, byte);
        for (i = 0; i < source->step; i++) {
            if (min == 0 && max == 0) {
                min = PIXEL_VALUE(source);
                max = min;

                FOR_DISCONTINUOUS_IMAGE_WITH_OFFSET(source, i)
                    CHECK_MINMAX()
            }
            factor = 256.0 / (double)(max - min);

            FOR_2_DISCONTINUOUS_IMAGES_WITH_OFFSET(i, i)
                NORMALIZE_VALUE()
        }
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

result convert_grey8_to_yuv24(
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
    CHECK_PARAM(target->format == YUV);
    CHECK_PARAM(source->width == target->width);
    CHECK_PARAM(source->height == target->height);

    if (pixel_image_is_continuous(source) && pixel_image_is_continuous(target)) {
        CONTINUOUS_IMAGE_VARIABLES(byte, byte);
        FOR_2_CONTINUOUS_IMAGES()
        {
            PIXEL_VALUE(target) = PIXEL_VALUE(source);
            PIXEL_VALUE_PLUS(target, 1) = 128;
            PIXEL_VALUE_PLUS(target, 2) = 128;
        }
    }
    else {
        uint32 x, y;
        DISCONTINUOUS_IMAGE_VARIABLES(byte, byte);
        FOR_2_DISCONTINUOUS_IMAGES()
        {
            PIXEL_VALUE(target) = PIXEL_VALUE(source);
            PIXEL_VALUE_PLUS(target, 1) = 128;
            PIXEL_VALUE_PLUS(target, 2) = 128;
        }
    }

    FINALLY(convert_grey8_to_yuv24);
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

result convert_rgb24_to_yuv24(
    const pixel_image *source,
    pixel_image *target
    )
{
    TRY();
    byte R, G, B;
    /*sint32 Y, U, V;*/
    double  Y, U, V;

    CHECK_POINTER(source);
    CHECK_POINTER(target);
    CHECK_POINTER(source->data);
    CHECK_POINTER(target->data);
    CHECK_PARAM(source->type == p_U8);
    CHECK_PARAM(target->type == p_U8);
    CHECK_PARAM(source->step == 3);
    CHECK_PARAM(target->step == 3);
    CHECK_PARAM(source->format == RGB);
    CHECK_PARAM(target->format == YUV);
    CHECK_PARAM(source->width == target->width);
    CHECK_PARAM(source->height == target->height);

    if (pixel_image_is_continuous(source) && pixel_image_is_continuous(target)) {
        CONTINUOUS_IMAGE_VARIABLES(byte, byte);
        FOR_2_CONTINUOUS_IMAGES()
        {
            R = PIXEL_VALUE(source);
            G = PIXEL_VALUE_PLUS(source, 1);
            B = PIXEL_VALUE_PLUS(source, 2);
            Y = ( 0.29900 * ((double)R / 255.0)) + ( 0.58700 * ((double)G / 255.0)) + ( 0.11400 * ((double)B / 255.0));
            U = (-0.14713 * ((double)R / 255.0)) + (-0.28886 * ((double)G / 255.0)) + ( 0.43600 * ((double)B / 255.0));
            V = ( 0.61500 * ((double)R / 255.0)) + (-0.51499 * ((double)G / 255.0)) + (-0.10001 * ((double)B / 255.0));
            /*
            Y = ( 77 * R) + (150 * G) + ( 29 * B) + 128;
            U = (-38 * R) + (-74 * G) + (112 * B) + 128;
            V = (112 * R) + (-94 * G) + (-18 * B) + 128;
            Y >>= 8;
            U >>= 8;
            V >>= 8;
            */
            PIXEL_VALUE(target)         = (byte)(Y * 255);
            PIXEL_VALUE_PLUS(target, 1) = (byte)(((U + 0.436) / (2 * 0.436)) * 255);
            PIXEL_VALUE_PLUS(target, 2) = (byte)(((V + 0.615) / (2 * 0.615)) * 255);
        }
    }
    else {
        uint32 x, y;
        DISCONTINUOUS_IMAGE_VARIABLES(byte, byte);
        FOR_2_DISCONTINUOUS_IMAGES()
        {
            R = PIXEL_VALUE(source);
            G = PIXEL_VALUE_PLUS(source, 1);
            B = PIXEL_VALUE_PLUS(source, 2);
            Y = ( 0.29900 * ((double)R / 255.0)) + ( 0.58700 * ((double)G / 255.0)) + ( 0.11400 * ((double)B / 255.0));
            U = (-0.14713 * ((double)R / 255.0)) + (-0.28886 * ((double)G / 255.0)) + ( 0.43600 * ((double)B / 255.0));
            V = ( 0.61500 * ((double)R / 255.0)) + (-0.51499 * ((double)G / 255.0)) + (-0.10001 * ((double)B / 255.0));
            /*
            Y = ( 77 * R) + (150 * G) + ( 29 * B) + 128;
            U = (-38 * R) + (-74 * G) + (112 * B) + 128;
            V = (112 * R) + (-94 * G) + (-18 * B) + 128;
            Y >>= 8;
            U >>= 8;
            V >>= 8;
            */
            PIXEL_VALUE(target)         = (byte)(Y * 255);
            PIXEL_VALUE_PLUS(target, 1) = (byte)(((U + 0.436) / (2 * 0.436)) * 255);
            PIXEL_VALUE_PLUS(target, 2) = (byte)(((V + 0.615) / (2 * 0.615)) * 255);
        }
    }

    FINALLY(convert_rgb24_to_yuv24);
    RETURN();
}

/******************************************************************************/

result convert_yuv24_to_rgb24(
    const pixel_image *source,
    pixel_image *target
    )
{
    TRY();
    byte Yb, Ub, Vb;
    /*sint32 Y, U, V;*/
    double Y, U, V, R, G, B;

    CHECK_POINTER(source);
    CHECK_POINTER(target);
    CHECK_POINTER(source->data);
    CHECK_POINTER(target->data);
    CHECK_PARAM(source->type == p_U8);
    CHECK_PARAM(target->type == p_U8);
    CHECK_PARAM(source->step == 3);
    CHECK_PARAM(target->step == 3);
    CHECK_PARAM(source->format == YUV);
    CHECK_PARAM(target->format == RGB);
    CHECK_PARAM(source->width == target->width);
    CHECK_PARAM(source->height == target->height);

    if (pixel_image_is_continuous(source) && pixel_image_is_continuous(target)) {
        CONTINUOUS_IMAGE_VARIABLES(byte, byte);
        FOR_2_CONTINUOUS_IMAGES()
        {
            Yb = PIXEL_VALUE(source);
            Ub = PIXEL_VALUE_PLUS(source, 1);
            Vb = PIXEL_VALUE_PLUS(source, 2);
            Y = ((double)Yb / 255.0);
            U = (((double)Ub / 255.0) * 2.0 * 0.436) - 0.436;
            V = (((double)Vb / 255.0) * 2.0 * 0.615) - 0.615;
            R = ( Y + 0.00000 * U + 1.13983 * V);
            G = ( Y - 0.39465 * U - 0.58060 * V);
            B = ( Y + 2.03211 * U + 0.00000 * V);
            /*
            Y = ( 77 * R) + (150 * G) + ( 29 * B) + 128;
            U = (-38 * R) + (-74 * G) + (112 * B) + 128;
            V = (112 * R) + (-94 * G) + (-18 * B) + 128;
            Y >>= 8;
            U >>= 8;
            V >>= 8;
            */
            PIXEL_VALUE(target)         = (byte)(R * 255);
            PIXEL_VALUE_PLUS(target, 1) = (byte)(G * 255);
            PIXEL_VALUE_PLUS(target, 2) = (byte)(B * 255);
        }
    }
    else {
        uint32 x, y;
        DISCONTINUOUS_IMAGE_VARIABLES(byte, byte);
        FOR_2_DISCONTINUOUS_IMAGES()
        {
            Yb = PIXEL_VALUE(source);
            Ub = PIXEL_VALUE_PLUS(source, 1);
            Vb = PIXEL_VALUE_PLUS(source, 2);
            Y = ((double)Yb / 255.0);
            U = (((double)Ub / 255.0) * 2.0 * 0.436) - 0.436;
            V = (((double)Vb / 255.0) * 2.0 * 0.615) - 0.615;
            R = ( Y + 0.00000 * U + 1.13983 * V);
            G = ( Y - 0.39465 * U - 0.58060 * V);
            B = ( Y + 2.03211 * U + 0.00000 * V);
            /*
            Y = ( 77 * R) + (150 * G) + ( 29 * B) + 128;
            U = (-38 * R) + (-74 * G) + (112 * B) + 128;
            V = (112 * R) + (-94 * G) + (-18 * B) + 128;
            Y >>= 8;
            U >>= 8;
            V >>= 8;
            */
            PIXEL_VALUE(target)         = (byte)(R * 255);
            PIXEL_VALUE_PLUS(target, 1) = (byte)(G * 255);
            PIXEL_VALUE_PLUS(target, 2) = (byte)(B * 255);
        }
    }

    FINALLY(convert_yuv24_to_rgb24);
    RETURN();
}

/******************************************************************************/

result convert_yuv24_to_grey8(
    const pixel_image *source,
    pixel_image *target
    )
{
    TRY();
    byte Y;

    CHECK_POINTER(source);
    CHECK_POINTER(target);
    CHECK_POINTER(source->data);
    CHECK_POINTER(target->data);
    CHECK_PARAM(source->type == p_U8);
    CHECK_PARAM(target->type == p_U8);
    CHECK_PARAM(source->step == 3);
    CHECK_PARAM(target->step == 1);
    CHECK_PARAM(source->format == YUV);
    CHECK_PARAM(target->format == GREY);
    CHECK_PARAM(source->width == target->width);
    CHECK_PARAM(source->height == target->height);

    if (pixel_image_is_continuous(source) && pixel_image_is_continuous(target)) {
        CONTINUOUS_IMAGE_VARIABLES(byte, byte);
        FOR_2_CONTINUOUS_IMAGES()
        {
            Y = PIXEL_VALUE(source);
            PIXEL_VALUE(target) = Y;
        }
    }
    else {
        uint32 x, y;
        DISCONTINUOUS_IMAGE_VARIABLES(byte, byte);
        FOR_2_DISCONTINUOUS_IMAGES()
        {
            Y = PIXEL_VALUE(source);
            PIXEL_VALUE(target) = Y;
        }
    }

    FINALLY(convert_yuv24_to_grey8);
    RETURN();
}

/******************************************************************************/

result pick_1_channel_from_3_channels(
    const pixel_image *source,
    pixel_image *target,
    uint32 channel
    )
{
    TRY();

    CHECK_POINTER(source);
    CHECK_POINTER(target);
    CHECK_POINTER(source->data);
    CHECK_POINTER(target->data);
    CHECK_PARAM(source->type == p_U8);
    CHECK_PARAM(target->type == p_U8);
    CHECK_PARAM(source->step == 3);
    CHECK_PARAM(target->step == 1);
    CHECK_PARAM(target->format == GREY);
    CHECK_PARAM(source->width == target->width);
    CHECK_PARAM(source->height == target->height);
    CHECK_PARAM(channel < 3);

    if (pixel_image_is_continuous(source) && pixel_image_is_continuous(target)) {
        CONTINUOUS_IMAGE_VARIABLES(byte, byte);
        FOR_2_CONTINUOUS_IMAGES()
        {
            PIXEL_VALUE(target) = PIXEL_VALUE_PLUS(source, channel);
        }
    }
    else {
        uint32 x, y;
        DISCONTINUOUS_IMAGE_VARIABLES(byte, byte);
        FOR_2_DISCONTINUOUS_IMAGES()
        {
            PIXEL_VALUE(target) = PIXEL_VALUE_PLUS(source, channel);
        }
    }

    FINALLY(pick_1_channel_from_3_channels);
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

image_rect pixel_image_create_rect
  (
  pixel_image *target,
  sint32 x,
  sint32 y,
  sint32 dx,
  sint32 dy,
  uint32 offset
  )
{
  image_rect rect;
  register uint32 width, height, step, stride;

  width = target->width;
  height = target->height;

  rect.valid = 0;
  if (x < 0) {
    dx = dx + x;
    x = 0;
  }
  if (y < 0) {
    dy = dy + y;
    y = 0;
  }
  if ((unsigned)x < width && (unsigned)y < height) {
    if (dx > 0 && dy > 0) {
      if (x + dx > width) dx = width - x;
      if (y + dy > height) dy = height - y;

      step = target->step;
      stride = target->stride;
      rect.valid = 1;
      rect.offset = ((unsigned)y) * stride + ((unsigned)x) * step + offset;
      rect.hstep = ((unsigned)dx);
      rect.vstep = ((unsigned)dy);
      rect.N = ((unsigned)dx) * ((unsigned)dy);
    }
  }

  return rect;
}

/******************************************************************************/

I_value pixel_image_find_min_byte
(
  pixel_image *target,
  sint32 x,
  sint32 y,
  sint32 dx,
  sint32 dy,
  uint32 offset
)
{
  image_rect rect;
  I_value min, value;

  rect = pixel_image_create_rect(target, x, y, dx, dy, offset);
  if (rect.valid == 0) {
    return 0;
  }
  else {
    IMAGE_RECT_VARIABLES(target, byte, rect);
    min = 255;
    FOR_IMAGE_RECT_BEGIN(target, rect)
      value = (I_value)PIXEL_VALUE(target);
      if (value < min) min = value;
    FOR_IMAGE_RECT_END(target);
    return min;
  }
}

/******************************************************************************/

I_value pixel_image_find_max_byte
(
  pixel_image *target,
  sint32 x,
  sint32 y,
  sint32 dx,
  sint32 dy,
  uint32 offset
)
{
  image_rect rect;
  I_value max, value;

  rect = pixel_image_create_rect(target, x, y, dx, dy, offset);
  if (rect.valid == 0) {
    return 0;
  }
  else {
    IMAGE_RECT_VARIABLES(target, byte, rect);
    max = 0;
    FOR_IMAGE_RECT_BEGIN(target, rect)
      value = (I_value)PIXEL_VALUE(target);
      if (value > max) max = value;
    FOR_IMAGE_RECT_END(target);
    return max;
  }
}

/******************************************************************************/

I_value pixel_image_calculate_mean_byte
(
  pixel_image *target,
  sint32 x,
  sint32 y,
  sint32 dx,
  sint32 dy,
  uint32 offset
)
{
  image_rect rect;
  I_value sum, mean;

  rect = pixel_image_create_rect(target, x, y, dx, dy, offset);
  if (rect.valid == 0) {
    return 0;
  }
  else {
    IMAGE_RECT_VARIABLES(target, byte, rect);
    sum = 0;
    FOR_IMAGE_RECT_BEGIN(target, rect)
      sum += (I_value)PIXEL_VALUE(target);
    FOR_IMAGE_RECT_END(target);
    mean = sum / ((I_value)rect.N);
    return mean;
  }
}

/******************************************************************************/

I_value pixel_image_calculate_variance_byte
(
  pixel_image *target,
  sint32 x,
  sint32 y,
  sint32 dx,
  sint32 dy,
  uint32 offset
)
{
  image_rect rect;
  I_value value, sum1, sum2, N, mean, variance;

  rect = pixel_image_create_rect(target, x, y, dx, dy, offset);
  if (rect.valid == 0) {
    return 0;
  }
  else {
    IMAGE_RECT_VARIABLES(target, byte, rect);
    sum1 = 0;
    sum2 = 0;
    FOR_IMAGE_RECT_BEGIN(target, rect)
      value = (I_value)PIXEL_VALUE(target);
      sum1 += value;
      sum2 += value*value;
    FOR_IMAGE_RECT_END(target);
    N = (I_value)rect.N;
    mean = sum1 / N;
    variance = sum2 / N - mean*mean;
    return variance;
  }
}

/******************************************************************************/
