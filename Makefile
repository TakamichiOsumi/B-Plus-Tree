CC	= gcc
CFLAGS	= -Wall -O0 -g

COMPONENTS	= b_plus_tree.c
OBJ_COMPONENTS	= b_plus_tree.o

TEST_APP	= exec_bptree

LIB	= libbplustree.a

all: $(TEST_APP)

$(OBJ_COMPONENTS):
	$(CC) b_plus_tree.c -c

$(TEST_APP): $(OBJ_COMPONENTS)
	$(CC) test_app.c $^ -o $@

.phony: clean

clean:
	@rm -rf *.o $(TEST_APP) $(TEST_APP).dSYM
