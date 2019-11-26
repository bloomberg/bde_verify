# Makefile                                                       -*-makefile-*-

PREFIX     ?= $(firstword $(wildcard /opt/bb /usr))
LLVMDIR    ?= $(PREFIX)

SYSTEM     := $(shell uname -s)
ifeq ($(notdir $(CXX)),g++)
GCCDIR     ?= $(patsubst %/bin/g++,%,$(shell which $(CXX)))
else
GCCDIR     ?= $(patsubst %/bin/g++,%,$(shell which g++))
endif

CSABASE     = csabase
LCB         = bde-verify
LIBCSABASE  = lib$(LCB).a

# Set up location of clang headers and libraries needed by bde_verify.
INCFLAGS   += -I$(LLVMDIR)/include

CXXFLAGS   += -m64 -fPIC -O3 -std=c++11
CXXFLAGS   += -Wall -Wno-unused-local-typedefs -Wno-comment
CXXFLAGS   += -Wno-ignored-attributes -Wno-unused-function
CXXFLAGS   += -Wno-deprecated-declarations
INCFLAGS   += -I$(PREFIX)/include -I/opt/swt/include

# Set up locations and flags for the compiler that will build bde_verify.
ifeq ($(notdir $(CXX)),clang++)
    CXXFLAGS   += --gcc-toolchain=$(GCCDIR) -Wno-mismatched-tags
endif
ifeq ($(notdir $(CXX)),g++)
    CXXFLAGS   += -Wno-maybe-uninitialized
endif

# _GLIBCXX_USE_CXX11_ABI must match how the clang libraries were built
ifeq ($(SYSTEM),Linux)
    AR          = /usr/bin/ar
    CXXFLAGS   += -D_GLIBCXX_USE_CXX11_ABI=0
else ifeq ($(SYSTEM),SunOS)
    AR          = /usr/ccs/bin/ar
    CXXFLAGS   += -D_GLIBCXX_USE_CXX11_ABI=1
    CXXFLAGS   += -DBYTE_ORDER=BIG_ENDIAN
endif

CXXFLAGS   += -DBDE_VERIFY_VERSION='"1.3.10"'

OBJ        := $(SYSTEM)-$(notdir $(CXX))

VERBOSE ?= @

# -----------------------------------------------------------------------------

CXXFILES =                                                 \
        csabase_abstractvisitor.cpp                        \
        csabase_analyse.cpp                                \
        csabase_analyser.cpp                               \
        csabase_attachments.cpp                            \
        csabase_checkregistry.cpp                          \
        csabase_clang.cpp                                  \
        csabase_config.cpp                                 \
        csabase_debug.cpp                                  \
        csabase_diagnostic_builder.cpp                     \
        csabase_diagnosticfilter.cpp                       \
        csabase_filenames.cpp                              \
        csabase_format.cpp                                 \
        csabase_location.cpp                               \
        csabase_ppobserver.cpp                             \
        csabase_registercheck.cpp                          \
        csabase_report.cpp                                 \
        csabase_tool.cpp                                   \
        csabase_util.cpp                                   \
        csabase_visitor.cpp                                \

# -----------------------------------------------------------------------------

DEFFLAGS += -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS
INCFLAGS += -I.
CXXFLAGS += -fno-common -fno-strict-aliasing -fno-exceptions -fno-rtti

OFILES = $(CXXFILES:%.cpp=$(OBJ)/%.o)

default: $(CSABASE)

$(CSABASE): $(OBJ)/$(LIBCSABASE)

$(OBJ)/$(LIBCSABASE): $(OFILES)
	@echo creating library
	$(VERBOSE) $(AR) cr $@ $(OFILES)

$(OBJ)/%.o: %.cpp
	@if [ ! -d $(@D) ]; then mkdir -p $(@D); fi
	@echo compiling $(@:$(OBJ)/%.o=%.cpp)
	$(VERBOSE) $(CXX) $(INCFLAGS) $(DEFFLAGS) $(CXXFLAGS) \
                          -o $@ -c $(@:$(OBJ)/%.o=%.cpp)

.PHONY: install install-bin install-dev

install: install-bin install-dev

install-bin:

install-dev: $(OBJ)/$(LIBCSABASE)
	mkdir -p $(DESTDIR)/include/bde-verify/utils
	cp csabase_*.h $(DESTDIR)/include/bde-verify
	cp utils/*.hpp $(DESTDIR)/include/bde-verify/utils
	mkdir -p $(DESTDIR)/lib64
	cp $(OBJ)/$(LIBCSABASE) $(DESTDIR)/lib64

.PHONY: clean

clean:
	@echo cleaning files
	$(VERBOSE) $(RM) -rf $(OBJ)

# -----------------------------------------------------------------------------

.PHONY: depend

depend $(OBJ)/make.depend:
	@if [ ! -d $(OBJ) ]; then mkdir $(OBJ); fi
	@echo analysing dependencies
	$(VERBOSE) $(CXX) $(INCFLAGS) $(DEFFLAGS) -M $(CXXFILES)                  \
            $(filter-out -Wno-unused-local-typedefs, $(CXXFLAGS))             \
        | perl -pe 's[^(?! )][$(OBJ)/]' > $(OBJ)/make.depend

ifneq "$(MAKECMDGOALS)" "clean"
    include $(OBJ)/make.depend
endif

## ----------------------------------------------------------------------------
## Copyright (C) 2014 Bloomberg Finance L.P.
##
## Licensed under the Apache License, Version 2.0 (the "License");
## you may not use this file except in compliance with the License.
## You may obtain a copy of the License at
##
##     http://www.apache.org/licenses/LICENSE-2.0
##
## Unless required by applicable law or agreed to in writing, software
## distributed under the License is distributed on an "AS IS" BASIS,
## WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
## See the License for the specific language governing permissions and
## limitations under the License.
## ----------------------------- END-OF-FILE ----------------------------------
