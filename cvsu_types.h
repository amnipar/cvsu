/**
 * @file cvsu_types.h
 * @author Matti J. Eskelinen (matti dot j dot eskelinen at jyu dot fi)
 * @brief Generic type definitions for the cvsu module.
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

#ifndef CVSUTYPES_H
#   define CVSUTYPES_H

/*#include <stddef.h>*/
/*#include <stdbool.h>*/

typedef long unsigned int size_t;

#ifndef __cplusplus
#ifndef __bool_true_false_are_defined
typedef long unsigned int bool;
#define true	1
#define false	0
#endif
#endif

#ifndef NULL
#ifndef __cplusplus
#define NULL ((void *)0)
#else   /* C++ */
#define NULL 0
#endif  /* C++ */
#endif

typedef unsigned char  byte;
typedef unsigned short word;
typedef unsigned long  dword;
typedef void *         pointer;
typedef byte *         data_pointer;
typedef const char *   string;

typedef signed char    sint8;
typedef unsigned char  uint8;
typedef signed short   sint16;
typedef unsigned short uint16;
typedef signed long    sint32;
typedef unsigned long  uint32;
typedef float          real32;
typedef double         real64;

typedef uint32 I_1_t;
typedef real64 I_2_t;

typedef struct point_t {
    uint16 x;
    uint16 y;
} point;

typedef struct line_t {
    point start;
    point end;
} line;

typedef struct rect_t {
    point top_left;
    point bottom_right;
} rect;

typedef struct statistics_t {
    sint16 mean;
    sint16 dev;
} statistics;

typedef struct fstatistics_t {
    double mean;
    double dev;
} fstatistics;

typedef enum result_t {
    SUCCESS = 0,
    CAUGHT_ERROR,
    BAD_POINTER,
    BAD_TYPE,
    BAD_SIZE,
    BAD_PARAM,
    NOT_FOUND,
    NOT_IMPLEMENTED
} result;
/*
typedef enum error_code_t {
    SUCCESS = 0,
    CAUGHT_ERROR,
    BAD_POINTER,
    BAD_TYPE,
    BAD_SIZE,
    BAD_PARAM,
    NOT_FOUND,
    NOT_IMPLEMENTED
} error_code;
*/
typedef struct fresult_t {
    result /*error_code*/ code;
    string function;
} fresult;

/**
 * Different types used for storing pixel values.
 */
typedef enum pixel_type_t {
    /** unsigned char (byte) values */
    p_U8 = 0,
    /** signed char values */
    p_S8,
    /** unsigned 16-bit integer (short) values */
    p_U16,
    /** signed 16-bit integer (short) values */
    p_S16,
    /** unsigned 32-bit integer (long) values */
    p_U32,
    /** signed 32-bit integer (long) values */
    p_S32,
    /** unsigned 64-bit integer (long long) values */
    p_U64,
    /** signed 64-bit integer (long long) values */
    p_S64,
    /** 32-bit floating point (float) values */
    p_F32,
    /** 64-bit floating point values (double) */
    p_F64
} pixel_type;

#define BYTE_IMAGE p_U8
#define CHAR_IMAGE p_S8
#define WORD_IMAGE p_U16
#define DWORD_IMAGE p_U32
#define LONG_IMAGE p_S32
#define FLOAT_IMAGE p_F32
#define DOUBLE_IMAGE p_F64

/**
 * Different one-channel and multi-channel pixel formats.
 */
typedef enum pixel_format_t {
    /** one-channel greyscale image */
    GREY = 0,
    /** three-channel image with RGB values */
    RGB,
    /** four-channel image with RGBA values */
    RGBA,
    /** three-channel image with HSV values */
    HSV,
    /** three-channel image with LAB values */
    LAB,
    /** two-channel image with UYVY values */
    UYVY
} pixel_format;

#endif /* CVSU_TYPES_H */
