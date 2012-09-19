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

cvsu-test: cvsu_memory.o cvsu_output.o cvsu_basic.o cvsu_integral.o cvsu_filter.o cvsu_scale.o cvsu_edges.o cvsu_list.o cvsu_simple_scene.o cvsu_image_tree.o cvsu_opencv.o main.o
	gcc -o cvsu-test cvsu_memory.o cvsu_output.o cvsu_basic.o cvsu_integral.o cvsu_filter.o cvsu_scale.o cvsu_edges.o cvsu_list.o cvsu_simple_scene.o cvsu_image_tree.o cvsu_opencv.o main.o -lm -lopencv_core -lopencv_highgui -I.

.PHONY: clean

clean:
	rm -f cvsu-test *.o
