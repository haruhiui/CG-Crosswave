CXX			:= g++
CXX_FLAGS   := -g -std=c++17 #-Wextra -Wall

SRC			:= src
INCLUDE     := ./include
LIB			:= ./lib

LIBRARIES	:= -lglad -lglfw3dll
EXECUTABLE	:= main

.PHONY : all run 

all : ./$(EXECUTABLE)

run : all
	./$(EXECUTABLE)

# $(EXECUTABLE) : $(SRC)/*.cpp
$(EXECUTABLE) : $(SRC)/$(TARGET) 
	$(CXX) $(CXX_FLAGS) -I $(INCLUDE) -L $(LIB) $^ -o $@ $(LIBRARIES)
