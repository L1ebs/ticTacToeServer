# Compiler options
CC = gcc
CFLAGS = -Wall -Wextra

# Source files
SRCS_TTT = ttt.c
SRCS_TTTS = ttts.c

# Object files
OBJS_TTT = $(SRCS_TTT:.c=.o)
OBJS_TTTS = $(SRCS_TTTS:.c=.o)

# Executables
EXE_TTT = ttt
EXE_TTTS = ttts

.PHONY: all clean

all: $(EXE_TTT) $(EXE_TTTS)

$(EXE_TTT): $(OBJS_TTT)
	$(CC) $(CFLAGS) -o $@ $^

$(EXE_TTTS): $(OBJS_TTTS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(EXE_TTT) $(EXE_TTTS) $(OBJS_TTT) $(OBJS_TTTS)
