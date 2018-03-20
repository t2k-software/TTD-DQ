#!/usr/bin/env make

### AUTHOR: MATTHEW HOGAN
### EMAIL: hoganman@gmail.com
###

TTDDQ  := ${PWD}
MACROS := $(TTDDQ)/macros
SRC    := $(TTDDQ)/src
APP    := $(TTDDQ)/app
VPATH  := $(TTDDQ):$(MACROS):$(SRC):$(APP)

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

# compile .o objects
#############################################
%.o: %.cxx
	$(CXX) $(CXXFLAGS) $(INC) -c $<
#############################################

# BeamTiming executible
#############################################
BeamTiming.exe: $(OBS) BeamTiming.o
	$(CXX) $^ $(LIBS) -o $@
#############################################

# TTD-DQ checks PATH and environ for software
#############################################
TTD_DQ.exe: TTD_DQ.o
	$(CXX) $^ $(LIBS) -o $@
#############################################

# clean
#########################
clean:
	rm *exe *o
#########################
