/**
 * @file cvsu_graph.h
 * @author Matti J. Eskelinen <matti.j.eskelinen@gmail.com>
 * @brief A generic attributed graph structure.
 *
 * Copyright (c) 2014, Matti Johannes Eskelinen
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

#ifndef CVSU_GRAPH_H
#   define CVSU_GRAPH_H

#ifdef __cplusplus
extern "C" {
#endif

#include "cvsu_types.h"
#include "cvsu_typed_pointer.h"
#include "cvsu_list.h"
#include "cvsu_pixel_image.h"

/******************************************************************************/

typedef struct attribute_t {
  uint32 key;
  typed_pointer value;
} attribute;

attribute *attribute_alloc();

void attribute_free
(
  attribute *ptr
);

result attribute_create
(
  attribute *target,
  uint32 key,
  typed_pointer *value
);

void attribute_destroy
(
  attribute *target
);

void attribute_nullify
(
  attribute *target
);

truth_value attribute_is_null
(
  attribute *target
);

/******************************************************************************/

typedef struct attribute_list_t {
  attribute *items;
  uint32 size;
  uint32 count;
} attribute_list;

attribute_list *attribute_list_alloc();

void attribute_list_free
(
  attribute_list *ptr
);

result attribute_list_create
(
  attribute_list *target,
  uint32 size
);

void attribute_list_destroy
(
  attribute_list *target
);

void attribute_list_nullify
(
  attribute_list *target
);

truth_value attribute_list_is_null
(
  attribute_list *target
);

result attribute_add
(
  attribute_list *target,
  attribute *source,
  attribute **added
);

attribute *attribute_find
(
  attribute_list *source,
  uint32 key
);

/******************************************************************************/

/**
 * Defines a generic graph node structure for use in sparse graphs, where most
 * nodes have only a few neighbors. Edges are stored within nodes.
 */
typedef struct node_t {
  integral_value x;
  integral_value y;
  integral_value orientation;
  uint32 scale;
  attribute_list attributes;
  attribute_list links;
} node;

result node_create
(
  node *target,
  uint32 attr_size,
  uint32 link_size
);

void node_destroy
(
  node *target
);

void node_nullify
(
  node *target
);

truth_value node_is_null
(
  node *target
);

/******************************************************************************/

/* forward declaration */
struct link_t;

typedef struct link_head_t {
  struct link_t *body;
  struct link_head_t *other;
  struct node_t *origin;
  attribute_list attributes;
} link_head;

/******************************************************************************/

/**
 * Defines a generic graph edge with two heads. Each node has one head.
 */
typedef struct link_t {
  struct link_head_t a;
  struct link_head_t b;
  integral_value weight;
  attribute_list attributes;
} link;

/******************************************************************************/

typedef struct graph_t {
  list nodes;
  list links;
  attribute_list sources;
} graph;

typedef enum graph_neighborhood_t {
  NEIGHBORHOOD_0 = 0,
  NEIGHBORHOOD_4 = 4,
  NEIGHBORHOOD_6 = 6,
  NEIGHBORHOOD_8 = 8
} graph_neighborhood;

/**
 * Allocates memory for a graph structure and returns the pointer.
 */
graph *graph_alloc();

/**
 * Frees the memory allocated for a graph structure, and destroys the structure.
 */
void graph_free
(
  graph *ptr
);

/**
 * Creates an empty graph with a pre-allocated node and link list. An attribute
 * can be given, indicating the key and value type for node attributes.
 */
result graph_create
(
  graph *target,
  uint32 node_size,
  uint32 link_size,
  attribute *attr_label
);

/**
 * Destroys a graph and deallocates all memory
 */
void graph_destroy
(
  graph *target
);

void graph_nullify
(
  graph *target
);

truth_value graph_is_null
(
  graph *target
);

/**
 * Creates a regular grid graph structure from a single image. The offset from
 * the top left corner of the image before the first node and the step between
 * nodes, both expressed in pixels, can be given. The attr_label specifies
 * which key should be used for the attribute storing the pixel values, as well
 * as the type used in the attribute.
 */
result graph_create_from_image
(
  graph *target,
  pixel_image *source,
  uint32 node_offset_x,
  uint32 node_offset_y,
  uint32 node_step_x,
  uint32 node_step_y,
  graph_neighborhood neighborhood,
  attribute *attr_label
);

#ifdef __cplusplus
}
#endif

#endif /* CVSU_GRAPH_H */
