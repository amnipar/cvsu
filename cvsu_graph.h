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
#include "cvsu_attribute.h"
#include "cvsu_list.h"
#include "cvsu_pixel_image.h"

/******************************************************************************/

/* forward declaration */
struct link_t;

typedef struct link_head_t {
  struct link_t *body;
  struct link_head_t *other;
  struct node_t *origin;
  direction dir;
  attribute_list attributes;
} link_head;

/**
 * Defines a generic graph edge with two heads. Each node has one head.
 */
typedef struct link_t {
  struct link_head_t a;
  struct link_head_t b;
  real *weight;
  attribute_list attributes;
} link;

typedef result (*link_function)(link *target, pointer params);

result link_create
(
  link *target,
  uint32 attr_size
);

void link_destroy
(
  link *target
);

void link_nullify
(
  link *target
);

truth_value link_is_null
(
  link *target
);

result link_weight_range_update
(
  link *target,
  pointer params
);

result link_attribute_range_update
(
  link *target,
  pointer params
);

/******************************************************************************/

typedef struct link_list_t {
  link_head **items;
  uint32 size;
  uint32 count;
} link_list;

link_list *link_list_alloc();

void link_list_free
(
  link_list *ptr
);

result link_list_create
(
  link_list *target,
  uint32 size
);

void link_list_destroy
(
  link_list *target
);

void link_list_nullify
(
  link_list *target
);

truth_value link_list_is_null
(
  link_list *target
);

result link_list_add
(
  link_list *target,
  link_head *source
);

result link_list_add_in_pos
(
  link_list *target,
  link_head *source,
  uint32 pos
);

/* adding and finding links are functions of the graph. */

/******************************************************************************/

/**
 * Defines a generic graph node structure for use in sparse graphs, where most
 * nodes have only a few neighbors. Links are stored within nodes.
 * All nodes have an id and a 2d position. For n-dimensional nodes, the 2d
 * position is used for visualizations.
 */
typedef struct node_t {
  uint32 id;
  position_2d *pos;
  real *weight;
  attribute_list attributes;
  link_list links;
} node;

typedef result (*node_function)(node *target, pointer params);

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

truth_value node_has_link_to
(
  node *node_1,
  node *node_2
);

result node_weight_range_update
(
  node *target,
  pointer params
);

result node_attribute_range_update
(
  node *target,
  pointer params
);

result node_ref_attribute_add
(
  attribute_list *target,
  uint32 key,
  node *source,
  node ***added
);

node *node_ref_attribute_get
(
  attribute_list *target,
  uint32 key
);

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

/*
typedef struct graph_visualization_params_t {
} graph_visualization_params;
*/

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
  uint32 link_size
);

/**
 * Destroys a graph and deallocates all memory
 */
void graph_destroy
(
  graph *target
);

/**
 * Sets the graph to the NULL (default) state.
 */
void graph_nullify
(
  graph *target
);

/**
 * Checks whether the graph is in the NULL (default) state.
 */
truth_value graph_is_null
(
  graph *target
);

result graph_add_node
(
  graph *target,
  uint32 attr_size,
  uint32 link_size,
  node **added
);

result graph_add_link
(
  graph *target,
  uint32 attr_size,
  link **added
);

result graph_link_nodes
(
  link *target,
  node *node_a,
  node *node_b
);

result graph_for_each_node
(
  graph *target,
  node_function func,
  pointer params
);

result graph_for_attrs_in_each_node
(
  graph *target,
  attribute_list_function func,
  pointer params
);

result graph_for_each_link
(
  graph *target,
  link_function func,
  pointer params
);

result graph_for_attrs_in_each_link
(
  graph *target,
  attribute_list_function func,
  pointer params
);

/**
 * Creates a regular grid graph structure from a single image. Pixel values are
 * stored in the nodes as value attributes (using pixel_value types, which
 * stores values as doubles). The offset from the top left corner of the image
 * before the first node and the step between nodes, both expressed in pixels,
 * can be given. The value_attr argument tells which key should be used for the
 * value attribute storing the pixel values, as well as the type used in the
 * attribute.
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
  uint32 pos_key,
  uint32 value_key,
  uint32 weight_key
);

#ifdef __cplusplus
}
#endif

#endif /* CVSU_GRAPH_H */
