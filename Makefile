# cvsu Makefile
#
# USAGE:
# To compile all -
#        make [ edges | threshold | segment | parse ]
#
# To clean executable and object files -
#        make clean
#
# Author: Matti J. Eskelinen <matti.j.eskelinen@gmail.com>
#
# Copyright (c) 2011-2013, Matti Johannes Eskelinen
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#   * Redistributions of source code must retain the above copyright notice,
#     this list of conditions and the following disclaimer.
#   * Redistributions in binary form must reproduce the above copyright notice,
#     this list of conditions and the following disclaimer in the documentation
#     and/or other materials provided with the distribution.
#   * Neither the name of the copyright holder nor the names of its
#     contributors may be used to endorse or promote products derived from this
#     software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDER BE LIABLE FOR ANY DIRECT,
# INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

CC=gcc
CFLAGS=-I. \
		-Wall -W -Wconversion -Wshadow -Wcast-qual -ansi \
		-pedantic -std=c89

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

.PHONY: clean

all: edges segment threshold parse

clean:
	rm -f find_edges quad_forest_segment threshold_adaptive *.o

edges: cvsu_memory.o cvsu_output.o cvsu_types.o cvsu_pixel_image.o \
		cvsu_integral.o cvsu_filter.o cvsu_edges.o cvsu_list.o \
		cvsu_opencv.o find_edges.o
	gcc -o find_edges cvsu_memory.o cvsu_output.o cvsu_types.o \
		cvsu_pixel_image.o cvsu_integral.o cvsu_filter.o cvsu_edges.o \
		cvsu_list.o cvsu_opencv.o \
		find_edges.o -lm -lopencv_core -lopencv_highgui -I.

segment: cvsu_memory.o cvsu_output.o cvsu_types.o cvsu_typed_pointer.o \
		cvsu_pixel_image.o cvsu_integral.o cvsu_list.o cvsu_edges.o \
		cvsu_filter.o cvsu_context.o cvsu_annotation.o cvsu_quad_tree.o \
		cvsu_quad_forest.o cvsu_segmentation.o cvsu_parsing.o \
		cvsu_opencv.o quad_forest_segment.o
	gcc -o quad_forest_segment cvsu_memory.o cvsu_output.o cvsu_types.o \
		cvsu_typed_pointer.o cvsu_pixel_image.o cvsu_integral.o \
		cvsu_list.o cvsu_edges.o cvsu_filter.o cvsu_context.o \
		cvsu_annotation.o cvsu_quad_tree.o cvsu_quad_forest.o \
		cvsu_segmentation.o cvsu_parsing.o cvsu_opencv.o \
		quad_forest_segment.o -lm -lopencv_core -lopencv_highgui -I.

threshold: cvsu_memory.o cvsu_output.o cvsu_types.o cvsu_pixel_image.o \
		cvsu_integral.o cvsu_list.o cvsu_connected_components.o \
		cvsu_opencv.o threshold_adaptive.o
	gcc -o threshold_adaptive cvsu_memory.o cvsu_output.o cvsu_types.o \
		cvsu_pixel_image.o cvsu_integral.o cvsu_list.o \
		cvsu_connected_components.o cvsu_opencv.o \
		threshold_adaptive.o -lm -lopencv_core -lopencv_highgui -I.

parse: cvsu_memory.o cvsu_output.o cvsu_types.o cvsu_typed_pointer.o \
		cvsu_pixel_image.o cvsu_integral.o cvsu_list.o cvsu_edges.o \
		cvsu_filter.o cvsu_quad_tree.o cvsu_quad_forest.o \
		cvsu_context.o cvsu_annotation.o cvsu_parsing.o  cvsu_opencv.o \
		parse.o
	gcc -o parse cvsu_memory.o cvsu_output.o cvsu_types.o \
		cvsu_typed_pointer.o cvsu_pixel_image.o cvsu_integral.o \
		cvsu_list.o cvsu_edges.o cvsu_filter.o cvsu_quad_tree.o \
		cvsu_quad_forest.o cvsu_context.o cvsu_annotation.o \
		cvsu_parsing.o cvsu_opencv.o \
		parse.o -lm -lopencv_core -lopencv_highgui -I.
