ORIGINAL_DATAFILE ?= numbers_light.txt

SRC_FILE      ?= mmt_c5.cpp
EXEC_FILE     ?= $(basename $(SRC_FILE))

TEST_DATAFILE_SIZE ?= $(shell echo "$$(( 1024 * 1024 ))" )
TEST_DATA_FILE     ?= datafile.txt
TEST_EXPECT_FILE   ?= "$(basename $(TEST_DATA_FILE))_expected.txt"
TEST_ACTUAL_FILE   ?= $(EXEC_FILE)_result.txt

ifeq ($(SRC_FILE),$(EXEC_FILE))
$(error "Incorrect src file name without suffix, SRC_FILE='$(SRC_FILE)'")
endif

help:
	@cat Readme.md

all: compile  datafile  result  diff

compile: $(EXEC_FILE)

$(EXEC_FILE):
	@echo "Compile start ..."
	g++ -v -Wall -pthread -lpthread  -std=c++11  -o $(EXEC_FILE)  $(SRC_FILE)
	chmod 0755 $(EXEC_FILE)
	@echo "Compile OK"

datafile: $(TEST_DATA_FILE)

$(TEST_DATA_FILE):
	@echo "Generate datafile start ..."
	@python3 generate_datafile.py  $(TEST_DATAFILE_SIZE)  $(TEST_DATA_FILE)  $(TEST_EXPECT_FILE) 2>/dev/null
	@echo "Generate datafile OK"

run: compile  datafile
	@./$(EXEC_FILE) $(TEST_DATA_FILE) 2>/dev/null | tee $(TEST_ACTUAL_FILE)

.PHONY: run

result: $(TEST_ACTUAL_FILE) compile  datafile

$(TEST_ACTUAL_FILE):
	@echo "Calculate result start ..."
	@./$(EXEC_FILE) $(TEST_DATA_FILE) 2>/dev/null > $(TEST_ACTUAL_FILE)
	@echo "Calculate result OK"

expected: datafile
	@echo
	@echo Expected:
	@echo "==== ==== ==== ==== ==== ===="
	@cat $(TEST_EXPECT_FILE)
	@echo

actual: result
	@echo
	@echo Actual:
	@echo "==== ==== ==== ==== ==== ===="
	@cat $(TEST_ACTUAL_FILE)
	@echo

diff:  expected  actual

check: datafile result
	diff $(TEST_EXPECT_FILE) $(TEST_ACTUAL_FILE)

.PHONY:  diff  expected  actual  check

execute:  compile
	./$(EXEC_FILE) $(ORIGINAL_DATAFILE)

.PHONY: execute

clean:
	rm -r -f ./$(EXEC_FILE)  ./$(EXEC_FILE).dSYM  $(TEST_ACTUAL_FILE)  ./__pycache__

clean-data:
	rm -f ./$(TEST_DATA_FILE) ./$(TEST_EXPECT_FILE)

clean-result:
	rm -f ./$(TEST_ACTUAL_FILE)

clean-all: clean clean-data clean-result

