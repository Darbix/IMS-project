######################################
# @file Makefile
# @author David Kedra, xkedra00
# @author Petr Kolařík, xkolar79
# 
# IMS Project - Cellular automata
# VUT FIT Brno, 2022/2023 
######################################

CC=g++
LDFLAGS=-I/usr/local/include/opencv2 -lopencv_core -lopencv_videoio -lopencv_highgui -lopencv_imgcodecs -lopencv_imgproc -lsimlib -O2
TARGET=simulator
OBJS = $(patsubst %.cpp, %.o, $(wildcard src/*.cpp))

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $^ -o $@ $(LDFLAGS)

clean:
	rm -f $(TARGET) $(OBJS)
