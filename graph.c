/**
 * @file graph.c
 * @author Matti J. Eskelinen <matti.j.eskelinen@gmail.com>
 * @brief Simple program to demonstrate graph-based image analysis operations.
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

#include "cvsu_macros.h"
#include "cvsu_output.h"
#include "cvsu_pixel_image.h"
#include "cvsu_typed_pointer.h"
#include "cvsu_attribute.h"
#include "cvsu_graph.h"
#include "cvsu_set.h"
#include "cvsu_opencv.h"

#include <string.h>

string union_for_smaller_than_name = "union_for_smaller_than";
string node_for_each_set_name = "node_for_each_set";
string link_for_neighboring_sets_name = "link_for_neighboring_sets";
string main_name = "graph";

const uint32 POS_ATTR = 1;
const uint32 VALUE_ATTR = 2;
const uint32 WEIGHT_ATTR = 3;
const uint32 SET_ATTR = 4;
const uint32 SET_STAT_ATTR = 5;
const uint32 SET_POS_ATTR = 6;
const uint32 SET_NODE_ATTR = 7;
const uint32 SET_COLOR_ATTR = 8;

result union_for_smaller_than
(
  link *target,
  pointer params
)
{
  TRY();
  real a, b, threshold;
  node *node_a, *node_b;
  disjoint_set *set_a, *set_b;
  
  CHECK_POINTER(target);
  CHECK_POINTER(params);
  
  threshold = *(real*)params;
  node_a = target->a.origin;
  node_b = target->b.origin;
  
  set_a = disjoint_set_attribute_get(&node_a->attributes, SET_ATTR);
  if (set_a == NULL) {
    TERMINATE(NOT_FOUND);
  }
  a = *node_a->weight;
  set_b = disjoint_set_attribute_get(&node_b->attributes, SET_ATTR);
  if (set_b == NULL) {
    TERMINATE(NOT_FOUND);
  }
  b = *node_b->weight;
  
  if (fabs(a-b) < threshold) {
    disjoint_set_union(set_a, set_b);
  }
  
  FINALLY(union_for_smaller_than);
  RETURN();
}

result node_for_each_set
(
  node *target,
  pointer params
)
{
  TRY();
  graph *g;
  node *node_ptr;
  disjoint_set *set, *parent;
  position_2d *node_pos;
  attribute_2d_pos *set_pos;
  attribute_2d_pos_acc acc;
  
  CHECK_POINTER(target);
  CHECK_POINTER(params);
  
  g = (graph*)params;
  
  set = disjoint_set_attribute_get(&target->attributes, SET_ATTR);
  if (set == NULL) {
    TERMINATE(NOT_FOUND);
  }
  parent = disjoint_set_find(set);
  
  if (set == parent && set->size > 0) {
    node_ptr = node_ref_attribute_get(&set->attributes, SET_NODE_ATTR);
    if (node_ptr == NULL) {
      CHECK(graph_add_node(g, 4, 100, &node_ptr));
      set_pos = attribute_2d_pos_attribute_get(&set->attributes, SET_POS_ATTR);
      if (set_pos == NULL) {
        node_ptr->pos = target->pos;
      }
      else {
        attribute_2d_pos_get(set_pos, &acc);
        CHECK(position_2d_attribute_add(&node_ptr->attributes, POS_ATTR,
                                        acc.cx, acc.cy, &node_pos));
        node_ptr->pos = node_pos;
      }
      CHECK(node_ref_attribute_add(&set->attributes, SET_NODE_ATTR, node_ptr,
                                   NULL));
    }
  }
  
  FINALLY(node_for_each_set);
  RETURN();
}

result link_for_neighboring_sets
(
  link *target,
  pointer params
)
{
  TRY();
  graph *g;
  node *node_a1, *node_a2, *node_b1, *node_b2;
  disjoint_set *set_a, *set_b;
  link *link_ptr;
  
  CHECK_POINTER(target);
  CHECK_POINTER(params);
  
  g = (graph*)params;
  node_a1 = target->a.origin;
  node_b1 = target->b.origin;
  /*
   * check if the two nodes belong to different sets
   * find the second-level nodes for the sets
   * check if there is a link between the nodes
   * if not, add it
   */
  set_a = disjoint_set_find(disjoint_set_attribute_get(&node_a1->attributes,
                                                       SET_ATTR));
  set_b = disjoint_set_find(disjoint_set_attribute_get(&node_b1->attributes,
                                                       SET_ATTR));
  if (set_a != NULL && set_b != NULL && set_a != set_b) {
    node_a2 = node_ref_attribute_get(&set_a->attributes, SET_NODE_ATTR);
    node_b2 = node_ref_attribute_get(&set_b->attributes, SET_NODE_ATTR);
    if (node_a2 != NULL && node_b2 != NULL && node_a2 != node_b2) {
      if (IS_FALSE(node_has_link_to(node_a2, node_b2))) {
        /*
        printf("link list %lu %lu\n", node_a2->links.size, node_a2->links.count);
        printf("link list %lu %lu\n", node_b2->links.size, node_b2->links.count);
        */
        CHECK(graph_add_link(g, 4, &link_ptr));
        CHECK(graph_link_nodes(link_ptr, node_a2, node_b2));
      }
    }
  }
  
  FINALLY(link_for_neighboring_sets);
  RETURN();
}

void print_usage()
{
  PRINT0("graph\n");
  PRINT0("Performs various operations using image graph representations.\n\n");
  PRINT0("Usage:\n\n");
  PRINT0("graph mode stepx stepy source target\n");
  PRINT0("  mode: analysis mode [ connected | msf | contour ]\n");
  PRINT0("  stepx: horizontal step in pixels between nodes\n");
  PRINT0("  stepy: vertical step in pixels between nodes\n");
  PRINT0("  source: source image file to process\n");
  PRINT0("  target: target image file to generate\n\n");
}

enum mode_t {
  m_UNDEF = 0,
  m_CONNECTED,
  m_MSF,
  m_CONTOUR
};

int main(int argc, char *argv[])
{
  TRY();
  pixel_image src_image, tmp_image, dst_image;
  graph g, greg;
  uint32 stepx, stepy;
  string smode, source_file, target_file;
  enum mode_t mode;
  
  if (argc < 6) {
    PRINT0("\nError: wrong number of parameters\n\n");
    print_usage();
    return 1;
  }
  else {
    int scan_result;

    mode = m_UNDEF;
    smode = argv[1];
    if (strcmp(smode, "connected") == 0) {
      mode = m_CONNECTED;
    }
    else
    if (strcmp(smode, "msf") == 0) {
      mode = m_MSF;
    }
    else
    if (strcmp(smode, "contour") == 0) {
      mode = m_CONTOUR;
    }
    else {
      PRINT1("\nError: unsupported mode (%s)\n\n", smode);
      print_usage();
      return 1;
    }

    scan_result = sscanf(argv[2], "%lu", &stepx);
    if (scan_result != 1) {
      PRINT0("\nError: failed to parse parameter stepx\n\n");
      print_usage();
      return 1;
    }
    scan_result = sscanf(argv[3], "%lu", &stepy);
    if (scan_result != 1) {
      PRINT0("\nError: failed to parse parameter stepy\n\n");
      print_usage();
      return 1;
    }
    source_file = argv[4];
    target_file = argv[5];
    if (stepx < 1) {
      PRINT0("\nError: stepx may not be smaller than 1\n\n");
      print_usage();
      return 1;
    }
    if (stepy < 1) {
      PRINT0("\nError: stepy may not be smaller than 1\n\n");
      print_usage();
      return 1;
    }

    {
      FILE *source;
      source = fopen(source_file, "r");
      if (source == NULL) {
        printf("\nError: the source file does not exist\n\n");
        print_usage();
        return 1;
      }
      fclose(source);
    }
  }

  PRINT0("load image...\n");
  CHECK(pixel_image_create_from_file(&src_image, source_file, p_U8, GREY));
  CHECK(pixel_image_create(&tmp_image, p_U8, RGB, src_image.width,
                           src_image.height, 3, 3 * src_image.width));
  CHECK(pixel_image_create(&dst_image, p_U8, RGB, 5 * src_image.width,
                           5 * src_image.height, 3, 3 * 5 * src_image.width));
  
  CHECK(convert_grey8_to_grey24(&src_image, &tmp_image));
  CHECK(pixel_image_replicate_pixels(&tmp_image, &dst_image, 5));
  
  PRINT0("create graph...\n");
  CHECK(graph_create_from_image(&g, &src_image, 5, 5, stepx, stepy,
                                NEIGHBORHOOD_4, POS_ATTR, VALUE_ATTR, WEIGHT_ATTR));
  
  /* run the requested algorithm on the graph */
  switch (mode) {
    case m_CONNECTED:
      PRINT0("finding connected components...\n");

      break;
    case m_MSF:
      PRINT0("finding minimum spanning forest...\n");
      {
        disjoint_set_stat_pos_attribute_params sparams;
        real threshold;
        
        /* the set attribute will have statistics dependent on the value attr */
        sparams.set_key = SET_ATTR;
        sparams.attribute_count = 4;
        sparams.stat_key = SET_STAT_ATTR;
        sparams.stat_dep_key = VALUE_ATTR;
        sparams.pos_key = SET_POS_ATTR;
        sparams.pos_dep_key = POS_ATTR;
        
        threshold = 5.1;
        
        /* add a set attribute containing statistics into each node */
        CHECK(graph_for_attrs_in_each_node(&g, 
                                           &disjoint_set_add_stat_pos_attr, 
                                           (pointer)&sparams));
        /* sort links by ascending weight using counting sort */
        /* 'remove' links by union of linked nodes meeting the criteria */
        CHECK(graph_for_each_link(&g,
                                  &union_for_smaller_than,
                                  (pointer)&threshold));
        /* clean up by eliminating too small regions */
        CHECK(graph_create(&greg, 100, 100));
        CHECK(graph_for_each_node(&g,
                                  &node_for_each_set,
                                  (pointer)&greg));
        printf("adding links between new nodes\n");
        CHECK(graph_for_each_link(&g,
                                  &link_for_neighboring_sets,
                                  (pointer)&greg));
        printf("links added\n");
      }
      break;
    case m_CONTOUR:
      PRINT0("finding contours...\n");

      break;
    default:
      PRINT0("unknown mode\n");
      ERROR(BAD_PARAM);
  }
  
  /* draw nodes on image */
  PRINT0("drawing graph...\n");
  CHECK(graph_draw_nodes(&g, &dst_image, SET_ATTR, WEIGHT_ATTR, 5));
  CHECK(graph_draw_nodes(&greg, &dst_image, 0, 0, 5));
  
  /* write the resulting image to file */
  PRINT0("writing result to file...\n");
  CHECK(pixel_image_write_to_file(&dst_image, target_file));
  PRINT0("done!\n");

  FINALLY(main);
  if (IS_FALSE(graph_is_null(&greg))) {
    graph_destroy(&greg);
  }
  PRINT0("destroy graph\n");
  graph_destroy(&g);
  PRINT0("destroyed\n");
  pixel_image_destroy(&dst_image);
  pixel_image_destroy(&tmp_image);
  pixel_image_destroy(&src_image);
  return 0;
}
