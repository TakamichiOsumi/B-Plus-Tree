CC	= gcc
CFLAGS	= -Wall -O0 -g

DEPENDENCY_LIB	= Linked-List

COMPONENTS	= b_plus_tree.c
OBJ_COMPONENTS	= b_plus_tree.o

KEYS_APP	= key_management_bptree
RECORDS_APP	= record_management_bptree
KEY_HANDLER_APP	= key_handler_bptree
COMPOSITE_KEYS_APP	= composite_keys_bptree

FULL_TESTS	= $(KEYS_APP) $(RECORDS_APP) $(KEY_HANDLER_APP) $(COMPOSITE_KEYS_APP)

LIB	= libbplustree.a

all: $(LIB) $(KEYS_APP) $(RECORDS_APP) $(COMPOSITE_KEYS_APP) $(KEY_HANDLER_APP)

library:
	for dir in $(DEPENDENCY_LIB); do make -C $$dir; done

$(OBJ_COMPONENTS): library
	$(CC) $(CFLAGS) bpt_key_handler.c -c
	$(CC) $(CFLAGS) b_plus_tree.c -c

$(KEYS_APP): $(OBJ_COMPONENTS)
	$(CC) $(CFLAGS) -L Linked-List -llinked_list tests/keys_bpt_app.c $^ -o ./tests/$@

$(RECORDS_APP): $(OBJ_COMPONENTS)
	$(CC) $(CFLAGS) -L Linked-List -llinked_list tests/records_bpt_app.c $^ -o ./tests/$@

$(COMPOSITE_KEYS_APP): $(OBJ_COMPONENTS)
	$(CC) $(CFLAGS) -L Linked-List -llinked_list tests/composite_keys_app.c $^ -o ./tests/$@

$(KEY_HANDLER_APP): $(OBJ_COMPONENTS)
	$(CC) $(CFLAGS) -L Linked-List -llinked_list tests/key_handler_tests.c bpt_key_handler.o -o ./tests/$@

$(LIB): $(OBJ_COMPONENTS)
	ar rs $@ $<

.phony: clean test

clean:
	@rm -rf *.o tests/$(KEYS_APP)* tests/$(RECORDS_APP)* \
		tests/$(COMPOSITE_KEYS_APP)* $(KEY_HANDLER_APP) $(LIB)
	@for dir in $(DEPENDENCY_LIB); do cd $$dir; make clean; cd ..; done

test: $(OBJ_COMPONENTS) $(FULL_TESTS)
	@echo "Run several tests. This will take some time..."
	@for exec in $(FULL_TESTS); do ./tests/$$exec &> /dev/null && echo "Success when the value is zero >>> $$?"; done
