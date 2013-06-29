SOURCES=$(shell find . -name \*.cpp)
HEADERS=$(shell find . -name \*.h)

CFLAGS=-O3

all: $(SOURCES) $(HEADERS)
	g++ $(CFLAGS) -o raytracer $(SOURCES)

clean:
	rm -f ./raytracer

run: all
	./raytracer