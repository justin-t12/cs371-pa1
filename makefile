CC = gcc
CFLAGS = -pthread -Wall -Wextra -Werror -fanalyzer -O2
SOURCES = pa1_skeleton.c
OBJECTS = $(SOURCES:.c=.o)
EXECUTABLE = pa1_skeleton

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

debug: CFLAGS += -g -DDEBUG
debug: clean all

clean:
	rm -f $(EXECUTABLE) $(OBJECTS)
