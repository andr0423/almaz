Test task
---------

doc/         - conditions and description

generate_datafile.py - test data generator

nmt_c5.cpp   - solution

Makefile     - make [help|all|compile|datafile|run|result|expected|actual|diff|check|execute|clean|clean-data|clean-all]

make all     - compile, generate test datafile, run binary with test datafile, diff result

make check   - compare generated tadafile and result from binary

make execute - run with any datafile, by default "numbers_light.txt"

make execute ORIGINAL_DATAFILE=anyfile.txt

