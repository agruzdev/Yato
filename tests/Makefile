
CXX = g++
CXX_FLAGS = -std=c++14 -Wall -O1 -o YatoTests.exe 

CXX_FLAGS += -IC:/gtest/include
CXX_FLAGS += -I../include

LDFLAGS  = -LC:/gtest/lib 
LDFLAGS += -lgtest
LDFLAGS += -lgtest_main

SRC = $(wildcard ./*.cpp)
    
all: 
	$(CXX) $(CXX_FLAGS) $(SRC) $(LDFLAGS) 
    
    