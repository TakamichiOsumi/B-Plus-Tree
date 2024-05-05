CC	= gcc
CFLAGS	= -Wall -O0 -g

DEPENDENCY_LIB	= Linked-List

COMPONENTS	= b_plus_tree.c
OBJ_COMPONENTS	= b_plus_tree.o

TEST_APP	= exec_bptree

LIB	= libbplustree.a

all: library $(TEST_APP)

library:
	for dir in $(DEPENDENCY_LIB); do make -C $$dir; done

$(OBJ_COMPONENTS):
	$(CC) b_plus_tree.c -c

$(TEST_APP): $(OBJ_COMPONENTS)
	$(CC) -L Linked-List -llinked_list test_app.c $^ -o $@

.phony: clean test

clean:
	@rm -rf *.o $(TEST_APP) $(TEST_APP).dSYM
	@for dir in $(DEPENDENCY_LIB); do cd $$dir; make clean; cd ..; done

test: $(TEST_APP)
	@./$(TEST_APP) &> /dev/null && echo "Success when the return value is zero >>> $$?"
