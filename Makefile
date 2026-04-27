TARGET = 65c832-emu

CC = gcc
CFLAGS = \
	-std=c99 \
	-Wpedantic \
	-Wextra \
	-Werror \
	-Wall \
	-Wstrict-aliasing=3 \
	-Wwrite-strings \
	-Wvla \
	-Wcast-align=strict \
	-Wstrict-prototypes \
	-Wstringop-overflow=4 \
	-Wshadow \
	-fanalyzer \
	-g \
	-O0

LEAK_CHKR = valgrind
LEAK_LVL = yes

AUTO_FMT = clang-format
FMT_STYLE = gnu

SRC_DIR = src
BIN_DIR = bin
OBJ_FILES = \
	main.o \
	opcode.o \
	addr_mode.o \
	utils.o

all: $(OBJ_FILES)
	$(CC) $(CFLAGS) $(BIN_DIR)/*.o -o $(TARGET)

%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -c -o$(BIN_DIR)/$@ $^

leak-check: all
	$(LEAK_CHKR) --leak-check=$(LEAK_LVL) ./$(TARGET) test-inputs/simple.bin

format:
	$(AUTO_FMT) -style=$(FMT_STYLE) -i $(SRC_DIR)/*.c $(SRC_DIR)/*.h

clean:
	-rm -rf $(BIN_DIR)
	-rm -rf $(TARGET)
