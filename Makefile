#!/usr/bin/env make

TTDDQ  := ${PWD}
MACROS := $(TTDDQ)/macros

ROOTCONFIG := $(shell which root-config)
CXX         = $(shell which g++)

INC  := $(MACROS)/BeamTimingData.cxx
LIBS := $(shell $(ROOTCONFIG) --cflags --libs)
EXE  := BeamTimingData.exe

.PHONY: all clean 
all: $(EXE)

# BeamTimingData.exe
#############################################
BeamTimingData.exe: $(INC)
	$(CXX) -o $(MACROS)/$@ $^ $(LIBS)
#############################################

# clean
#########################
clean:
	rm $(MACROS)/*exe
#########################
