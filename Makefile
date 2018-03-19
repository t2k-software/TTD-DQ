#!/usr/bin/env make

TTDDQ  := ${PWD}
MACROS := $(TTDDQ)/macros
SRC    := $(TTDDQ)/src
VPATH  := $(TTDDQ):$(MACROS):$(SRC)

ROOTCONFIG := $(shell which root-config)
CXX         = $(shell which g++)

CFLAGS   := $(shell $(ROOTCONFIG) --cflags)
CXXFLAGS := $(CFLAGS)

LIBS     := $(shell $(ROOTCONFIG) --glibs)
INC      := -I$(SRC)

OBS  := BeamTimingData.o
EXE  := BeamTiming.exe TTD_DQ.exe

.PHONY: all clean 
all: $(OBS) $(EXE)

#############################################
%.o: %.cxx
	$(CXX) $(CXXFLAGS) -c $<
#############################################

#############################################
BeamTiming.exe: $(OBS)
	$(CXX) $(CXXFLAGS) $^ $(LIBS) $(INC) app/BeamTiming.cxx -o $@
#############################################

TTD_DQ.exe:
	$(CXX) $(CXXFLAGS) app/TTD_DQ.cxx $(LIBS) $(INC) -o $@

# clean
#########################
clean:
	rm *exe *o
#########################
