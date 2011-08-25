/**
 * @file list_t.c
 * @author Matti Eskelinen (matti dot j dot eskelinen at jyu dot fi)
 * @brief Testing code for list.c
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

#include "list.h"

#include <stdlib.h>
#include <stdio.h>

result print_item(const list_item *item)
{
    printf("Item: %d\n", *(long *)item->data);
    return SUCCESS;
}

bool match_item(const void *a, const void *b)
{
    long la, lb;
    bool r;
    la = *(long *)a;
    lb = *(long *)b;
    printf("match_item a=%d b=%d\n", la, lb);
    r = (la == lb);//(*(long *)a == *(long *)b);
    printf("match_item result=%d\n", (int)r);
    return r;
}

int main(int argc, char *argv[])
{
    printf("Starting list tests\n");

    result r;
    list test_list;
    long value;
    r = list_allocate(&test_list, 10, sizeof(long));
    printf("list_allocate result=%d\n", (int)r);
    value = 1;
    r = list_append(&test_list, &value);
    printf("list_append result=%d\n", (int)r);
    value = 2;
    r = list_append(&test_list, &value);
    printf("list_append result=%d\n", (int)r);
    value = 3;
    r = list_prepend(&test_list, &value);
    printf("list_prepend result=%d\n", (int)r);
    r = list_iterate_forward(test_list.first, test_list.last, &print_item);
    printf("list_iterate_forward result=%d\n", (int)r);
    value = 1;
    r = list_remove(&test_list, &value, &match_item);
    printf("list_remove result=%d\n", (int)r);
    r = list_iterate_forward(test_list.first, test_list.last, &print_item);
    printf("list_iterate_forward result=%d\n", (int)r);
    printf("chunk item count = %d\n", test_list.data_chunk.count);
    r = list_append(&test_list, &value);
    printf("list_append result=%d\n", (int)r);
    printf("chunk item count = %d\n", test_list.data_chunk.count);
    r = list_iterate_forward(test_list.first, test_list.last, &print_item);
    printf("list_iterate_forward result=%d\n", (int)r);
    value = 3;
    r = list_remove(&test_list, &value, &match_item);
    printf("list_remove result=%d\n", (int)r);
    printf("chunk item count = %d\n", test_list.data_chunk.count);
    r = list_iterate_forward(test_list.first, test_list.last, &print_item);
    printf("list_iterate_forward result=%d\n", (int)r);
    value = 2;
    r = list_remove(&test_list, &value, &match_item);
    printf("list_remove result=%d\n", (int)r);
    printf("chunk item count = %d\n", test_list.data_chunk.count);
    r = list_iterate_forward(test_list.first, test_list.last, &print_item);
    printf("list_iterate_forward result=%d\n", (int)r);
    value = 1;
    r = list_remove(&test_list, &value, &match_item);
    printf("list_remove result=%d\n", (int)r);
    printf("chunk item count = %d\n", test_list.data_chunk.count);
    r = list_iterate_forward(test_list.first, test_list.last, &print_item);
    printf("list_iterate_forward result=%d\n", (int)r);
    r = list_destroy(&test_list);
    printf("list_destroy result=%d\n", (int)r);
}
