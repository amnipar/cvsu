/**
 * @file cvsu_output.h
 * @author Matti J. Eskelinen <matti.j.eskelinen@gmail.com>
 * @brief Output handling functions for cvsu.
 *
 * Copyright (c) 2011-2013, Matti Johannes Eskelinen
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

#ifndef CVSU_OUTPUT_H
#   define CVSU_OUTPUT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "cvsu_config.h"
#include "cvsu_types.h"

#if (OUTPUT_METHOD == OUTPUT_WITH_STDIO)
#include <stdio.h>
#endif

void report_result(result r, string func);

#if (OUTPUT_METHOD == OUTPUT_DISABLED)

#define PRINT0(f)
#define PRINT1(f,a)
#define PRINT2(f,a,b)

#elif (OUTPUT_METHOD == OUTPUT_WITH_STDIO)

#define PRINT0(f)       printf(f)
#define PRINT1(f, a)    printf(f, a)
#define PRINT2(f, a, b) printf(f, a, b)

#else
#error "Output method not specified"
#endif

#ifdef __cplusplus
}
#endif

#endif  /* CVSU_OUTPUT_H */
