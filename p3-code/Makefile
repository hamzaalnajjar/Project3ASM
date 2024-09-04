# CMSC216 Project 3 Makefile
AN = p3
CLASS = 216

# -Wno-comment: disable warnings for multi-line comments, present in
# some tests

# -fpic -pie : defaults on most linux systems but NOT GRACE; include
# these so that students using incorrect syntax to access global
# variables will get errors

CFLAGS = -Wall -Wno-comment -Werror -g -Og -fpic -pie
CC     = gcc $(CFLAGS)
SHELL  = /bin/bash
CWD    = $(shell pwd | sed 's/.*\///g')

PROGRAMS = \
	thermo_main \
	test_thermo_update \



all : $(PROGRAMS)

clean :
	rm -f $(PROGRAMS) *.o core vgcore.*

help :
	@echo 'Typical usage is:'
	@echo '  > make                          # build all programs'
	@echo '  > make clean                    # remove all compiled items'
	@echo '  > make zip                      # create a zip file for submission'
	@echo '  > make prob1                    # built targets associated with problem 1'
	@echo '  > make prob1 testnum=5          # run problem 1 test #5 only'
	@echo '  > make test                     # run all tests'

############################################################
# 'make zip' to create complete.zip for submission
ZIPNAME = $(AN)-complete.zip
zip : clean clean-tests
	rm -f $(ZIPNAME)
	cd .. && zip "$(CWD)/$(ZIPNAME)" -r "$(CWD)"
	@echo Zip created in $(ZIPNAME)
	@if (( $$(stat -c '%s' $(ZIPNAME)) > 10*(2**20) )); then echo "WARNING: $(ZIPNAME) seems REALLY big, check there are no abnormally large test files"; du -h $(ZIPNAME); fi
	@if (( $$(unzip -t $(ZIPNAME) | wc -l) > 256 )); then echo "WARNING: $(ZIPNAME) has 256 or more files in it which may cause submission problems"; fi

################################################################################
# thermometer problem

# build .o files from corresponding .c files
%.o : %.c thermo.h
	$(CC) -c $<

# build assembly object via gcc + debug flags
thermo_update_asm.o : thermo_update_asm.s thermo.h
	$(CC) -c $<

thermo_main : thermo_main.o thermo_sim.o thermo_update_asm.o 
	$(CC) -o $@ $^

# thermo_update functions testing program
test_thermo_update : test_thermo_update.o thermo_sim.o thermo_update_asm.o test_thermo_update_asm.s
	$(CC) -o $@ $^


################################################################################
# Testing Targets
test-setup :
	@chmod u+rx testy puzzlebin

test: test-prob1 test-prob2

test-prob1: thermo_main test_thermo_update test-setup
	./testy test_thermo_update.org $(testnum)

test-prob2: puzzlebin test-setup
	./puzzlebin input.txt

clean-tests : 
	rm -rf test-results/ test_thermo_update 

