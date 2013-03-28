# cvsu-test Makefile
#
# USAGE:
# To compile executable -
#        make
#
# To clean executable and object files -
#        make clean
#
# Author: Matti Johannes Eskelinen <matti.j.eskelinen@jyu.fi>
# Copyright (c) 2011 University of Jyvaskyla.
# All rights reserved.

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

edges: cvsu_memory.o cvsu_output.o cvsu_types.o cvsu_pixel_image.o cvsu_integral.o cvsu_filter.o cvsu_edges.o cvsu_list.o cvsu_opencv.o find_edges.o
	gcc -o find_edges cvsu_memory.o cvsu_output.o cvsu_types.o cvsu_pixel_image.o cvsu_integral.o cvsu_filter.o cvsu_edges.o cvsu_list.o cvsu_opencv.o find_edges.o -lm -lopencv_core -lopencv_highgui -I.

segment: cvsu_memory.o cvsu_output.o cvsu_types.o cvsu_typed_pointer.o cvsu_pixel_image.o cvsu_integral.o cvsu_list.o cvsu_edges.o cvsu_filter.o cvsu_context.o cvsu_annotation.o cvsu_quad_tree.o cvsu_quad_forest.o cvsu_segmentation.o cvsu_parsing.o cvsu_opencv.o quad_forest_segment.o
	gcc -o quad_forest_segment cvsu_memory.o cvsu_output.o cvsu_types.o cvsu_typed_pointer.o cvsu_pixel_image.o cvsu_integral.o cvsu_list.o cvsu_edges.o cvsu_filter.o cvsu_context.o cvsu_annotation.o cvsu_quad_tree.o cvsu_quad_forest.o cvsu_segmentation.o cvsu_parsing.o cvsu_opencv.o quad_forest_segment.o -lm -lopencv_core -lopencv_highgui -I.

threshold: cvsu_memory.o cvsu_output.o cvsu_types.o cvsu_pixel_image.o cvsu_integral.o cvsu_list.o cvsu_connected_components.o cvsu_opencv.o threshold_adaptive.o
	gcc -o threshold_adaptive cvsu_memory.o cvsu_output.o cvsu_types.o cvsu_pixel_image.o cvsu_integral.o cvsu_list.o cvsu_connected_components.o cvsu_opencv.o threshold_adaptive.o -lm -lopencv_core -lopencv_highgui -I.

parse: cvsu_memory.o cvsu_output.o cvsu_types.o cvsu_typed_pointer.o cvsu_pixel_image.o cvsu_integral.o cvsu_list.o cvsu_edges.o cvsu_filter.o cvsu_quad_tree.o cvsu_quad_forest.o cvsu_context.o cvsu_annotation.o cvsu_parsing.o  cvsu_opencv.o parse.o
	gcc -o parse cvsu_memory.o cvsu_output.o cvsu_types.o cvsu_typed_pointer.o cvsu_pixel_image.o cvsu_integral.o cvsu_list.o cvsu_edges.o cvsu_filter.o cvsu_quad_tree.o cvsu_quad_forest.o cvsu_context.o cvsu_annotation.o cvsu_parsing.o cvsu_opencv.o parse.o -lm -lopencv_core -lopencv_highgui -I.
