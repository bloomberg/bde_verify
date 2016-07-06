# Makefile                                                       -*-makefile-*-

PREFIX     ?= $(firstword $(wildcard /opt/bb /usr))
LLVMDIR    ?= $(PREFIX)

SYSTEM     := $(shell uname -s)

ifeq ($(notdir $(CXX)),g++)
GCCDIR     ?= $(patsubst %/bin/g++,%,$(shell which $(CXX)))
else
GCCDIR     ?= $(patsubst %/bin/g++,%,$(shell which g++))
endif

ifeq ($(notdir $(CXX)),clang++)
CLANG      ?= $(CXX)
else
CLANG      ?= $(shell which clang++)
endif

CLANG_RES  ?= $(shell \
      $(CLANG) -xc++ -E -v /dev/null 2>&1 | \
      grep 'resource-dir' | \
      sed 's/.*resource-dir *//;s/^\([^"][^ ]*\).*/\1/;s/^["]\([^"]*\)".*/\1/')

TARGET      = bde_verify_bin
CSABASE     = csabase
LCB         = bde-verify
LIBCSABASE  = lib$(LCB).a
CSABASEDIR  = groups/csa/csabase

# Set up location of clang headers and libraries needed by bde_verify.
INCFLAGS   += -I$(LLVMDIR)/include
LDFLAGS    += -L$(CSABASEDIR)/$(OBJ)

CXXFLAGS   += -m64 -std=c++11
CXXFLAGS   += -Wall -Wno-unused-local-typedefs

CXXFLAGS   += -DSPELL_CHECK=1
INCFLAGS   += -I$(PREFIX)/include -I/opt/swt/include

# Set up locations and flags for the compiler that will build bde_verify.
ifeq ($(notdir $(CXX)),clang++)
    CXXFLAGS   += --gcc-toolchain=$(GCCDIR) -Wno-mismatched-tags
endif

ifeq ($(SYSTEM),Linux)
    AR          = /usr/bin/ar
    LIBDIRS     = $(GCCDIR)/lib64                                             \
                  $(LLVMDIR)/lib64                                            \
                  $(PREFIX)/lib64                                             \
                  /opt/swt/lib64                                              \
                  /usr/lib64
    LDFLAGS    += -Wl,--enable-new-dtags
    LDFLAGS    += $(foreach L,$(LIBDIRS),-Wl,-L$(L),-rpath,$(L))
else ifeq ($(SYSTEM),SunOS)
    AR          = /usr/ccs/bin/ar
    CXXFLAGS   += -DBYTE_ORDER=BIG_ENDIAN
    LIBDIRS     = $(GCCDIR)/lib/sparcv9                                       \
                  $(LLVMDIR)/lib64                                            \
                  $(PREFIX)/lib64                                             \
                  /opt/swt/lib64                                              \
                  /usr/lib/sparcv9
    LDFLAGS    += $(foreach L,$(LIBDIRS),-Wl,-L$(L),-R,$(L))
    EXTRALIBS  += -lrt
    ifneq (,$(wildcard $(foreach L,$(LIBDIRS),$(L)/libtinfo.so)))
        EXTRALIBS += -ltinfo
    endif
    ifneq (,$(wildcard $(foreach L,$(LIBDIRS),$(L)/libmalloc.so)))
        EXTRALIBS += -lmalloc
    endif
endif

OBJ        := $(SYSTEM)-$(notdir $(CXX))
DESTDIR    ?= $(CURDIR)/$(OBJ)

VERBOSE ?= @

#  ----------------------------------------------------------------------------

CXXFILES =                                                                    \
        groups/csa/csaaq/csaaq_cppinexternc.cpp                               \
        groups/csa/csaaq/csaaq_freefunctionsdepend.cpp                        \
        groups/csa/csaaq/csaaq_friendsinheaders.cpp                           \
        groups/csa/csaaq/csaaq_globaldata.cpp                                 \
        groups/csa/csaaq/csaaq_includeinexternc.cpp                           \
        groups/csa/csaaq/csaaq_inentns.cpp                                    \
        groups/csa/csaaq/csaaq_runtimeinit.cpp                                \
        groups/csa/csaaq/csaaq_transitiveincludes.cpp                         \
        groups/csa/csabde/csabde_tool.cpp                                     \
        groups/csa/csabbg/csabbg_allocatorforward.cpp                         \
        groups/csa/csabbg/csabbg_allocatornewwithpointer.cpp                  \
        groups/csa/csabbg/csabbg_assertassign.cpp                             \
        groups/csa/csabbg/csabbg_bslovrdstl.cpp                               \
        groups/csa/csabbg/csabbg_bslstdstring.cpp                             \
        groups/csa/csabbg/csabbg_classsections.cpp                            \
        groups/csa/csabbg/csabbg_cmporder.cpp                                 \
        groups/csa/csabbg/csabbg_deprecated.cpp                               \
        groups/csa/csabbg/csabbg_enumvalue.cpp                                \
        groups/csa/csabbg/csabbg_functioncontract.cpp                         \
        groups/csa/csabbg/csabbg_midreturn.cpp                                \
        groups/csa/csabbg/csabbg_membernames.cpp                              \
        groups/csa/csabbg/csabbg_testdriver.cpp                               \
        groups/csa/csafmt/csafmt_banner.cpp                                   \
        groups/csa/csafmt/csafmt_comments.cpp                                 \
        groups/csa/csafmt/csafmt_headline.cpp                                 \
        groups/csa/csafmt/csafmt_indent.cpp                                   \
        groups/csa/csafmt/csafmt_longlines.cpp                                \
        groups/csa/csafmt/csafmt_nonascii.cpp                                 \
        groups/csa/csafmt/csafmt_whitespace.cpp                               \
        groups/csa/csamisc/csamisc_anonymousnamespaceinheader.cpp             \
        groups/csa/csamisc/csamisc_arrayargument.cpp                          \
        groups/csa/csamisc/csamisc_arrayinitialization.cpp                    \
        groups/csa/csamisc/csamisc_boolcomparison.cpp                         \
        groups/csa/csamisc/csamisc_charclassrange.cpp                         \
        groups/csa/csamisc/csamisc_charvsstring.cpp                           \
        groups/csa/csamisc/csamisc_constantreturn.cpp                         \
        groups/csa/csamisc/csamisc_contiguousswitch.cpp                       \
        groups/csa/csamisc/csamisc_cstylecastused.cpp                         \
        groups/csa/csamisc/csamisc_donotuseendl.cpp                           \
        groups/csa/csamisc/csamisc_dumpast.cpp                                \
        groups/csa/csamisc/csamisc_funcalpha.cpp                              \
        groups/csa/csamisc/csamisc_hashptr.cpp                                \
        groups/csa/csamisc/csamisc_longinline.cpp                             \
        groups/csa/csamisc/csamisc_memberdefinitioninclassdefinition.cpp      \
        groups/csa/csamisc/csamisc_movablerefref.cpp                          \
        groups/csa/csamisc/csamisc_namespacetags.cpp                          \
        groups/csa/csamisc/csamisc_opvoidstar.cpp                             \
        groups/csa/csamisc/csamisc_spellcheck.cpp                             \
        groups/csa/csamisc/csamisc_strictaliasing.cpp                         \
        groups/csa/csamisc/csamisc_stringadd.cpp                              \
        groups/csa/csamisc/csamisc_swapab.cpp                                 \
        groups/csa/csamisc/csamisc_swapusing.cpp                              \
        groups/csa/csamisc/csamisc_thatwhich.cpp                              \
        groups/csa/csamisc/csamisc_thrownonstdexception.cpp                   \
        groups/csa/csamisc/csamisc_unnamed_temporary.cpp                      \
        groups/csa/csamisc/csamisc_verifysameargumentnames.cpp                \
        groups/csa/csastil/csastil_externalguards.cpp                         \
        groups/csa/csastil/csastil_implicitctor.cpp                           \
        groups/csa/csastil/csastil_includeorder.cpp                           \
        groups/csa/csastil/csastil_leakingmacro.cpp                           \
        groups/csa/csastil/csastil_templatetypename.cpp                       \
        groups/csa/csastil/csastil_uppernames.cpp                             \
        groups/csa/csatr/csatr_componentheaderinclude.cpp                     \
        groups/csa/csatr/csatr_componentprefix.cpp                            \
        groups/csa/csatr/csatr_entityrestrictions.cpp                         \
        groups/csa/csatr/csatr_files.cpp                                      \
        groups/csa/csatr/csatr_friendship.cpp                                 \
        groups/csa/csatr/csatr_globalfunctiononlyinsource.cpp                 \
        groups/csa/csatr/csatr_globaltypeonlyinsource.cpp                     \
        groups/csa/csatr/csatr_groupname.cpp                                  \
        groups/csa/csatr/csatr_includeguard.cpp                               \
        groups/csa/csatr/csatr_nesteddeclarations.cpp                         \
        groups/csa/csatr/csatr_packagename.cpp                                \
        groups/csa/csatr/csatr_usingdeclarationinheader.cpp                   \
        groups/csa/csatr/csatr_usingdirectiveinheader.cpp                     \
        groups/csa/csaxform/csaxform_refactor.cpp                             \
        groups/csa/csaxform/csaxform_refactor_config.cpp                      \

# -----------------------------------------------------------------------------

DEFFLAGS += -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS
INCFLAGS += -I. -I$(CSABASEDIR) -Igroups/csa/csadep
CXXFLAGS += -fno-common -fno-strict-aliasing -fno-exceptions -fno-rtti

OFILES = $(CXXFILES:%.cpp=$(OBJ)/%.o)

LIBS     =    -l$(LCB)                                                        \
              -lLLVMX86Info                                                   \
              -lLLVMSparcInfo                                                 \
              -lclangFrontendTool                                             \
              -lclangCodeGen                                                  \
              -lLLVMIRReader                                                  \
              -lLLVMLinker                                                    \
              -lLLVMipo                                                       \
              -lLLVMX86AsmParser                                              \
              -lLLVMSparcAsmParser                                            \
              -lLLVMX86CodeGen                                                \
              -lLLVMSparcCodeGen                                              \
              -lLLVMSelectionDAG                                              \
              -lLLVMCodeGen                                                   \
              -lLLVMScalarOpts                                                \
              -lLLVMInstCombine                                               \
              -lLLVMVectorize                                                 \
              -lLLVMInstrumentation                                           \
              -lLLVMObjCARCOpts                                               \
              -lLLVMTransformUtils                                            \
              -lLLVMAnalysis                                                  \
              -lclangStaticAnalyzerFrontend                                   \
              -lclangRewriteFrontend                                          \
              -lclangARCMigrate                                               \
              -lclangFrontend                                                 \
              -lclangSerialization                                            \
              -lLLVMProfileData                                               \
              -lLLVMX86Desc                                                   \
              -lLLVMSparcDesc                                                 \
              -lLLVMObject                                                    \
              -lLLVMBitReader                                                 \
              -lLLVMTarget                                                    \
              -lLLVMAsmParser                                                 \
              -lLLVMBitWriter                                                 \
              -lLLVMAsmPrinter                                                \
              -lLLVMX86AsmPrinter                                             \
              -lLLVMSparcAsmPrinter                                           \
              -lLLVMX86Utils                                                  \
              -lLLVMCore                                                      \
              -lclangParse                                                    \
              -lLLVMMCParser                                                  \
              -lLLVMMCDisassembler                                            \
              -lclangSema                                                     \
              -lclangStaticAnalyzerCheckers                                   \
              -lclangStaticAnalyzerCore                                       \
              -lclangASTMatchers                                              \
              -lclangEdit                                                     \
              -lclangAnalysis                                                 \
              -lclangAST                                                      \
              -lclangToolingCore                                              \
              -lclangRewrite                                                  \
              -lclangLex                                                      \
              -lclangDriver                                                   \
              -lclangBasic                                                    \
              -lLLVMMC                                                        \
              -lLLVMOption                                                    \
              -lLLVMSupport                                                   \
              -lncurses                                                       \
              -lpthread                                                       \
              -ldl                                                            \
              -lz                                                             \
              -laspell                                                        \
              $(EXTRALIBS)

default: $(OBJ)/$(TARGET)

.PHONY: csabase

$(CSABASEDIR)/$(OBJ)/$(LIBCSABASE): csabase
	$(VERBOSE) $(MAKE) -C $(CSABASEDIR)

$(OBJ)/$(TARGET): $(CSABASEDIR)/$(OBJ)/$(LIBCSABASE) $(OFILES)
	@echo linking executable
	$(VERBOSE) $(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@.$$ $(OFILES) $(LIBS)
	mv $@.$$ $@

$(OBJ)/%.o: %.cpp
	@if [ ! -d $(@D) ]; then mkdir -p $(@D); fi
	@echo compiling $(@:$(OBJ)/%.o=%.cpp)
	$(VERBOSE) $(CXX) $(INCFLAGS) $(DEFFLAGS) $(CXXFLAGS) \
                          -o $@ -c $(@:$(OBJ)/%.o=%.cpp)

.PHONY: install install-bin install-dev

install: install-bin install-dev

install-bin: $(OBJ)/$(TARGET)
	$(VERBOSE) $(MAKE) -C $(CSABASEDIR) DESTDIR=$(DESTDIR) install-bin
	mkdir -p $(DESTDIR)/libexec/bde-verify
	cp $(OBJ)/$(TARGET) $(DESTDIR)/libexec/bde-verify
	mkdir -p $(DESTDIR)/bin
	cp scripts/bde_verify scripts/bb_cppverify scripts/check_bos $(DESTDIR)/bin
	mkdir -p $(DESTDIR)/etc/bde-verify
	cp bde.cfg bde_verify.cfg bb_cppverify.cfg $(DESTDIR)/etc/bde-verify
	mkdir -p $(DESTDIR)/include/bde-verify/clang
	cp -r $(CLANG_RES)/include $(DESTDIR)/include/bde-verify/clang

install-dev:
	$(VERBOSE) $(MAKE) -C $(CSABASEDIR) DESTDIR=$(DESTDIR) install-dev

.PHONY: clean

clean:
	@echo cleaning files
	$(VERBOSE) $(MAKE) -C $(CSABASEDIR) clean
	$(VERBOSE) $(RM) -rf $(OBJ)

gh-pages:
	$(VERBOSE) $(MAKE) -C doc \
    SPHINXOPTS='-t bb_cppverify' BUILDDIR='../bb_cppverify_build' clean html
	$(VERBOSE) $(MAKE) -C doc \
    SPHINXOPTS='-t bde_verify'   BUILDDIR='../bde_verify_build'   clean html
	cp doc/index.html .
	# git checkout gh-pages
	# rm -rf Linux-*g++ $(CSABASEDIR)/Linux-*g++
	# rm -rf SunOS-*g++ $(CSABASEDIR)/SunOS-*g++
	# git add -A
	# git commit -m "Generate gh-pages"

# -----------------------------------------------------------------------------

BDE_VERIFY_DIR := $(shell /bin/pwd)

# All the Makefiles below the checks directory.
define ALLM :=
    $(shell find checks -name Makefile | sort)
endef

# All the Makefiles below both the checks and CURRENT directory.
define CURM :=
    $(shell find checks -regex 'checks\(/.*\)?/$(CURRENT)\(/.*\)?/Makefile' | \
            sort)
endef

CNAMES   := $(foreach N,$(ALLM),$(patsubst %,%.check,$(N)))
CCURNAME := $(foreach N,$(CURM),$(patsubst %,%.check,$(N)))
RNAMES   := $(foreach N,$(ALLM),$(patsubst %,%.run,$(N)))
RCURNAME := $(foreach N,$(CURM),$(patsubst %,%.run,$(N)))

.PHONY: check-current check $(CNAMES) run-current run $(RNAMES)

check: install $(CNAMES)
check-current: install $(CCURNAME)

$(CNAMES):
	$(VERBOSE) $(MAKE) DESTDIR=$(DESTDIR) -C $(@D) \
            -k --no-print-directory check

run: install $(RNAMES)
run-current: install $(RCURNAME)

$(RNAMES):
	$(VERBOSE) $(MAKE) DESTDIR=$(DESTDIR) -C $(@D) -k --no-print-directory run

# -----------------------------------------------------------------------------

.PHONY: depend

depend $(OBJ)/make.depend:
	@if [ ! -d $(OBJ) ]; then mkdir $(OBJ); fi
	@echo analysing dependencies
	$(VERBOSE) $(CXX) $(INCFLAGS) $(DEFFLAGS) -M $(CXXFILES)                  \
            $(filter-out -Wno-unused-local-typedefs, $(CXXFLAGS))             \
        | perl -pe 's[^(?! )][$(OBJ)/]' > $(OBJ)/make.depend

ifneq "$(MAKECMDGOALS)" "clean"
ifneq "$(MAKECMDGOALS)" "gh-pages"
    include $(OBJ)/make.depend
endif
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
