CC=g++
LDFLAGS=-std=c++11 -O3 -lm
SOURCES=*.cpp
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=fp
INCLUDES=floorplanner.h module.h

all: $(SOURCES) bin/$(EXECUTABLE)

bin/$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

%.o:  %.c  ${INCLUDES}
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf *.o bin/$(EXECUTABLE)
