/**
* @file cvsu_connected_components.h
* @author Matti J. Eskelinen <matti.j.eskelinen@gmail.com>
* @brief Connected components handling for the cvsu module.
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

#ifndef CVSU_CONNECTED_COMPONENTS_H
#define CVSU_CONNECTED_COMPONENTS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "cvsu_config.h"
#include "cvsu_types.h"
#include "cvsu_basic.h"

/**
 * Stores region information for connected component analysis with union-find
 * equivalence class approach. In addition to id and rank information contains
 * also the region bounding box.
 */
typedef struct region_t
{
  struct region_t *id;
  uint32 rank;
  uint32 x1;
  uint32 y1;
  uint32 x2;
  uint32 y2;
} region;

/**
 * Stores the connected components information extracted from a pixel image.
 * The information is stored as an array of region structures, one for each
 * pixel of the image.
 */
typedef struct connected_components_t
{
  pixel_image *original;
  region *regions;
} connected_components;

#ifdef __cplusplus
}
#endif

#endif /* CVSU_CONNECTED_COMPONENTS_H */
