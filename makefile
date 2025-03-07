CC = gcc
CFLAGS = -pthread -Wall -Wextra -Werror -fanalyzer -O2
SOURCES = pa1.c
OBJECTS = $(SOURCES:.c=.o)
EXECUTABLE = pa1

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

debug: CFLAGS += -g -DDEBUG
debug: clean all

clean:
	rm -f $(EXECUTABLE) $(OBJECTS)
