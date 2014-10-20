 #
 # CatChat
 # Copyright (C) 2014 TheKK <thumbd03803@gmail.com>
 #
 # This program is free software; you can redistribute it and/or modify
 # it under the terms of the GNU General Public License as published by
 # the Free Software Foundation; either version 2 of the License, or
 # (at your option) any later version.
 #
 # This program is distributed in the hope that it will be useful,
 # but WITHOUT ANY WARRANTY; without even the implied warranty of
 # MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 # GNU General Public License for more details.
 #
 # You should have received a copy of the GNU General Public License
 # along with this program; if not, write to the Free Software
 # Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 #

CXX = gcc
CXXFLAG = -Wall -std=gnu99 -g

SRC_PATH = $(PWD)/src

# Object files
OBJ_PATH = $(PWD)/obj
SERVER_OBJ = server_main.o	\
	     connect.o		\
	     thpool.o

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
