CC = gcc
CFLAGS = -Wall -I./include
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

# List of source files
SERVER_SRC = $(SRC_DIR)/server.c $(SRC_DIR)/common.c
CLIENT_SRC = $(SRC_DIR)/client.c $(SRC_DIR)/common.c

# matching object files
SERVER_OBJ = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SERVER_SRC))
CLIENT_OBJ = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(CLIENT_SRC))

# executables
SERVER_EXE = $(BIN_DIR)/server
CLIENT_EXE = $(BIN_DIR)/client

all: $(SERVER_EXE) $(CLIENT_EXE)

# compile server
$(SERVER_EXE): $(SERVER_OBJ)
	$(CC) -o $@ $^

# Compile client
$(CLIENT_EXE): $(CLIENT_OBJ)
	$(CC) -o $@ $^

# Compile object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

# create directories if they dont exist
$(shell mkdir -p $(OBJ_DIR) $(BIN_DIR))

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)
