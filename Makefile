CC=g++
LDFLAGS=-I/usr/local/include/opencv2 -lopencv_core -lopencv_videoio -lopencv_highgui -lopencv_imgcodecs -lopencv_imgproc -lsimlib -O2
TARGET=a.out
OBJS = $(patsubst %.cpp, %.o, $(wildcard src/*.cpp))

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $^ -o $@ $(LDFLAGS)

clean:
	rm -f $(TARGET) $(OBJS)
