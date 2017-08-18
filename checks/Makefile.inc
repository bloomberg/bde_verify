# Makefile.inc                                                   -*-makefile-*-

VERBOSE    ?= @
SHELL      ?= bash
SYSTEM      = $(shell uname -s)
COMPILER   ?= g++
BDEVERIFY  ?= $(DESTDIR)/bin/bde_verify
EXE        ?= $(DESTDIR)/libexec/bde-verify/bde_verify_bin

ALL ?= off

define CHECKARGS
    -cc=$(CXX)                                                                \
    -exe=$(EXE)                                                               \
    -nodefdef                                                                 \
    -config=/dev/null                                                         \
    -cl='namespace bde_verify'                                                \
    -cl='all $(ALL)'                                                          \
    $(foreach N,$(CHECKNAME),-cl='check $(N) on')                             \
    -Wno-string-plus-int                                                      \
    -std=c++11                                                                \
    -D_X86INTRIN_H_INCLUDED                                                   \
    -D__USE_XOPEN2K8                                                          \
    -I $(BDE_VERIFY_DIR)/checks/include                                       \
    $(BDE_VERIFY_ARGS)
endef

define CHECK
	$(VERBOSE) $(BDEVERIFY) $(CHECKARGS) $(basename $@) 2>&1 | \
        diff -q - *.exp >/dev/null && echo OK $(basename $@) || \
    $(BDEVERIFY) $(CHECKARGS) $(basename $@) 2>&1 | \
        diff - *.exp
endef

define RUN
	$(VERBOSE) $(BDEVERIFY) $(CHECKARGS) -fcolor-diagnostics $(basename $@)
endef

CHECKFILES = $(patsubst %,%.check,$(FILES))
RUNFILES   = $(patsubst %,%.run,$(FILES))

.PHONY: run check $(CHECKFILES) $(RUNFILES)

check: $(CHECKFILES)

$(CHECKFILES):
	$(CHECK)

run: $(RUNFILES)

$(RUNFILES):
	$(RUN)

.EXPORT_ALL_VARIABLES:

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
