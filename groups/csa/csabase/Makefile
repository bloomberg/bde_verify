#   Makefile                                                     -*-makefile-*-
#  ----------------------------------------------------------------------------
#  Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
#  Modified  2013 Hyman Rosen (hrosen4@bloomberg.net)
#  Distributed under the Boost Software License, Version 1.0. (See file  
#  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
#  ----------------------------------------------------------------------------
# $Id$

CSABASE  = csabase
LIBCSABASE = libcsabase.a

default: $(CSABASE)

SHELL    = /opt/swt/bin/bash

SYSTEM   = $(shell uname -s)

COMPILER = gcc
#COMPILER = clang

STD      = CXX2011

ifeq ($(SYSTEM),Linux)
BB = /bb/build/share/packages/refroot/amd64/unstable/bb
    ifeq    ($(COMPILER),gcc)
VERSION  = 4.8.1
CCDIR    = /opt/swt/install/gcc-$(VERSION)
CXX      = $(CCDIR)/bin/g++
         ifeq        ($(STD),CXX2011)
CXXFLAGS += -std=c++0x
         endif
LINK     = $(CXX)
AR       = /usr/bin/ar
CXXFLAGS += -Wno-unused-local-typedefs
    endif
    ifeq ($(COMPILER),clang)
VERSION  = 3.4
CCDIR    = /home/hrosen4/mbig/llvm-$(VERSION)/install-$(SYSTEM)
CXX      = $(CCDIR)/bin/clang++
        ifeq    ($(STD),CXX2011)
CXXFLAGS += -std=c++0x
        else
CXXFLAGS += -Wno-c++11-extensions
        endif
LINK     = $(CXX)
    endif
endif
ifeq ($(SYSTEM),SunOS)
BB = /bb/build/share/packages/refroot/solaris10-sparc/unstable/bb
    ifeq    ($(COMPILER),gcc)
VERSION  = 4.8.1
CCDIR    = /opt/swt/install/gcc-$(VERSION)
CXX      = $(CCDIR)/bin/g++
         ifeq        ($(STD),CXX2011)
CXXFLAGS += -m64 -pthreads -mno-faster-structs
CFLAGS   += -m64 -pthreads -mno-faster-structs
CXXFLAGS += -std=c++0x
         endif
LINK     = $(CXX)
AR       = /usr/ccs/bin/ar
CXXFLAGS += -Wno-unused-local-typedefs
    endif
endif

OBJ      = $(SYSTEM)-$(COMPILER)-$(VERSION)

LLVM     = /home/hrosen4/mbig/llvm-3.4/install-$(SYSTEM)
INCFLAGS += -I$(LLVM)/include

#VERBOSE  =
VERBOSE  = @

#  ----------------------------------------------------------------------------

CXXFILES =                                                 \
        csabase_abstractvisitor.cpp                        \
        csabase_analyse.cpp                                \
        csabase_analyser.cpp                               \
        csabase_attachments.cpp                            \
        csabase_checkregistry.cpp                          \
        csabase_config.cpp                                 \
        csabase_debug.cpp                                  \
        csabase_diagnosticfilter.cpp                       \
        csabase_filenames.cpp                              \
        csabase_format.cpp                                 \
        csabase_location.cpp                               \
        csabase_ppobserver.cpp                             \
        csabase_registercheck.cpp                          \
        csabase_tool.cpp                                   \
        csabase_util.cpp                                   \
        csabase_visitor.cpp                                \

# -----------------------------------------------------------------------------

#DEBUG    = on
DEBUG    = off

REDIRECT = $(VERBOSE:@=>/dev/null 2>&1)

#DEFFLAGS += -D_DEBUG
#DEFFLAGS += -D_GNU_SOURCE
DEFFLAGS += -D__STDC_LIMIT_MACROS
DEFFLAGS += -D__STDC_CONSTANT_MACROS
INCFLAGS += -I.
CXXFLAGS += -g -fno-common -fno-strict-aliasing -fno-exceptions -fno-rtti

OFILES = $(CXXFILES:%.cpp=$(OBJ)/%.o)

$(CSABASE): $(OBJ)/$(LIBCSABASE)

$(OBJ)/$(LIBCSABASE): $(OFILES)
	@echo creating library
	$(VERBOSE) $(AR) cr $@ $(OFILES)

$(OBJ)/%.o: %.cpp
	@if [ ! -d $(@D) ]; then scripts/mkdirhier $(@D); fi
	@echo compiling $(@:$(OBJ)/%.o=%.cpp)
	$(VERBOSE) $(CXX) $(INCFLAGS) $(DEFFLAGS) $(CXXFLAGS) $(WARNFLAGS) \
                          -o $@ -c $(@:$(OBJ)/%.o=%.cpp)

clean:
	$(RM) $(OFILES)
	$(RM) $(OBJ)/$(LIBCSABASE)
	$(RM) $(OBJ)/make.depend
	$(RM) -r $(OBJ)
	$(RM) mkerr olderr *~

# -----------------------------------------------------------------------------

depend $(OBJ)/make.depend:
	@if [ ! -d $(OBJ) ]; then mkdir $(OBJ); fi
	@echo analysing dependencies
	$(VERBOSE) $(CXX) $(INCFLAGS) $(DEFFLAGS) -M $(CXXFILES) \
		| perl -pe 's[^(?! )][$(OBJ)/]' > $(OBJ)/make.depend

           include $(OBJ)/make.depend
