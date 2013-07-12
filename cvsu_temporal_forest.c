/**
 * @file cvsu_temporal_forest.c
 * @author Matti J. Eskelinen <matti.j.eskelinen@gmail.com>
 * @brief Temporal Forest structure for analyzing image changes over time.
 *
 * Copyright (c) 2013, Matti Johannes Eskelinen
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

#include "cvsu_temporal_forest.h"
#include "cvsu_macros.h"
#include "cvsu_memory.h"
#include "cvsu_parsing.h"
#include <math.h>

/******************************************************************************/
/* constants for reporting function names in error messages                   */

string temporal_forest_alloc_name = "temporal_forest_alloc";
string temporal_forest_free_name = "temporal_forest_free";
string temporal_forest_create_name = "temporal_forest_create";
string temporal_forest_destroy_name = "temporal_forest_destroy";
string temporal_forest_nullify_name = "temporal_forest_nullify";
string temporal_forest_update_name = "temporal_forest_update";
string temporal_forest_visualize_name = "temporal_forest_visualize";
string temporal_forest_get_segments_name = "temporal_forest_get_segments";
string temporal_forest_get_segment_boundary_name = "temporal_forest_get_segment_boundary";

/******************************************************************************/

temporal_forest *temporal_forest_alloc()
{
  TRY();
  temporal_forest *forest;

  CHECK(memory_allocate((data_pointer*)&forest, 1, sizeof(temporal_forest)));
  CHECK(temporal_forest_nullify(forest));

  FINALLY(temporal_forest_alloc);
  return forest;
}

/******************************************************************************/

void temporal_forest_free
(
  temporal_forest *forest
)
{
  TRY();

  r = SUCCESS;
  if (forest != NULL) {
    CHECK(temporal_forest_destroy(forest));
    CHECK(memory_deallocate((data_pointer*)&forest));
  }

  FINALLY(temporal_forest_free);
}

/******************************************************************************/

result temporal_forest_create
(
  temporal_forest *target,
  pixel_image *source,
  uint32 max_size,
  uint32 min_size,
  uint32 frame_count,
  uint32 history_count
)
{
  TRY();
  uint32 i;

  CHECK_POINTER(target);
  CHECK_POINTER(source);
  CHECK_TRUE(temporal_forest_is_null(target));
  CHECK_PARAM(min_size > 0);
  CHECK_PARAM(max_size >= min_size);
  CHECK_PARAM(frame_count > 0);

  CHECK(memory_allocate((data_pointer*)&target->forests, frame_count, sizeof(quad_forest)));
  for (i = 0; i < frame_count; i++) {
    CHECK(quad_forest_create(&target->forests[i], source, max_size, min_size));
  }
  target->count = frame_count;
  target->current = 0;
  target->frames = 0;
  CHECK(pixel_image_create(&target->visual, p_U8, RGB, source->width, source->height, 3, 3 * source->stride));
  target->tree_max_size = max_size;
  target->tree_min_size = min_size;
  target->rows = target->forests[0].rows;
  target->cols = target->forests[0].cols;
  target->dx = target->forests[0].dx;
  target->dy = target->forests[0].dy;
  if (history_count > 0) {
    /* create background forest target->background */
  }

  FINALLY(temporal_forest_create);
  RETURN();
}

/******************************************************************************/

result temporal_forest_destroy
(
  temporal_forest *target
)
{
  TRY();
  uint32 i;

  for (i = 0; i < target->count; i++) {
      CHECK(quad_forest_destroy(&target->forests[i]));
  }
  CHECK(memory_deallocate((data_pointer*)&target->forests));
  CHECK(pixel_image_destroy(&target->visual));
  CHECK(temporal_forest_nullify(target));

  FINALLY(temporal_forest_destroy);
  RETURN();
}

/******************************************************************************/

result temporal_forest_nullify
(
  temporal_forest *target
)
{
  TRY();

  CHECK_POINTER(target);

  CHECK(pixel_image_nullify(&target->visual));
  target->background = NULL;
  target->forests = NULL;
  target->rows = 0;
  target->cols = 0;
  target->tree_max_size = 0;
  target->tree_min_size = 0;
  target->dx = 0;
  target->dy = 0;
  target->count = 0;
  target->current = 0;
  target->frames = 0;

  FINALLY(temporal_forest_nullify);
  RETURN();
}

/******************************************************************************/

truth_value temporal_forest_is_null
(
  temporal_forest *target
)
{
  if (target != NULL) {
    if (target->forests == NULL) {
      return TRUE;
    }
  }
  return FALSE;
}

/******************************************************************************/

result temporal_forest_update
(
  temporal_forest *target,
  pixel_image *source
)
{
  TRY();
  uint32 i, size;
  integral_value mean1, mean2, dev1, dev2, dev;
  quad_forest *forest1, *forest2;
  quad_tree *tree1, *tree2;
  segment *parent, *neighbor_parent;

  CHECK_POINTER(target);
  CHECK_POINTER(source);

  target->current++;
  if (target->current >= target->count) {
    target->current = 0;
  }
  CHECK(pixel_image_copy(target->forests[target->current].source, source));
  target->frames++;
  target->forests[target->current].token = target->frames;
  CHECK(quad_forest_update(&target->forests[target->current]));
  if (target->frames > 1) {
    if (target->current == 0) {
      forest1 = &target->forests[target->count - 1];
    }
    else {
      forest1 = &target->forests[target->current - 1];
    }
    forest2 = &target->forests[target->current];

    /*forest2->token = target->frames;*/
    /*CHECK(quad_forest_calculate_neighborhood_stats(forest2, TRUE, 2, TRUE, FALSE, TRUE));*/
    /*CHECK(quad_forest_calculate_accumulated_regs(forest2, 5));*/
    CHECK(quad_forest_parse(forest2, 1, FALSE));
    /*
    size = target->rows * target->cols;
    for (i = 0; i < size; i++) {
      tree1 = forest1->roots[i];
      tree2 = forest2->roots[i];
      mean1 = tree1->stat.mean;
      mean2 = tree2->stat.mean;
      dev1 = getmax(1, tree1->stat.deviation);
      dev2 = getmax(1, tree2->stat.deviation);
      dev = dev1 + dev2;
      if (fabs(mean1 - mean2) > dev) {
        quad_tree_segment_create(tree2);
      }
      else {
        tree2->segment.parent = NULL;
      }
    }
    for (i = 0; i < size; i++) {
      tree1 = forest2->roots[i];
      parent = quad_tree_segment_find(tree1);
      if (parent == &tree1->segment) {
        tree2 = tree1->n;
        if (tree2 != NULL) {
          neighbor_parent = quad_tree_segment_find(tree2);
          if (neighbor_parent == &tree2->segment) {
            quad_tree_segment_union(tree1, tree2);
          }
          else {
            mean1 = tree1->stat.mean;
            mean2 = tree2->stat.mean;
            dev1 = getmax(1, tree1->stat.deviation);
            dev2 = getmax(1, tree2->stat.deviation);
            dev = dev1 + dev2;
            if (fabs(mean1 - mean2) < dev) {
              quad_tree_segment_create(tree2);
              quad_tree_segment_union(tree1, tree2);
            }
          }
        }
        tree2 = tree1->e;
        if (tree2 != NULL) {
          neighbor_parent = quad_tree_segment_find(tree2);
          if (neighbor_parent == &tree2->segment) {
            quad_tree_segment_union(tree1, tree2);
          }
          else {
            mean1 = tree1->stat.mean;
            mean2 = tree2->stat.mean;
            dev1 = getmax(1, tree1->stat.deviation);
            dev2 = getmax(1, tree2->stat.deviation);
            dev = dev1 + dev2;
            if (fabs(mean1 - mean2) < dev) {
              quad_tree_segment_create(tree2);
              quad_tree_segment_union(tree1, tree2);
            }
          }
        }
        tree2 = tree1->s;
        if (tree2 != NULL) {
          neighbor_parent = quad_tree_segment_find(tree2);
          if (neighbor_parent == &tree2->segment) {
            quad_tree_segment_union(tree1, tree2);
          }
          else {
            mean1 = tree1->stat.mean;
            mean2 = tree2->stat.mean;
            dev1 = getmax(1, tree1->stat.deviation);
            dev2 = getmax(1, tree2->stat.deviation);
            dev = dev1 + dev2;
            if (fabs(mean1 - mean2) < dev) {
              quad_tree_segment_create(tree2);
              quad_tree_segment_union(tree1, tree2);
            }
          }
        }
        tree2 = tree1->w;
        if (tree2 != NULL) {
          neighbor_parent = quad_tree_segment_find(tree2);
          if (neighbor_parent == &tree2->segment) {
            quad_tree_segment_union(tree1, tree2);
          }
          else {
            mean1 = tree1->stat.mean;
            mean2 = tree2->stat.mean;
            dev1 = getmax(1, tree1->stat.deviation);
            dev2 = getmax(1, tree2->stat.deviation);
            dev = dev1 + dev2;
            if (fabs(mean1 - mean2) < dev) {
              quad_tree_segment_create(tree2);
              quad_tree_segment_union(tree1, tree2);
            }
          }
        }
      }
    }
    for (i = 0; i < size; i++) {
      tree1 = forest2->roots[i];
      parent = quad_tree_segment_find(tree1);
      if (parent != NULL) {
        tree2 = tree1->n;
        if (tree2 != NULL) {
          neighbor_parent = quad_tree_segment_find(tree2);
          if (neighbor_parent != NULL && parent != neighbor_parent) {
            quad_tree_segment_union(tree1, tree2);
          }
        }
        tree2 = tree1->e;
        if (tree2 != NULL) {
          neighbor_parent = quad_tree_segment_find(tree2);
          if (neighbor_parent != NULL && parent != neighbor_parent) {
            quad_tree_segment_union(tree1, tree2);
          }
        }
        tree2 = tree1->s;
        if (tree2 != NULL) {
          neighbor_parent = quad_tree_segment_find(tree2);
          if (neighbor_parent != NULL && parent != neighbor_parent) {
            quad_tree_segment_union(tree1, tree2);
          }
        }
        tree2 = tree1->w;
        if (tree2 != NULL) {
          neighbor_parent = quad_tree_segment_find(tree2);
          if (neighbor_parent != NULL && parent != neighbor_parent) {
            quad_tree_segment_union(tree1, tree2);
          }
        }
      }
    }
    CHECK(quad_forest_refresh_segments(forest2));
    */

  }

  FINALLY(temporal_forest_update);
  RETURN();
}

/******************************************************************************/

result temporal_forest_visualize
(
  temporal_forest *target,
  pixel_image *image
)
{
  TRY();
  list_item *trees, *end;
  quad_forest *forest;
  quad_tree *tree;
  segment *parent;
  uint32 x, y, width, height, stride, row_step;
  byte *target_data, *target_pos, color0, color1, color2;
  list lines;

  CHECK_POINTER(target);

  forest = &target->forests[target->current];
  width = target->visual.width;
  height = target->visual.height;

  if (image != NULL) {
    CHECK_PARAM(image->width == width);
    CHECK_PARAM(image->height == height);
    CHECK_PARAM(image->type == p_U8);
    CHECK_PARAM(image->format == RGB);

    stride = image->stride;
    target_data = (byte*)image->data;
  }
  else {
    stride = target->visual.stride;
    target_data = (byte*)target->visual.data;
  }

  /*
  CHECK(pixel_image_clear(&target->visual));
  CHECK(convert_grey8_to_grey24(forest->source, &target->visual));
  */

  CHECK(list_create(&lines, 1000, sizeof(colored_line), 1));

  /*CHECK(quad_forest_visualize_accumulated_regs(forest, &target->visual));*/
  if (image != NULL) {
    CHECK(quad_forest_visualize_parse_result(forest, image));
    /*CHECK(quad_forest_visualize_neighborhood_stats(forest, image, v_SCORE));*/
    /*CHECK(quad_forest_get_links(forest, &lines, v_LINK_MEASURE));*/
    /*
    CHECK(quad_forest_get_links(forest, &lines, v_LINK_EDGE));
    CHECK(pixel_image_draw_colored_lines(image, &lines, 1));
    */
  }
  else {
    CHECK(quad_forest_visualize_parse_result(forest, &target->visual));
    /*CHECK(quad_forest_visualize_neighborhood_stats(forest, &target->visual, v_SCORE));*/
    /*CHECK(quad_forest_get_links(forest, &lines, v_LINK_MEASURE));*/
    /*
    CHECK(quad_forest_get_links(forest, &lines, v_LINK_EDGE));
    CHECK(pixel_image_draw_colored_lines(&target->visual, &lines, 2));
    */
  }
  /*
  CHECK(list_create(&lines, 1000, sizeof(line), 1));
  trees = forest->trees.first.next;
  end = &forest->trees.last;
  while (trees != end) {
    tree = (quad_tree *)trees->data;
    if (tree->nw == NULL) {
      CHECK(quad_tree_edge_response_to_line(tree, &lines));
    }
    trees = trees->next;
  }
  CHECK(pixel_image_draw_lines(&target->visual, &lines));
  */
  /*
  CHECK(pixel_image_clear(&target->visual));

  trees = forest->trees.first.next;
  end = &forest->trees.last;
  while (trees != end) {
    tree = (quad_tree *)trees->data;
    if (tree->nw == NULL) {
      parent = quad_tree_segment_find(tree);
      if (parent != NULL) {
        color0 = parent->color[0];
        color1 = parent->color[1];
        color2 = parent->color[2];
      }
      else {
        color0 = (byte)tree->stat.mean;
        color1 = color0;
        color2 = color0;
      }
      width = tree->size;
      height = width;
      row_step = stride - 3 * width;
      target_pos = target_data + tree->y * stride + tree->x * 3;
      for (y = 0; y < height; y++, target_pos += row_step) {
        for (x = 0; x < width; x++) {
          *target_pos = color0;
          target_pos++;
          *target_pos = color1;
          target_pos++;
          *target_pos = color2;
          target_pos++;
        }
      }
    }
    trees = trees->next;
  }
  */
  FINALLY(temporal_forest_visualize);
  list_destroy(&lines);
  RETURN();
}

/******************************************************************************/

quad_forest *temporal_forest_get_current
(
  temporal_forest *target
)
{
  if (target != NULL) {
    if (target->count > 0) {
      return &target->forests[target->current];
    }
  }
  return NULL;
}

/******************************************************************************/

uint32 temporal_forest_segment_count
(
  temporal_forest *target
)
{
  quad_forest *forest;
  forest = temporal_forest_get_current(target);
  if (forest != NULL) {
    return forest->segments;
  }
  return 0;
}

/******************************************************************************/

result temporal_forest_get_segments
(
  temporal_forest *forest,
  segment **segments
)
{
  TRY();

  CHECK(quad_forest_get_segments(temporal_forest_get_current(forest), segments));

  FINALLY(temporal_forest_get_segments);
  RETURN();
}

/******************************************************************************/

result temporal_forest_get_segment_boundary
(
  temporal_forest *forest,
  segment *input_segment,
  list *boundary_list
)
{
  TRY();

  CHECK(quad_forest_get_segment_boundary(temporal_forest_get_current(forest), input_segment, boundary_list));

  FINALLY(temporal_forest_get_segment_boundary);
  RETURN();
}

/* end of file                                                                */
/******************************************************************************/
