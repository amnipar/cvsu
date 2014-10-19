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

string evaluate_diff_acc_attr_name = "evaluate_diff_acc_attr";
string evaluate_idiff_pool_attr_name = "evaluate_idiff_pool_attr";
string evaluate_adiff_pool_attr_name = "evaluate_adiff_pool_attr";
string evaluate_difference_attr_name = "evaluate_difference_attr";
string add_diffusion_attrs_name = "add_diffusion_attrs";
string add_difference_attrs_name = "add_difference_attrs";
string add_diffusion_dependencies_name = "add_diffusion_dependencies";
string run_diffusion_name = "run_diffusion";
string finish_diffusion_name = "finish_diffusion";
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
const uint32 DIFF_POOL_ATTR = 9;
const uint32 DIFF_ACC_ATTR = 10;
const uint32 DIFF_DIFF_ATTR = 11;

/**
 * Evaluator for accumulator attribute used in diffusion processes.
 * Always a scalar attribute.
 * Always has a dependency to 1 scalar attribute.
 * The value is simply copied from the dependency. The accumulator is used as
 * the dependency for the difference attributes, so it is a backup of the value
 * from the previous time step.
 */
result evaluate_diff_acc_attr
(
  attribute *target,
  attribute **dependencies,
  uint32 length,
  uint32 token
)
{
  TRY();
  real *acc, pool;
  /*printf("eval acc i\n");*/
  CHECK_POINTER(target);
  CHECK_POINTER(dependencies);
  /*printf("1\n");*/
  CHECK_PARAM(length == 1);
  /*printf("2\n");*/
  CHECK_PARAM(dependencies[0]->value.type == t_real);
  /*printf("3\n");*/
  acc = IS_TYPE((&target->value), real);
  CHECK_PARAM(acc != NULL);
  /*printf("4\n");*/
  /* ensure the dependencies are up to date */
  /* the pooled value must be from the same time step */
  CHECK(attribute_update(dependencies[0], token));
  /*printf("5\n");*/
  pool = attribute_to_real(dependencies[0]);
  
  *acc = pool;
  /*printf("eval acc o\n");*/
  FINALLY(evaluate_diff_acc_attr);
  RETURN();
}

/**
 * Evaluator for pool attribute used in isotropic diffusion.
 * Always a scalar attribute.
 * Always has a dependency to 5 scalar attributes.
 * The value is a finite difference approximation for one timestep of an
 * isotropic diffusion process.
 */
result evaluate_idiff_pool_attr
(
  attribute *target,
  attribute **dependencies,
  uint32 length,
  uint32 token
)
{
  TRY();
  real *pool, c, w, n, e, s;
  /*printf("eval pool i\n");*/
  CHECK_POINTER(target);
  CHECK_POINTER(dependencies);
  /*printf("1\n");*/
  CHECK_PARAM(length == 5);
  /*printf("2\n");*/
  CHECK_PARAM(dependencies[0]->value.type == t_real);
  /*printf("3\n");*/
  pool = IS_TYPE((&target->value), real);
  CHECK_PARAM(pool != NULL);
  /*printf("4\n");*/
  /* ensure the dependencies are up to date */
  /* the pooled value and differences must be from the *previous* time step */
  CHECK(attribute_update(dependencies[0], token - 1));
  /*printf("5\n");*/
  c = attribute_to_real(dependencies[0]);
  
  if (dependencies[1] == NULL) {
    w = 0;
  }
  else {
    /*printf("5.1 %d %d\n", t_real, dependencies[1]->value.type);*/
    CHECK_PARAM(dependencies[1]->value.type == t_real);
    /*printf("5.2\n");*/
    CHECK(attribute_update(dependencies[1], token - 1));
    w = attribute_to_real(dependencies[1]);
  }
  /*printf("6\n");*/
  if (dependencies[2] == NULL) {
    n = 0;
  }
  else {
    CHECK_PARAM(dependencies[2]->value.type == t_real);
    CHECK(attribute_update(dependencies[2], token - 1));
    n = attribute_to_real(dependencies[2]);
  }
  /*printf("7\n");*/
  if (dependencies[3] == NULL) {
    e = 0;
  }
  else {
    CHECK_PARAM(dependencies[3]->value.type == t_real);
    CHECK(attribute_update(dependencies[3], token - 1));
    e = -attribute_to_real(dependencies[3]);
  }
  /*printf("8\n");*/
  if (dependencies[4] == NULL) {
    s = 0;
  }
  else {
    CHECK_PARAM(dependencies[4]->value.type == t_real);
    CHECK(attribute_update(dependencies[4], token - 1));
    s = -attribute_to_real(dependencies[4]);
  }
  /*printf("9\n");*/
  *pool = c + 0.25 * (w + n + e + s);
  
  /*printf("eval pool o\n");*/
  FINALLY(evaluate_idiff_pool_attr);
  RETURN();
}

/* constant describing the effect of noise in the gradient */
/* could be calculated by taking e.g. the 90% point of diff histogram */
const real K = 4.0;

real gtoc(real g)
{
  return 1.0 / (1.0 + pow(fabs(g) / K, 2.0));
}

/**
 * Evaluator for pool attribute used in anisotropic diffusion.
 * Always a scalar attribute.
 * Always has a dependency to 5 scalar attributes.
 * The value is a finite difference approximation for one timestep of an
 * anisotropic diffusion process.
 */
result evaluate_adiff_pool_attr
(
  attribute *target,
  attribute **dependencies,
  uint32 length,
  uint32 token
)
{
  TRY();
  real *pool, c, dw, dn, de, ds, cw, cn, ce, cs;
  
  CHECK_POINTER(target);
  CHECK_POINTER(dependencies);
  CHECK_PARAM(length == 5);
  CHECK_PARAM(dependencies[0]->value.type == t_real);
  
  pool = IS_TYPE((&target->value), real);
  CHECK_PARAM(pool != NULL);
  
  /* ensure the dependencies are up to date */
  /* the pooled value and differences must be from the *previous* time step */
  CHECK(attribute_update(dependencies[0], token - 1));
  c = attribute_to_real(dependencies[0]);
  
  if (dependencies[1] == NULL) {
    dw = 0;
  }
  else {
    CHECK_PARAM(dependencies[1]->value.type == t_real);
    CHECK(attribute_update(dependencies[1], token - 1));
    dw = attribute_to_real(dependencies[1]);
  }
  if (dependencies[2] == NULL) {
    dn = 0;
  }
  else {
    CHECK_PARAM(dependencies[2]->value.type == t_real);
    CHECK(attribute_update(dependencies[2], token - 1));
    dn = attribute_to_real(dependencies[2]);
  }
  if (dependencies[3] == NULL) {
    de = 0;
  }
  else {
    CHECK_PARAM(dependencies[3]->value.type == t_real);
    CHECK(attribute_update(dependencies[3], token - 1));
    de = -attribute_to_real(dependencies[3]);
  }
  if (dependencies[4] == NULL) {
    ds = 0;
  }
  else {
    CHECK_PARAM(dependencies[4]->value.type == t_real);
    CHECK(attribute_update(dependencies[4], token - 1));
    ds = -attribute_to_real(dependencies[4]);
  }
  
  /* calculate the dynamic conduction coefficients */
  cw = gtoc(dw);
  cn = gtoc(dn);
  ce = gtoc(de);
  cs = gtoc(ds);
  
  *pool = c + 0.25 * (cw * dw + cn * dn + ce * de + cs * ds);
  
  FINALLY(evaluate_adiff_pool_attr);
  RETURN();
}

/**
 * Evaluator for node difference attribute used in anisotropic diffusion.
 * Always a scalar attribute.
 * Always has a dependency to 2 scalar attributes.
 * The value is simply the first value minus the second value.
 */
result evaluate_difference_attr
(
  attribute *target,
  attribute **dependencies,
  uint32 length,
  uint32 token
)
{
  TRY();
  real a, b, *c;
  
  CHECK_POINTER(target);
  CHECK_POINTER(dependencies);
  CHECK_PARAM(length == 2);
  CHECK_PARAM(dependencies[0]->value.type == t_real);
  CHECK_PARAM(dependencies[1]->value.type == t_real);
  c = IS_TYPE((&target->value), real);
  CHECK_PARAM(c != NULL);
  
  /* ensure the dependencies are up to date*/
  CHECK(attribute_update(dependencies[0], token));
  CHECK(attribute_update(dependencies[1], token));
  
  a = attribute_to_real(dependencies[0]);
  b = attribute_to_real(dependencies[1]);
  
  /* b should be the central node */
  *c = a - b;
  
  FINALLY(evaluate_difference_attr);
  RETURN();
}

result add_diffusion_attrs
(
  node *target,
  pointer params
)
{
  TRY();
  attribute *pool_attr, *acc_attr;
  pixel_value *value_attr;
  real *value;
  
  CHECK_POINTER(target);
  /*CHECK_POINTER(params);*/
  (void)params;
  /*
  CHECK(scalar_attribute_add(&target->attributes, DIFF_ACC_ATTR, 0, &value));
  */
  CHECK(attribute_list_add_new(&target->attributes, DIFF_ACC_ATTR, t_real,
                               &acc_attr));
  acc_attr->value.token = 0;
  /*
  CHECK(scalar_attribute_add(&target->attributes, DIFF_POOL_ATTR, 0, NULL));
  */
  CHECK(attribute_list_add_new(&target->attributes, DIFF_POOL_ATTR, t_real,
                               &pool_attr));
  
  value = (real*)pool_attr->value.value;
  value_attr = pixel_value_attribute_get(&target->attributes, VALUE_ATTR);
  if (value_attr != NULL) {
    *value = value_attr->cache;
  }
  pool_attr->value.token = 1;
  
  CHECK(attribute_add_dependencies(acc_attr, 1, &evaluate_diff_acc_attr));
  acc_attr->dependencies->attributes[0] = pool_attr;
  
  CHECK(attribute_update(acc_attr, 1));
  
  FINALLY(add_diffusion_attrs);
  RETURN();
}

result add_difference_attrs
(
  link *target,
  pointer params
)
{
  TRY();
  attribute *diff_attr, *a_attr, *b_attr;
  
  CHECK_POINTER(target);
  (void)params;
  
  a_attr = attribute_find(&target->a.origin->attributes, DIFF_ACC_ATTR);
  b_attr = attribute_find(&target->b.origin->attributes, DIFF_ACC_ATTR);
  CHECK_PARAM(a_attr != NULL && b_attr != NULL);
  
  CHECK(attribute_list_add_new(&target->attributes, DIFF_DIFF_ATTR, t_real,
                               &diff_attr));
  diff_attr->value.token = 0;
  
  CHECK(attribute_add_dependencies(diff_attr, 2, &evaluate_difference_attr));
  diff_attr->dependencies->attributes[0] = a_attr;
  diff_attr->dependencies->attributes[1] = b_attr;
  
  CHECK(attribute_update(diff_attr, 1));
  
  FINALLY(add_difference_attrs);
  RETURN();
}

result add_diffusion_dependencies
(
  node *target,
  pointer params
)
{
  TRY();
  attribute *pool_attr, *c_attr, *w_attr, *n_attr, *e_attr, *s_attr;
  attribute_evaluator eval;
  
  CHECK_POINTER(target);
  CHECK_POINTER(params);
  CHECK_PARAM(target->links.count >= 4);
  
  eval = (attribute_evaluator)params;
  
  pool_attr = attribute_find(&target->attributes, DIFF_POOL_ATTR);
  CHECK_PARAM(pool_attr != NULL);
  
  CHECK(attribute_add_dependencies(pool_attr, 5, eval));
  
  c_attr = attribute_find(&target->attributes, DIFF_ACC_ATTR);
  CHECK_PARAM(c_attr != NULL);
  
  w_attr = NULL;
  if (target->links.items[3] != NULL) {
    w_attr = attribute_find(&target->links.items[3]->body->attributes,
                            DIFF_DIFF_ATTR);
    CHECK_PARAM(w_attr->value.type == t_real);
    CHECK_PARAM(w_attr != NULL);
  }
  n_attr = NULL;
  if (target->links.items[0] != NULL) {
    n_attr = attribute_find(&target->links.items[0]->body->attributes,
                            DIFF_DIFF_ATTR);
    CHECK_PARAM(n_attr->value.type == t_real);
    CHECK_PARAM(n_attr != NULL);
  }
  e_attr = NULL;
  if (target->links.items[1] != NULL) {
    e_attr = attribute_find(&target->links.items[1]->body->attributes,
                            DIFF_DIFF_ATTR);
    CHECK_PARAM(e_attr->value.type == t_real);
    CHECK_PARAM(e_attr != NULL);
  }
  s_attr = NULL;
  if (target->links.items[2] != NULL) {
    s_attr = attribute_find(&target->links.items[2]->body->attributes,
                            DIFF_DIFF_ATTR);
    CHECK_PARAM(s_attr->value.type == t_real);
    CHECK_PARAM(s_attr != NULL);
  }
  
  pool_attr->dependencies->attributes[0] = c_attr;
  pool_attr->dependencies->attributes[1] = w_attr;
  pool_attr->dependencies->attributes[2] = n_attr;
  pool_attr->dependencies->attributes[3] = e_attr;
  pool_attr->dependencies->attributes[4] = s_attr;
  
  FINALLY(add_diffusion_dependencies);
  RETURN();
}

result run_diffusion
(
  node *target,
  pointer params
)
{
  TRY();
  attribute *pool_attr;
  uint32 token;
  
  CHECK_POINTER(target);
  CHECK_POINTER(params);
  
  token = *(uint32*)params;
  
  pool_attr = attribute_find(&target->attributes, DIFF_POOL_ATTR);
  /*printf("update attribute b\n");*/
  CHECK(attribute_update(pool_attr, token));
  /*printf("update attribute a\n");*/
  
  FINALLY(run_diffusion);
  RETURN();
}

result finish_diffusion
(
  node *target,
  pointer params
)
{
  TRY();
  attribute *acc_attr;
  uint32 token;
  
  CHECK_POINTER(target);
  CHECK_POINTER(params);
  
  token = *(uint32*)params;
  
  acc_attr = attribute_find(&target->attributes, DIFF_ACC_ATTR);
  CHECK(attribute_update(acc_attr, token));

  FINALLY(finish_diffusion);
  RETURN();
}

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
      CHECK(graph_add_node(g, 4, 1000, &node_ptr));
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
  PRINT0("graph mode stepx stepy viz source target\n");
  PRINT0("  mode: analysis mode [ connected | msf | contour ]\n");
  PRINT0("  dx: horizontal offset for the first column of nodes\n");
  PRINT0("  dy: vertical offset for the first row of nodes\n");
  PRINT0("  stepx: horizontal step in pixels between nodes, >= 1\n");
  PRINT0("  stepy: vertical step in pixels between nodes, >= 1\n");
  PRINT0("  scale: scaling factor for the output image, >= 1\n");
  PRINT0("  viz: visualization mode [nodes | pixels]\n");
  PRINT0("  source: source image file to process\n");
  PRINT0("  target: target image file to generate\n\n");
}

enum mode_t {
  m_UNDEF = 0,
  m_IDIFFUSE,
  m_ADIFFUSE,
  m_CONNECTED,
  m_MSF,
  m_CONTOUR
};

enum viz_t {
  v_UNDEF = 0,
  v_NODES,
  v_PIXELS
};

int main(int argc, char *argv[])
{
  TRY();
  pixel_image src_image, tmp_image, dst_image;
  graph g, greg;
  uint32 dx, dy, stepx, stepy, scale;
  string smode, vmode, source_file, target_file;
  enum mode_t mode;
  enum viz_t viz;
  
  if (argc < 10) {
    PRINT0("\nError: wrong number of parameters\n\n");
    print_usage();
    return 1;
  }
  else {
    int scan_result;

    mode = m_UNDEF;
    smode = argv[1];
    if (strcmp(smode, "idiffuse") == 0) {
      mode = m_IDIFFUSE;
    }
    else
    if (strcmp(smode, "adiffuse") == 0) {
      mode = m_ADIFFUSE;
    }
    else
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

    scan_result = sscanf(argv[2], "%lu", &dx);
    if (scan_result != 1) {
      PRINT0("\nError: failed to parse parameter dx\n\n");
      print_usage();
      return 1;
    }
    
    scan_result = sscanf(argv[3], "%lu", &dy);
    if (scan_result != 1) {
      PRINT0("\nError: failed to parse parameter dy\n\n");
      print_usage();
      return 1;
    }
    
    scan_result = sscanf(argv[4], "%lu", &stepx);
    if (scan_result != 1) {
      PRINT0("\nError: failed to parse parameter stepx\n\n");
      print_usage();
      return 1;
    }
    
    scan_result = sscanf(argv[5], "%lu", &stepy);
    if (scan_result != 1) {
      PRINT0("\nError: failed to parse parameter stepy\n\n");
      print_usage();
      return 1;
    }
    
    scan_result = sscanf(argv[6], "%lu", &scale);
    if (scan_result != 1) {
      PRINT0("\nError: failed to parse parameter scale\n\n");
      print_usage();
      return 1;
    }
    
    viz = v_UNDEF;
    vmode = argv[7];
    if (strcmp(vmode, "nodes") == 0) {
      viz = v_NODES;
    }
    else
    if (strcmp(vmode, "pixels") == 0) {
      viz = v_PIXELS;
    }
    else {
      PRINT1("\nError: unsupported viz mode (%s)\n\n", vmode);
      print_usage();
      return 1;
    }
    
    source_file = argv[8];
    target_file = argv[9];
    
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
    if (scale < 1) {
      PRINT0("\nError: scale may not be smaller than 1\n\n");
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
  CHECK(pixel_image_create(&dst_image, p_U8, RGB, scale * src_image.width,
                           scale * src_image.height, 3, 
                           3 * scale * src_image.width));
  
  CHECK(convert_grey8_to_grey24(&src_image, &tmp_image));
  CHECK(pixel_image_replicate_pixels(&tmp_image, &dst_image, scale));
  
  PRINT0("create graph...\n");
  CHECK(graph_create_from_image(&g, &src_image, dx, dy, stepx, stepy,
                                NEIGHBORHOOD_4, POS_ATTR, VALUE_ATTR, WEIGHT_ATTR));
  /* this graph is not always created, need to make it null */
  /* otherwise there are problems when trying to destroy it */
  graph_nullify(&greg);
  
  /* run the requested algorithm on the graph */
  switch (mode) {
    case m_IDIFFUSE:
      PRINT0("running isotropic diffusion...\n");
      {
        uint32 i, diffusion_rounds;
        
        graph_for_each_node(&g,
                            &add_diffusion_attrs,
                            NULL);
        
        graph_for_each_link(&g,
                            &add_difference_attrs,
                            NULL);
        graph_for_each_node(&g,
                            &add_diffusion_dependencies,
                            &evaluate_idiff_pool_attr);
        
        diffusion_rounds = 9;
        for (i = 2; i < diffusion_rounds; i++) {
          graph_for_each_node(&g,
                              &run_diffusion,
                              &i);
        }
        /* acc attributes need to be updated one more time */
        i--;
        graph_for_each_node(&g,
                            &finish_diffusion,
                            &i);
      }
      break;
    case m_ADIFFUSE:
      PRINT0("running anisotropic diffusion...\n");
      {
        uint32 i, diffusion_rounds;
        
        graph_for_each_node(&g,
                            &add_diffusion_attrs,
                            NULL);
        
        graph_for_each_link(&g,
                            &add_difference_attrs,
                            NULL);
        /* the only difference to isotropic diffusion is the function for */
        /* evaluating the pool attributes */
        graph_for_each_node(&g,
                            &add_diffusion_dependencies,
                            &evaluate_adiff_pool_attr);
        
        diffusion_rounds = 9;
        for (i = 2; i < diffusion_rounds; i++) {
          graph_for_each_node(&g,
                              &run_diffusion,
                              &i);
        }
        /* acc attributes need to be updated one more time */
        i--;
        graph_for_each_node(&g,
                            &finish_diffusion,
                            &i);
      }
      break;
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
        CHECK(graph_create(&greg, 1000, 1000));
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
  if (viz == v_NODES) {
    CHECK(graph_draw_nodes(&g, &dst_image, SET_ATTR, WEIGHT_ATTR, scale));
    CHECK(graph_draw_nodes(&greg, &dst_image, 0, 0, (real)scale));
  }
  else {
    CHECK(graph_draw_pixels(&g, &dst_image, DIFF_ACC_ATTR,
                            (real)scale, stepx, stepy));
    /*CHECK(graph_draw_nodes(&greg, &dst_image, 0, 0, (real)scale));*/
  }
  
  /* write the resulting image to file */
  PRINT0("writing result to file...\n");
  CHECK(pixel_image_write_to_file(&dst_image, target_file));
  PRINT0("done!\n");

  FINALLY(main);
  if (IS_FALSE(graph_is_null(&greg))) {
    printf("d b\n");
    graph_destroy(&greg);
    printf("d a\n");
  }
  PRINT0("destroy graph\n");
  graph_destroy(&g);
  PRINT0("destroyed\n");
  pixel_image_destroy(&dst_image);
  pixel_image_destroy(&tmp_image);
  pixel_image_destroy(&src_image);
  return 0;
}
