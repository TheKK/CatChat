 #
 # Author: KK <thumbd03803@gmail.com>
 #
 # File: Makefile
 #

CXX = gcc
CXXFLAG = -Wall -std=gnu99 -g

SRC_PATH = $(PWD)/src

# Object files
OBJ_PATH = $(PWD)/obj
SERVER_OBJ = server_main.o	\
	     connect.o		\
	     clientPool.o	\

CLIENT_OBJ = client_main.o	\
	     connect.o		\

# Include flags
INCLUDE = -I $(PWD)/include

# Libs flags
LIB = -lpthread

SERVER_OUT_EXE = server
CLIENT_OUT_EXE = client

all: $(SERVER_OUT_EXE) $(CLIENT_OUT_EXE)
	@echo "===========[[Everything done!!]]============"

$(SERVER_OUT_EXE): $(addprefix $(OBJ_PATH)/, $(SERVER_OBJ))
	@echo "    LD    " $(notdir $@)
	@$(CXX) $(addprefix $(OBJ_PATH)/, $(SERVER_OBJ)) $(CXXFLAG) $(LIB) -o $@

$(CLIENT_OUT_EXE): $(addprefix $(OBJ_PATH)/, $(CLIENT_OBJ))
	@echo "    LD    " $(notdir $@)
	@$(CXX) $(addprefix $(OBJ_PATH)/, $(CLIENT_OBJ)) $(CXXFLAG) $(LIB) -o $@

$(OBJ_PATH)/%.o: $(SRC_PATH)/%.c
	@echo "    CC    " $(notdir $@)
	@$(CXX) -c $< $(CXXFLAG) $(INCLUDE) -o $@

.PHONY: tag
tag:
	@rm -f tags
	@ctags -aR --fields=+l --c-kinds=+px /usr/include/pthread.h
	@ctags -aR --fields=+l --c-kinds=+px /usr/include/sys/socket.h
	@ctags -aR --fields=+l --c-kinds=+px /usr/include/bits/socket.h
	@ctags -aR --fields=+l --c-kinds=+px $(PWD)
	@echo "===========[[Tags updated!!]]============"

.PHONY: clean
clean:
	@rm -frv $(OBJ_PATH)/*.o $(SERVER_OUT_EXE) $(CLIENT_OUT_EXE)
	@echo "===========[[Everything removed!!]]============"
