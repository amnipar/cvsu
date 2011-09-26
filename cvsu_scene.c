
result create_hierarchical_scene(hierarchical_scene *dst, pixel_image *src)
{
    result r;
    long row, col, pos;

    if (dst == NULL) {
        return BAD_POINTER;
    }

    dst->hstep = 16;
    dst->vstep = 16;
    dst->hmargin = 16;
    dst->vmargin = 16;
    dst->box_width = 16;
    dst->box_length = 8;
    dst->width = src->width;
    dst->height = src->height;
    dst->rows = (long)((dst->height - 2 * dst->vmargin) / dst->vstep);
    dst->cols = (long)((dst->width - 2 * dst->hmargin) / dst->hstep);
    dst->dx = (long)((dst->hstep - dst->box_width) / 2);
    dst->dy = (long)((dst->vstep - dst->box_width) / 2);

    r = integral_image_create(&dst->Int, src);
    if (r != SUCCESS) {
        return r;
    }
    r = memory_allocate((data_pointer *)&dst->block_array_1, dst->rows * dst->cols, sizeof(block));
    if (r != SUCCESS) {
        return r;
    }
    r = memory_allocate((data_pointer *)&dst->block_array_2, dst->rows * dst->cols, sizeof(block));
    if (r != SUCCESS) {
        return r;
    }
    r = list_create_from_data(&dst->blocks_1, (byte *)dst->block_array_1, dst->rows * dst->cols, sizeof(block), 10);
    if (r != SUCCESS) {
        return r;
    }
    r = list_create_from_data(&dst->blocks_2, (byte *)dst->block_array_2, dst->rows * dst->cols, sizeof(block), 10);
    if (r != SUCCESS) {
        return r;
    }
    /* should be done in scene_update */
    r = sublist_create(&dst->blocks_by_deviation, &dst->blocks_1);
    if (r != SUCCESS) {
        return r;
    }
    r = pointer_list_create(&dst->vedges_1, dst->rows * dst->width, sizeof(edge_elem), 10, 10);
    if (r != SUCCESS) {
        return r;
    }
    r = pointer_list_create(&dst->vedges_2, dst->rows * dst->width, sizeof(edge_elem), 10, 10);
    if (r != SUCCESS) {
        return r;
    }
    r = pointer_list_create(&dst->hedges_1, dst->height * dst->cols, sizeof(edge_elem), 10, 10);
    if (r != SUCCESS) {
        return r;
    }
    r = pointer_list_create(&dst->hedges_2, dst->height * dst->cols, sizeof(edge_elem), 10, 10);
    if (r != SUCCESS) {
        return r;
    }
    r = list_create(&dst->lines_1, 1000, sizeof(line), 10);
    if (r != SUCCESS) {
        return r;
    }
    r = list_create(&dst->lines_2, 1000, sizeof(line), 10);
    if (r != SUCCESS) {
        return r;
    }
    r = list_create(&dst->boundaries_1, 100, sizeof(boundary), 10);
    if (r != SUCCESS) {
        return r;
    }
    r = list_create(&dst->boundaries_2, 100, sizeof(boundary), 10);
    if (r != SUCCESS) {
        return r;
    }

    dst->current_block_array = dst->block_array_1;
    dst->previous_block_array = dst->block_array_2;
    dst->current_blocks = &dst->blocks_1;
    dst->previous_blocks = &dst->blocks_2;
    dst->current_vedges = &dst->vedges_1;
    dst->previous_vedges = &dst->vedges_2;
    dst->current_hedges = &dst->hedges_1;
    dst->previous_hedges = &dst->hedges_2;
    dst->current_lines = &dst->lines_1;
    dst->previous_lines = &dst->lines_2;
    dst->current_boundaries = &dst->boundaries_1;
    dst->previous_boundaries = &dst->boundaries_2;

    pos = 0;
    for (row = 0; row < dst->rows; row++) {
        for (col = 0; col < dst->cols; col++) {
            dst->block_array_1[pos].row = row;
            dst->block_array_1[pos].col = col;
            dst->block_array_1[pos].pos_x = col * dst->hstep + dst->hmargin;
            dst->block_array_1[pos].pos_y = row * dst->vstep + dst->vmargin;
            dst->block_array_1[pos].width = dst->hstep;
            dst->block_array_1[pos].height = dst->vstep;
            dst->block_array_1[pos].mean = 0;
            dst->block_array_1[pos].dev = 0;

            pointer_sublist_create(&dst->block_array_1[pos].vedge_list, &dst->vedges_1,
                    row * dst->width + dst->hmargin + col * dst->hstep, dst->hstep);
            pointer_sublist_create(&dst->block_array_1[pos].hedge_list, &dst->hedges_1,
                    (row * dst->vstep + dst->vmargin) * dst->cols + col, dst->vstep);

            // define neighbors above the block
            if (row > 0) {
                if (col > 0) {
                    dst->block_array_1[pos].neighbor_nw.b = &dst->block_array_1[pos - dst->cols - 1];
                }
                else {
                    dst->block_array_1[pos].neighbor_nw.b = NULL;
                    dst->block_array_1[pos].neighbor_nw.strength = 0;
                }
                dst->block_array_1[pos].neighbor_n.b = &dst->block_array_1[pos - dst->cols];
                if (col < dst->cols - 1) {
                    dst->block_array_1[pos].neighbor_ne.b = &dst->block_array_1[pos - dst->cols + 1];
                }
                else {
                    dst->block_array_1[pos].neighbor_ne.b = NULL;
                    dst->block_array_1[pos].neighbor_ne.strength = 0;
                }
            }
            else {
                dst->block_array_1[pos].neighbor_nw.b = NULL;
                dst->block_array_1[pos].neighbor_nw.strength = 0;
                dst->block_array_1[pos].neighbor_n.b = NULL;
                dst->block_array_1[pos].neighbor_n.strength = 0;
                dst->block_array_1[pos].neighbor_ne.b = NULL;
                dst->block_array_1[pos].neighbor_ne.strength = 0;
            }

            // define the right neighbor
            if (col < dst->cols - 1) {
                dst->block_array_1[pos].neighbor_e.b = &dst->block_array_1[pos + 1];
            }
            else {
                dst->block_array_1[pos].neighbor_e.b = NULL;
                dst->block_array_1[pos].neighbor_e.strength = 0;
            }

            // define neighbors below the block
            if (row < dst->rows - 1) {
                if (col < dst->cols - 1) {
                    dst->block_array_1[pos].neighbor_se.b = &dst->block_array_1[pos + dst->cols + 1];
                }
                else {
                    dst->block_array_1[pos].neighbor_se.b = NULL;
                    dst->block_array_1[pos].neighbor_se.strength = 0;
                }
                dst->block_array_1[pos].neighbor_s.b = &dst->block_array_1[pos + dst->cols];
                if (col > 0) {
                    dst->block_array_1[pos].neighbor_sw.b = &dst->block_array_1[pos + dst->cols - 1];
                }
                else {
                    dst->block_array_1[pos].neighbor_sw.b = NULL;
                    dst->block_array_1[pos].neighbor_sw.strength = 0;
                }
            }
            else {
                dst->block_array_1[pos].neighbor_se.b = NULL;
                dst->block_array_1[pos].neighbor_se.strength = 0;
                dst->block_array_1[pos].neighbor_s.b = NULL;
                dst->block_array_1[pos].neighbor_s.strength = 0;
                dst->block_array_1[pos].neighbor_sw.b = NULL;
                dst->block_array_1[pos].neighbor_sw.strength = 0;
            }

            // define the left neighbor
            if (col > 0) {
                dst->block_array_1[pos].neighbor_w.b = &dst->block_array_1[pos - 1];
            }
            else {
                dst->block_array_1[pos].neighbor_w.b = NULL;
                dst->block_array_1[pos].neighbor_w.strength = 0;
            }

            list_append_index(&dst->blocks_1, pos);

            pos++;
        }
    }

    return SUCCESS;
}

result destroy_hierarchical_scene(hierarchical_scene *dst)
{
    result r;

    if (dst == NULL) {
        return BAD_POINTER;
    }
    r = integral_image_destroy(&dst->Int);
    if (r != SUCCESS) {
        return r;
    }
    r = list_destroy(&dst->boundaries_1);
    if (r != SUCCESS) {
        return r;
    }
    r = list_destroy(&dst->boundaries_2);
    if (r != SUCCESS) {
        return r;
    }
    r = list_destroy(&dst->lines_1);
    if (r != SUCCESS) {
        return r;
    }
    r = list_destroy(&dst->lines_2);
    if (r != SUCCESS) {
        return r;
    }
    r = list_destroy(&dst->blocks_1);
    if (r != SUCCESS) {
        return r;
    }
    r = list_destroy(&dst->blocks_2);
    if (r != SUCCESS) {
        return r;
    }
    r = pointer_list_destroy(&dst->vedges_1);
    if (r != SUCCESS) {
        return r;
    }
    r = pointer_list_destroy(&dst->vedges_2);
    if (r != SUCCESS) {
        return r;
    }
    r = pointer_list_destroy(&dst->hedges_1);
    if (r != SUCCESS) {
        return r;
    }
    r = pointer_list_destroy(&dst->hedges_2);
    if (r != SUCCESS) {
        return r;
    }

    return SUCCESS;
}

long check_neighbor(block *current, neighbor_relation *relation)
{
    block *neighbor;
    double diff, dev, dist;
    neighbor = relation->b;
    if (neighbor != NULL) {
        diff = current->fdev - neighbor->fdev;
        if (diff < 0) diff = -diff;
        dev = current->fdev + neighbor->fdev;
        if (dev < 1) dev = 1;
        dist = ((double)diff / dev);
        relation->strength = (long)(255 * dist);
        return relation->strength;
    }
    else {
        return 0;
    }
}

result update_hierarchical_scene(hierarchical_scene *dst)
{
    result r;
    char *edge_data;
    long *integral_data;
    double *integral2_data;
    long i, width, height, margin, pos, dx, dy, sum, delta, match, strength;
    long row, rows, rowInc, col, cols, colInc, row_height, col_width;
    short ymean;
    double N, sumsqr, mean, var;
    short value1, value2, diff1, diff2;
    const long *iA, *iB, *iC, *iD;
    const double *i2A, *i2B, *i2C, *i2D;

    block *current_block;
    line new_line, *l;
    boundary new_boundary, *b;
    list_item *lines, *boundaries, *temp, *block_item;

    if (dst == NULL) {
        return BAD_POINTER;
    }

    r = integral_image_update(&dst->Int);
    if (r != SUCCESS) {
        return r;
    }

    rowInc = dst->vstep * dst->width;
    colInc = dst->hstep;
    N = (double)dst->hstep * dst->vstep;
    integral_data = (long *)dst->Int.I_1.data;
    integral2_data = (double *)dst->Int.I_2.data;

    if (integral_data == NULL || integral2_data == NULL) {
        return BAD_POINTER;
    }

    list_clear(&dst->blocks_by_deviation);
    pos = 0;
    for (row = 0; row < dst->rows; row++) {
        iA = integral_data + (row * dst->vstep + dst->vmargin - 1) * dst->width + dst->hmargin - 1;
        iB = iA + colInc;
        iC = iB + rowInc;
        iD = iA + rowInc;

        i2A = integral2_data + (row * dst->vstep + dst->vmargin - 1) * dst->width + dst->hmargin - 1;
        i2B = i2A + colInc;
        i2C = i2B + rowInc;
        i2D = i2A + rowInc;

        for (col = 0; col < dst->cols; col++) {
            sum = *iC - *iB - *iD + *iA;
            sumsqr = *i2C - *i2B - *i2D + *i2A;
            mean = sum / N;
            var = (sumsqr / N) - (mean * mean);
            if (var < 0) var = 0;

            dst->current_block_array[pos].mean = (short)mean;
            dst->current_block_array[pos].fdev = var;
            if (var < 1) var = 1;
            dst->current_block_array[pos].dev = (short)sqrt(var);

            iA += colInc;
            iB += colInc;
            iC += colInc;
            iD += colInc;
            i2A += colInc;
            i2B += colInc;
            i2C += colInc;
            i2D += colInc;

            list_insert_sorted_index(&dst->blocks_by_deviation, pos, &compare_blocks_by_deviation);

            pos++;
        }
    }

    block_item = dst->blocks_by_deviation.first.next;
    while (block_item != &dst->blocks_by_deviation.last) {
        current_block = (block *)block_item->data;
        strength = 0;
        strength += check_neighbor(current_block, &current_block->neighbor_nw);
        strength += check_neighbor(current_block, &current_block->neighbor_n);
        strength += check_neighbor(current_block, &current_block->neighbor_ne);
        strength += check_neighbor(current_block, &current_block->neighbor_e);
        strength += check_neighbor(current_block, &current_block->neighbor_se);
        strength += check_neighbor(current_block, &current_block->neighbor_s);
        strength += check_neighbor(current_block, &current_block->neighbor_sw);
        strength += check_neighbor(current_block, &current_block->neighbor_w);
        strength /= 8;
        current_block->strength = strength;
        block_item = block_item->next;
    }


    return SUCCESS;
}
