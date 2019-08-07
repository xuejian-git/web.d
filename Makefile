PROJECT := $(shell pwd)
MAIN 	:= $(PROJECT)/src/Main.cpp
SRC  	:= $(wildcard $(PROJECT)/src/*.cpp $(PROJECT)/src/base/*.cpp $(PROJECT)/src/core/*.cpp)
override SRC := $(filter-out $(MAIN), $(SRC))
OBJECT  := $(patsubst %.cpp, %.o, $(SRC))
BIN 	:= $(PROJECT)/bin
TARGET  := webd
CXX     := g++
LIBS    := -lpthread
INCLUDE	:= -I ./usr/local/lib
CFLAGS  := -std=c++11 -g -pg -Wall -O3 -D_PTHREADS
CXXFLAGS:= $(CFLAGS)

all : $(BIN)/$(TARGET)

$(BIN)/$(TARGET) : $(OBJECT) $(PROJECT)/src/Main.o
	[ -e $(BIN) ] || mkdir $(BIN)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS)

clean :
	find . -name '*.o' | xargs rm -f
	find . -name $(TARGET) | xargs rm -f

common :
	find . -name '*.o' | xargs rm -f

debug :
	@echo mkdir Debug
	[ -e $(PROJECT)/debug ] || mkdir -p $(PROJECT)/debug
	cp -r $(PROJECT)/src $(PROJECT)/debug
	cp -r $(BIN) $(PROJECT)/debug
	@echo copy webd.service
	cp -r $(PROJECT)/webd.service $(PROJECT)/debug
	@echo ln -s Target oo
	ln -s $(PROJECT)/conf $(PROJECT)/debug/conf
	ln -s $(PROJECT)/www $(PROJECT)/debug/www
	mv $(PROJECT)/debug ../
