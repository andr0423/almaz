SRC_FILE      ?= mmt_c5.cpp
EXEC_FILE     ?= $(basename $(SRC_FILE))

DATAFILE_SIZE ?= $(shell echo "$$(( 1024 * 1024 ))" )
DATA_FILE     ?= datafile.txt
EXPECT_FILE   ?= "$(basename $(DATA_FILE))_expected.txt"
ACTUAL_FILE   ?= $(EXEC_FILE)_result.txt

ifeq ($(SRC_FILE),$(EXEC_FILE))
$(error "Incorrect src file name  without suffix, SRC_FILE='$(SRC_FILE)'")
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

datafile: $(DATA_FILE)

$(DATA_FILE):
	@echo "Generate datafile start ..."
	@python3 generate_datafile.py  $(DATAFILE_SIZE)  $(DATA_FILE)  $(EXPECT_FILE) 2>/dev/null
	@echo "Generate datafile OK"

exec:  compile  datafile
	@echo "Exec $(EXEC_FILE)"
	time ./$(EXEC_FILE) | tee $(ACTUAL_FILE)
	@echo

run: compile  datafile
	@./$(EXEC_FILE) 2>/dev/null | tee $(ACTUAL_FILE)

.PHONY: exec run

result: $(ACTUAL_FILE) compile  datafile

$(ACTUAL_FILE):
	@echo "Calculate result start ..."
	@./$(EXEC_FILE) 2>/dev/null > $(ACTUAL_FILE)
	@echo "Calculate result OK"

expected: datafile
	@echo
	@echo Expected:
	@echo "==== ==== ==== ==== ==== ===="
	@cat $(EXPECT_FILE)
	@echo

actual: result
	@echo
	@echo Actual:
	@echo "==== ==== ==== ==== ==== ===="
	@cat $(ACTUAL_FILE)
	@echo

diff:  expected  actual

.PHONY:  diff  expected  actual

clean:
	rm -r -f ./$(EXEC_FILE)  ./$(EXEC_FILE).dSYM  $(ACTUAL_FILE)  ./__pycache__

clean-data:
	rm -f ./$(DATA_FILE) ./$(EXPECT_FILE)

clean-result:
	rm -f ./$(ACTUAL_FILE)

clean-all: clean clean-data clean-result

