#Makefile

SHELL := /bin/bash
CXX = g++
CXXFLAGS = -Wall -std=c++2b
######################################
files = ./src/*.cc
######################################
BOLD   := $(shell tput bold)
CLEAR  := $(shell tput sgr0)
######################################
all: netcp

netcp: $(files)
	@$(CXX) $(CXXFLAGS) -o netcp $(files)

######################################
.PHONY: clean
clean:
	@echo "Cleaning up the directory..."
	@rm -rf *.o netcp
######################################
.PHONY: help
help:
	@echo "${BOLD}make${CLEAR} -> compile"
	@echo "${BOLD}make clean${CLEAR} -> remove [.out/.o] files"
	@echo "${BOLD}make test1${CLEAR} -> de momento no hay test"
	@echo "${BOLD}make test2${CLEAR} -> de momento no hay test"

######################################
#.PHONY: test1
#test1:
#	@chmod u+x ./test/test1.sh
#	@./test/test1.sh

#.PHONY: test2
#test2:
#	@chmod u+x ./test/test2.sh
#	@./test/test2.sh