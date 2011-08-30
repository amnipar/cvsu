/**
 * @file cv_edges.c
 * @author Matti Eskelinen (matti dot j dot eskelinen at jyu dot fi)
 * @brief Edge detection and handling for the cv module.
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

#include "cv_edges.h"
#include "cv_filter.h"
#include "alloc.h"

#include <math.h>

result create_edge_image(edge_image *dst, pixel_image *src, long hstep, long vstep, long hmargin, long vmargin, long box_width, long box_length)
{
    result r;
    if (dst == NULL) {
        return BAD_POINTER;
    }
    if (src->type != U8) {
        return BAD_TYPE;
    }

    r = create_integral_image(&dst->integral, src);
    if (r != SUCCESS) {
        return r;
    }

    dst->hstep = hstep;
    dst->vstep = vstep;
    dst->hmargin = hmargin;
    dst->vmargin = vmargin;
    dst->box_width = box_width;
    dst->box_length = box_length;
    dst->width = (long)((src->width - 2 * hmargin) / hstep);
    dst->height = (long)((src->height - 2 * vmargin) / vstep);

    dst->dx = (long)((hstep - box_width) / 2);
    dst->dy = (long)((vstep - box_width) / 2);

    r = allocate_char_image(&dst->vedges, src->width, dst->height, 1);
    if (r != SUCCESS) {
        return r;
    }
    r = allocate_char_image(&dst->hedges, dst->width, src->height, 1);
    if (r != SUCCESS) {
        return r;
    }

    return SUCCESS;
}

result destroy_edge_image(edge_image *dst)
{
    //result r;
    if (dst == NULL) {
        return BAD_POINTER;
    }
    if ((*dst).hedges.data != NULL) {
        destroy_image(&dst->hedges);
    }
    if ((*dst).vedges.data != NULL) {
        destroy_image(&dst->vedges);
    }
    if ((*dst).integral.original != NULL) {
        destroy_integral_image(&dst->integral);
    }

    return SUCCESS;
}

result clone_edge_image(edge_image *dst, edge_image *src)
{
    result r;

    if (src == NULL || dst == NULL) {
        return BAD_POINTER;
    }
    r = clone_integral_image(&dst->integral, &src->integral);
    if (r != SUCCESS) {
        return r;
    }
    r = clone_image(&dst->hedges, &src->hedges);
    if (r != SUCCESS) {
        return r;
    }
    r = clone_image(&dst->vedges, &src->vedges);
    if (r != SUCCESS) {
        return r;
    }

    dst->width = src->width;
    dst->height = src->height;
    dst->hstep = src->hstep;
    dst->vstep = src->vstep;
    dst->hmargin = src->hmargin;
    dst->vmargin = src->vmargin;
    dst->box_width = src->box_width;
    dst->box_length = src->box_length;
    dst->dx = src->dx;
    dst->dy = src->dy;

    return SUCCESS;
}

result copy_edge_image(edge_image *dst, edge_image *src)
{
    result r;

    if (src == NULL || dst == NULL) {
        return BAD_POINTER;
    }
    if (src->width != dst->width || src->height != dst->height) {
        return BAD_SIZE;
    }
    r = copy_integral_image(&dst->integral, &src->integral);
    if (r != SUCCESS) {
        return r;
    }
    r = copy_image(&dst->hedges, &src->hedges);
    if (r != SUCCESS) {
        return r;
    }
    r = copy_image(&dst->vedges, &src->vedges);
    if (r != SUCCESS) {
        return r;
    }

    return SUCCESS;
}

long edgel_fisher_unsigned(long N, long sum1, long sum2, double sumsqr1, double sumsqr2)
{
    double mean1, mean2, diff, var1, var2, var;
    mean1 = (double)sum1 / N;
    mean2 = (double)sum2 / N;
    diff = mean2 - mean1;
    diff = diff * diff;
    var1 = (sumsqr1 / N) - (mean1 * mean1);
    var2 = (sumsqr2 / N) - (mean2 * mean2);
    var = var1 + var2;
    if (var < 1) var = 1;
    return (long)(diff / var);
}

long edgel_fisher_signed(long N, long sum1, long sum2, double sumsqr1, double sumsqr2)
{
    double mean1, mean2, var1, var2, var;

    mean1 = (double)sum1 / N;
    mean2 = (double)sum2 / N;
    var1 = (sumsqr1 / N) - (mean1 * mean1);
    var2 = (sumsqr2 / N) - (mean2 * mean2);
    var = var1 + var2;
    if (var < 1) var = 1;
    return (long)((mean2 - mean1) / sqrt(var));
}

result edgel_response_x(integral_image *src, pixel_image *dst, long hsize, long vsize, edgel_criterion_calculator criterion)
{
    long *dst_data;
    long *integral_data;
    double *integral2_data;
    long i, width, height, row, col, pos, rowInc, colInc, rInc;
    double N;
    const long *iAl, *iBl, *iCl, *iDl, *iAr, *iBr, *iCr, *iDr;
    const double *i2Al, *i2Bl, *i2Cl, *i2Dl, *i2Ar, *i2Br, *i2Cr, *i2Dr;

    long g, sum1, sum2;
    double sumsqr1, sumsqr2;

    if (src == NULL || dst == NULL) {
        return BAD_POINTER;
    }
    if ((*src).integral.data == NULL || (*src).integral2.data == NULL || dst->data == NULL) {
        return BAD_POINTER;
    }
    // src must be byte image, integral must be long image, integral2 must be double image
    if (dst->type != S32) {
        return BAD_TYPE;
    }
    // all images must have same dimensions
    if (src->width != dst->width || src->height != dst->height) {
        return BAD_SIZE;
    }

    dst_data = (long *)dst->data;
    integral_data = (long *)(*src).integral.data;
    integral2_data = (double *)(*src).integral2.data;

    width = src->width;
    height = src->height;

    rowInc = vsize * width;
    colInc = hsize;
    rInc = hsize + 1;

    N = (double)(hsize * vsize);

    reset((byte *)dst_data, (unsigned)(width * height), sizeof(long));

    for (row = 0; row < height - vsize; row += vsize) {
        iAl = integral_data + (row * width);
        iBl = iAl + colInc;
        iCl = iBl + rowInc;
        iDl = iAl + rowInc;
        iAr = iAl + rInc;
        iBr = iBl + rInc;
        iCr = iCl + rInc;
        iDr = iDl + rInc;

        i2Al = integral2_data + (row * width);
        i2Bl = i2Al + colInc;
        i2Cl = i2Bl + rowInc;
        i2Dl = i2Al + rowInc;
        i2Ar = i2Al + rInc;
        i2Br = i2Bl + rInc;
        i2Cr = i2Cl + rInc;
        i2Dr = i2Dl + rInc;

        pos = row * width + hsize + 1;
        for (col = hsize + 1; col < width - hsize; col++) {
            sum1 = *iCl - *iBl - *iDl + *iAl;
            sum2 = *iCr - *iBr - *iDr + *iAr;
            sumsqr1 = *i2Cl - *i2Bl - *i2Dl + *i2Al;
            sumsqr2 = *i2Cr - *i2Br - *i2Dr + *i2Ar;

            g = criterion(N, sum1, sum2, sumsqr1, sumsqr2);
            for (i = 0; i < vsize; i++) {
                dst_data[pos + i * width] = g;
            }

            iAl++;
            iBl++;
            iCl++;
            iDl++;
            iAr++;
            iBr++;
            iCr++;
            iDr++;
            i2Al++;
            i2Bl++;
            i2Cl++;
            i2Dl++;
            i2Ar++;
            i2Br++;
            i2Cr++;
            i2Dr++;

            pos++;
        }
    }
    return SUCCESS;
}

result edges_x_box_deviation(integral_image *src, pixel_image *temp, pixel_image *dst, long hsize, long vsize)
{
    result r;
    r = calculate_integrals(src);
    if (r != SUCCESS) {
        return r;
    }
    r = edgel_response_x(src, temp, hsize, vsize, &edgel_fisher_unsigned);
    if (r != SUCCESS) {
        return r;
    }
    r = extrema_x(temp, temp);
    if (r != SUCCESS) {
        return r;
    }
    r = normalize(temp, dst);
    if (r != SUCCESS) {
        return r;
    }

    return SUCCESS;
}

result calculate_edges(edge_image *edge)
{
    result r;
    long *integral_data;
    double *integral2_data;
    char *hedges_data;
    char *vedges_data;
    int width, height, row, rows, col, cols, pos, rowInc, colInc, g, prev;
    bool rising, falling;
    double N, sum1, sum2, sumsqr1, sumsqr2;

    if (edge == NULL) {
        return BAD_POINTER;
    }
    if ((*edge).integral.original == NULL || (*edge).hedges.data == NULL || (*edge).vedges.data == NULL) {
        return BAD_POINTER;
    }
    if ((*edge).vedges.type != S8 || (*edge).hedges.type != S8) {
        return BAD_TYPE;
    }

    r = calculate_integrals(&edge->integral);
    if (r != SUCCESS) {
        return r;
    }

    integral_data = (long *)((*edge).integral.integral.data);
    integral2_data = (double *)((*edge).integral.integral2.data);
    hedges_data = (char *)((*edge).hedges.data);
    vedges_data = (char *)((*edge).vedges.data);

    width = (*edge).integral.width;
    height = (*edge).integral.height;

    N = (double)(edge->box_width * edge->box_length);

    /*
      calculate vertical edges
    */
    {
        int startcol, endcol, rInc;
        const long *iAl, *iBl, *iCl, *iDl, *iAr, *iBr, *iCr, *iDr;
        const double *i2Al, *i2Bl, *i2Cl, *i2Dl, *i2Ar, *i2Br, *i2Cr, *i2Dr;

        rowInc = edge->box_width * width;
        colInc = edge->box_length;
        rInc = edge->box_length + 1;
        startcol = edge->box_length;
        endcol = width - edge->box_length;

        reset((byte *)vedges_data, (*edge).vedges.size, sizeof(char));

        rows = (*edge).vedges.height;
        for (row = 0; row < rows; row++) {
            iAl = integral_data + ((edge->vmargin + edge->dy + row * edge->vstep) * width);
            iBl = iAl + colInc;
            iCl = iBl + rowInc;
            iDl = iAl + rowInc;
            iAr = iAl + rInc;
            iBr = iBl + rInc;
            iCr = iCl + rInc;
            iDr = iDl + rInc;

            i2Al = integral2_data + ((edge->vmargin + edge->dy + row * edge->vstep) * width);
            i2Bl = i2Al + colInc;
            i2Cl = i2Bl + rowInc;
            i2Dl = i2Al + rowInc;
            i2Ar = i2Al + rInc;
            i2Br = i2Bl + rInc;
            i2Cr = i2Cl + rInc;
            i2Dr = i2Dl + rInc;

            rising = false;
            falling = false;
            pos = row * width + startcol;
            for (col = startcol; col < endcol; col++) {
                sum1 = *iAl + ((*iCl - *iBl) - *iDl);
                sum2 = *iAr + ((*iCr - *iBr) - *iDr);
                sumsqr1 = *i2Al + ((*i2Cl - *i2Bl) - *i2Dl);
                sumsqr2 = *i2Ar + ((*i2Cr - *i2Br) - *i2Dr);

                g = edgel_fisher_signed(N, sum1, sum2, sumsqr1, sumsqr2);

                if (col > startcol) {
                    if (g < prev) {
                        if (rising) { // found maximum at previous column
                            vedges_data[pos - 1] = (char)prev;
                            rising = false;
                        }
                        falling = true;
                    }
                    else if (g > prev) {
                        if (falling) { // found minimum
                            vedges_data[pos - 1] = (char)prev;
                            falling = false;
                        }
                        rising = true;
                    }
                }
                prev = g;

                iAl++;
                iBl++;
                iCl++;
                iDl++;
                iAr++;
                iBr++;
                iCr++;
                iDr++;
                i2Al++;
                i2Bl++;
                i2Cl++;
                i2Dl++;
                i2Ar++;
                i2Br++;
                i2Cr++;
                i2Dr++;

                pos++;
            }
        }
    }

    /*
      calculate horizontal edges
    */
    {
        int startrow, endrow, bInc;
        const long *iAt, *iBt, *iCt, *iDt, *iAb, *iBb, *iCb, *iDb;
        const double *i2At, *i2Bt, *i2Ct, *i2Dt, *i2Ab, *i2Bb, *i2Cb, *i2Db;

        rowInc = edge->box_length * width;
        colInc = edge->box_width;
        bInc = (edge->box_length + 1) * width;
        startrow = edge->box_length;
        endrow = height - edge->box_length;

        reset((byte *)hedges_data, (*edge).hedges.size, sizeof(char));

        cols = (*edge).hedges.width;
        for (col = 0; col < cols; col++) {
            iAt = integral_data + edge->hmargin + edge->dx + col * edge->hstep;
            iBt = iAt + colInc;
            iCt = iBt + rowInc;
            iDt = iAt + rowInc;
            iAb = iAt + bInc;
            iBb = iBt + bInc;
            iCb = iCt + bInc;
            iDb = iDt + bInc;

            i2At = integral2_data + edge->hmargin + edge->dx + col * edge->hstep;
            i2Bt = i2At + colInc;
            i2Ct = i2Bt + rowInc;
            i2Dt = i2At + rowInc;
            i2Ab = i2At + bInc;
            i2Bb = i2Bt + bInc;
            i2Cb = i2Ct + bInc;
            i2Db = i2Dt + bInc;

            rising = false;
            falling = false;
            pos = startrow * cols + col;
            for (row = startrow; row < endrow; row++) {
                sum1 = *iAt + ((*iCt - *iBt) - *iDt);
                sum2 = *iAb + ((*iCb - *iBb) - *iDb);
                sumsqr1 = *i2At + ((*i2Ct - *i2Bt) - *i2Dt);
                sumsqr2 = *i2Ab + ((*i2Cb - *i2Bb) - *i2Db);

                g = edgel_fisher_signed(N, sum1, sum2, sumsqr1, sumsqr2);

                if (row > startrow) {
                    if (g < prev) {
                        if (rising) { // found maximum at previous row
                            hedges_data[pos - cols] = (char)prev;
                            rising = false;
                        }
                        falling = true;
                    }
                    else if (g > prev) {
                        if (falling) { // found minimum at previous row
                            hedges_data[pos - cols] = (char)prev;
                            falling = false;
                        }
                        rising = true;
                    }
                }
                prev = g;

                iAt += width;
                iBt += width;
                iCt += width;
                iDt += width;
                iAb += width;
                iBb += width;
                iCb += width;
                iDb += width;
                i2At += width;
                i2Bt += width;
                i2Ct += width;
                i2Dt += width;
                i2Ab += width;
                i2Bb += width;
                i2Cb += width;
                i2Db += width;

                pos += cols;
            }
        }
    }

    return SUCCESS;
}
