.DEFAULT_GOAL := all

CFLAGS = -fopenmp

src=$(wildcard *.cpp)
obj=$(src:.cpp=.o)

all: raytracer.bin clean

run: all
	./raytracer.bin

clean:
	rm *.o

%.o: %.cpp
	g++ -c -fopenmp $< -o $@

raytracer.bin: $(obj)
	g++ $(obj) -o raytracer.bin -std=c++11 -fopenmp -lsfml-graphics -lsfml-window -lsfml-system
