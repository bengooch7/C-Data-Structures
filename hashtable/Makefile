CFLAGS     = -Wall -Wextra -Wpedantic
LDLIBS     = -lpthread

# Automatic project management
ROOTDIR = .
SRCDIR  = src
OBJDIR  = build
BINDIR  = $(OBJDIR)/bin
SRCS    = $(wildcard $(SRCDIR)/*.c)
OBJS    = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRCS))
BIN     = $(BINDIR)/hashtable.exe

all: $(BIN)

$(BIN): $(OBJS)
	mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LDLIBS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

debug: CFLAGS += -ggdb3
debug: $(BIN)

clean:
	$(RM) $(BIN) $(OBJDIR)/*.o

good: clean debug
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes -s ./$(BIN)

test: clean all
	./$(BIN)

test_dbg: clean debug
	./$(BIN)

