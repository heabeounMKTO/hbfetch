OBJS = main.cpp

CC = g++

COMPILER_FLAGS = -Wall -DNDEBUG -O3

RELEASE_NAME = hbfetch
LINKER_FLAGS = 
OBJ_NAME = main.out

debug: $(OBJS) 
	$(CC) $(OBJS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(OBJ_NAME)
	./$(OBJ_NAME)

release:
	$(CC) $(OBJS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(RELEASE_NAME) 

install: release
	mv $(RELEASE_NAME) /usr/bin
	
uninstall:
	rm /usr/bin/$(RELEASE_NAME)
