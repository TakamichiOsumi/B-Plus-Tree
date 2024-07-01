CC	= gcc
CFLAGS	= -Wall -O0 -g

DEPENDENCY_LIB	= Linked-List

COMPONENTS	= b_plus_tree.c
OBJ_COMPONENTS	= b_plus_tree.o
OUTPUT_LIB	= libbpt.a

KEYS_APP	= key_management_bptree
RECORDS_APP	= record_management_bptree

LIB	= libbplustree.a

all: library $(OUTPUT_LIB)

library:
	for dir in $(DEPENDENCY_LIB); do make -C $$dir; done

$(OBJ_COMPONENTS):
	$(CC) $(CFLAGS) b_plus_tree.c -c

$(KEYS_APP): $(OBJ_COMPONENTS)
	$(CC) $(CFLAGS) -L Linked-List -llinked_list tests/keys_bpt_app.c $^ -o ./tests/$@

$(RECORDS_APP): $(OBJ_COMPONENTS)
	$(CC) $(CFLAGS) -L Linked-List -llinked_list tests/records_bpt_app.c $^ -o ./tests/$@

$(OUTPUT_LIB): $(OBJ_COMPONENTS)
	ar rs $@ $<

.phony: clean test

clean:
	@rm -rf *.o tests/$(KEYS_APP)* tests/$(RECORDS_APP)* $(OUTPUT_LIB)
	@for dir in $(DEPENDENCY_LIB); do cd $$dir; make clean; cd ..; done

test: library $(KEYS_APP) $(RECORDS_APP)
	@./tests/$(KEYS_APP) &> /dev/null && echo "Success when the value of keys management tests is zero >>> $$?"
	@./tests/$(RECORDS_APP) &> /dev/null && echo "Success when the value of records management tests is zero >>> $$?"
