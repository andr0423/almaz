SRC_FILE      ?= mmt_c5.cpp
EXEC_FILE     ?= $(basename $(SRC_FILE))

DATAFILE_SIZE ?= 1024
DATA_FILE     ?= datafile.txt
EXPECT_FILE   ?= "$(basename $(DATA_FILE))_expected.txt"
#datafile_expected.txt

ifeq ($(SRC_FILE),$(EXEC_FILE))
$(error "Incorrect src file name  without suffix, SRC_FILE='$(SRC_FILE)'")
#else
#$(info "$(SRC_FILE),$(EXEC_FILE)")
endif

help:
	@cat Readme.md

all: compile  datatfile  run

compile: $(EXEC_FILE)

$(EXEC_FILE):
	@echo "Compile start ..."
	g++ -v -Wall -pthread -lpthread  -std=c++11  -o $(EXEC_FILE)  $(SRC_FILE)
	chmod 0755 $(EXEC_FILE)
	@echo "Compile OK"

datatfile: $(DATA_FILE)

$(DATA_FILE):
	python3 generate_datafile.py  $(DATAFILE_SIZE)  $(DATA_FILE)  $(EXPECT_FILE)

exec:  compile  datatfile
	@echo "Exec $(EXEC_FILE)"
	time ./$(EXEC_FILE)
	@echo

run: compile  datatfile
	@./$(EXEC_FILE) 2>/dev/null

diff:  compile  datatfile
	@echo
	@echo Expected:
	@echo "==== ==== ==== ==== ==== ===="
	@cat $(EXPECT_FILE)
	@echo
	@echo Actual:
	@echo "==== ==== ==== ==== ==== ===="
	@./$(EXEC_FILE) 2>/dev/null
	@echo

clean:
	rm -r -f ./$(EXEC_FILE)  ./$(EXEC_FILE).dSYM  ./__pycache__

clean-data:
	rm -f ./$(DATA_FILE) ./datafile_expected.txt

clean-all: clean clean-data
