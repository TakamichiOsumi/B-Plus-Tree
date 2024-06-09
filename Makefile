CC	= gcc
CFLAGS	= -Wall -O0 -g

DEPENDENCY_LIB	= Linked-List

COMPONENTS	= b_plus_tree.c
OBJ_COMPONENTS	= b_plus_tree.o

TEST_APP	= run_bptree

LIB	= libbplustree.a

all: library $(OBJ_COMPONENTS) $(TEST_APP)

library:
	for dir in $(DEPENDENCY_LIB); do make -C $$dir; done

$(OBJ_COMPONENTS):
	$(CC) $(CFLAGS) b_plus_tree.c -c

$(TEST_APP): $(OBJ_COMPONENTS)
	$(CC) $(CFLAGS) -L Linked-List -llinked_list tests/test_bpt_app.c $^ -o ./tests/$@

.phony: clean test

clean:
	@rm -rf *.o tests/$(TEST_APP)*
	@for dir in $(DEPENDENCY_LIB); do cd $$dir; make clean; cd ..; done

test: library $(TEST_APP)
	@./tests/$(TEST_APP) &> /dev/null && echo "Success when the return value is zero >>> $$?"
