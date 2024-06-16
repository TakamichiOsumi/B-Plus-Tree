CC	= gcc
CFLAGS	= -Wall -O0 -g

DEPENDENCY_LIB	= Linked-List

COMPONENTS	= b_plus_tree.c
OBJ_COMPONENTS	= b_plus_tree.o

KEYS_APP	= key_management_bptree
RECORDS_APP	= record_management_bptree

LIB	= libbplustree.a

all: library $(OBJ_COMPONENTS) $(KEYS_APP) $(RECORDS_APP)

library:
	for dir in $(DEPENDENCY_LIB); do make -C $$dir; done

$(OBJ_COMPONENTS):
	$(CC) $(CFLAGS) b_plus_tree.c -c

$(KEYS_APP): $(OBJ_COMPONENTS)
	$(CC) $(CFLAGS) -L Linked-List -llinked_list tests/keys_bpt_app.c $^ -o ./tests/$@

$(RECORDS_APP): $(OBJ_COMPONENTS)
	$(CC) $(CFLAGS) -L Linked-List -llinked_list tests/records_bpt_app.c $^ -o ./tests/$@

.phony: clean test

clean:
	@rm -rf *.o tests/$(KEYS_APP)* tests/$(RECORDS_APP)*
	@for dir in $(DEPENDENCY_LIB); do cd $$dir; make clean; cd ..; done

test: library $(KEYS_APP) $(RECORDS_APP)
	@./tests/$(KEYS_APP) &> /dev/null && echo "Success when the return value is zero >>> $$?"
	@./tests/$(RECORDS_APP) &> /dev/null && echo "Success when the return value is zero >>> $$?"
