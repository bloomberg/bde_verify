#   Makefile                                                     -*-makefile-*-
#  ----------------------------------------------------------------------------
#  Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
#  Modified  2013 Hyman Rosen (hrosen4@bloomberg.net)
#  Distributed under the Boost Software License, Version 1.0. (See file  
#  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
#  ----------------------------------------------------------------------------
# $Id$

TARGET   = bde_verify
CSABASE  = csabase
LIBCSABASE = libcsabase.a
CSABASEDIR = groups/csa/csabase

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
LDFLAGS  += -Wl,-rpath,$(CCDIR)/lib64
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
ASPELL   = /opt/swt/install/aspell-0.60.6.1-64
CXXFLAGS += -DSPELL_CHECK=1
INCFLAGS += -I$(ASPELL)/include
LDFLAGS  += -L$(ASPELL)/lib64 -laspell
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
LDFLAGS  += -m64 -pthreads -mno-faster-structs
CXXFLAGS += -std=c++0x
         endif
LINK     = $(CXX)
AR       = /usr/ccs/bin/ar
LDFLAGS  += -Wl,-L,$(CCDIR)/lib/sparcv9 -Wl,-R,$(CCDIR)/lib/sparcv9
CXXFLAGS += -Wno-unused-local-typedefs
    endif
ASPELL   = /opt/swt/install/aspell-0.60.6.1-64
CXXFLAGS += -DSPELL_CHECK=1
INCFLAGS += -I$(ASPELL)/include
LDFLAGS  += -Wl,-L,$(ASPELL)/lib64 -Wl,-R,$(ASPELL)/lib64 -laspell -L$(OBJ)
EXTRALIBS += -lrt
EXTRALIBS += -lmalloc
endif

OBJ      = $(SYSTEM)-$(COMPILER)-$(VERSION)

LLVM     = /home/hrosen4/mbig/llvm-3.4/install-$(SYSTEM)
INCFLAGS += -I$(LLVM)/include
LDFLAGS  += -L$(LLVM)/lib -L$(CSABASEDIR)/$(OBJ)

#export VERBOSE =
export VERBOSE ?= @

#  ----------------------------------------------------------------------------

TSTCXXFILES =                                                                 \
        groups/csa/csabde/csabde_tool.cpp                                     \
        groups/csa/csabbg/csabbg_allocatorforward.cpp                         \
        groups/csa/csabbg/csabbg_allocatornewwithpointer.cpp                  \
        groups/csa/csabbg/csabbg_enumvalue.cpp                                \
        groups/csa/csabbg/csabbg_functioncontract.cpp                         \
        groups/csa/csabbg/csabbg_midreturn.cpp                                \
        groups/csa/csabbg/csabbg_testdriver.cpp                               \
        groups/csa/csafmt/csafmt_banner.cpp                                   \
        groups/csa/csafmt/csafmt_comments.cpp                                 \
        groups/csa/csafmt/csafmt_headline.cpp                                 \
        groups/csa/csafmt/csafmt_indent.cpp                                   \
        groups/csa/csafmt/csafmt_longlines.cpp                                \
        groups/csa/csafmt/csafmt_nonascii.cpp                                 \
        groups/csa/csafmt/csafmt_whitespace.cpp                               \
        groups/csa/csamisc/csamisc_anonymousnamespaceinheader.cpp             \
        groups/csa/csamisc/csamisc_arrayinitialization.cpp                    \
        groups/csa/csamisc/csamisc_boolcomparison.cpp                         \
        groups/csa/csamisc/csamisc_charvsstring.cpp                           \
        groups/csa/csamisc/csamisc_constantreturn.cpp                         \
        groups/csa/csamisc/csamisc_contiguousswitch.cpp                       \
        groups/csa/csamisc/csamisc_cstylecastused.cpp                         \
        groups/csa/csamisc/csamisc_funcalpha.cpp                              \
        groups/csa/csamisc/csamisc_hashptr.cpp                                \
        groups/csa/csamisc/csamisc_longinline.cpp                             \
        groups/csa/csamisc/csamisc_memberdefinitioninclassdefinition.cpp      \
        groups/csa/csamisc/csamisc_opvoidstar.cpp                             \
        groups/csa/csamisc/csamisc_spellcheck.cpp                             \
        groups/csa/csamisc/csamisc_stringadd.cpp                              \
        groups/csa/csamisc/csamisc_swapab.cpp                                 \
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

TODO =                                                                        \
        groups/csa/csadep/csadep_dependencies.cpp                             \
        groups/csa/csadep/csadep_types.cpp                                    \
        groups/csa/csamisc/csamisc_calls.cpp                                  \
        groups/csa/csamisc/csamisc_includeguard.cpp                           \
        groups/csa/csamisc/csamisc_selfinitialization.cpp                     \
        groups/csa/csamisc/csamisc_superfluoustemporary.cpp                   \

# -----------------------------------------------------------------------------

#DEFFLAGS += -D_DEBUG
#DEFFLAGS += -D_GNU_SOURCE
DEFFLAGS += -D__STDC_LIMIT_MACROS
DEFFLAGS += -D__STDC_CONSTANT_MACROS
INCFLAGS += -I.
INCFLAGS += -I$(CSABASEDIR)
INCFLAGS += -Igroups/csa/csadep
CXXFLAGS += -g -fno-common -fno-strict-aliasing -fno-exceptions -fno-rtti
LDFLAGS += -g

TSTOFILES = $(TSTCXXFILES:%.cpp=$(OBJ)/%.o)

LIBS     =    -lcsabase                                                       \
              -lLLVMX86AsmParser                                              \
              -lclangFrontendTool                                             \
              -lclangCodeGen                                                  \
              -lLLVMIRReader                                                  \
              -lLLVMLinker                                                    \
              -lLLVMipo                                                       \
              -lLLVMX86CodeGen                                                \
              -lLLVMSparcCodeGen                                              \
              -lLLVMSelectionDAG                                              \
              -lLLVMAsmPrinter                                                \
              -lLLVMJIT                                                       \
              -lLLVMInterpreter                                               \
              -lLLVMCodeGen                                                   \
              -lLLVMScalarOpts                                                \
              -lLLVMInstrumentation                                           \
              -lLLVMInstCombine                                               \
              -lLLVMVectorize                                                 \
              -lclangRewriteFrontend                                          \
              -lclangARCMigrate                                               \
              -lclangStaticAnalyzerFrontend                                   \
              -lclangIndex                                                    \
              -lclangFormat                                                   \
              -lclangTooling                                                  \
              -lclangFrontend                                                 \
              -lclangDriver                                                   \
              -lLLVMObjCARCOpts                                               \
              -lLLVMTransformUtils                                            \
              -lLLVMipa                                                       \
              -lLLVMAnalysis                                                  \
              -lLLVMAsmParser                                                 \
              -lclangSerialization                                            \
              -lLLVMBitReader                                                 \
              -lLLVMBitWriter                                                 \
              -lLLVMTarget                                                    \
              -lLLVMExecutionEngine                                           \
              -lLLVMCore                                                      \
              -lLLVMDebugInfo                                                 \
              -lclangParse                                                    \
              -lLLVMMCParser                                                  \
              -lLLVMX86Desc                                                   \
              -lLLVMSparcDesc                                                 \
              -lLLVMX86Info                                                   \
              -lLLVMSparcInfo                                                 \
              -lLLVMX86AsmPrinter                                             \
              -lLLVMX86Utils                                                  \
              -lclangSema                                                     \
              -lclangStaticAnalyzerCheckers                                   \
              -lclangStaticAnalyzerCore                                       \
              -lclangDynamicASTMatchers                                       \
              -lclangASTMatchers                                              \
              -lclangEdit                                                     \
              -lclangAnalysis                                                 \
              -lclangRewriteCore                                              \
              -lclangAST                                                      \
              -lclangLex                                                      \
              -lclangBasic                                                    \
              -lLLVMMC                                                        \
              -lLLVMObject                                                    \
              -lLLVMOption                                                    \
              -lLLVMTableGen                                                  \
              -lLLVMSupport                                                   \
              -lpthread                                                       \
              -lcurses                                                        \
              -ldl                                                            \
              $(EXTRALIBS)

default: $(OBJ)/$(TARGET)

.PHONY: csabase

$(CSABASEDIR)/$(OBJ)/$(LIBCSABASE): csabase
	$(VERBOSE) $(MAKE) -s -C $(CSABASEDIR)

$(OBJ)/$(TARGET): $(CSABASEDIR)/$(OBJ)/$(LIBCSABASE) $(TSTOFILES)
	@echo linking executable
	$(VERBOSE) $(LINK) $(LDFLAGS) -o $@ $(TSTOFILES) $(LIBS)

$(OBJ)/%.o: %.cpp
	@if [ ! -d $(@D) ]; then scripts/mkdirhier $(@D); fi
	@echo compiling $(@:$(OBJ)/%.o=%.cpp)
	$(VERBOSE) $(CXX) $(INCFLAGS) $(DEFFLAGS) $(CXXFLAGS) $(WARNFLAGS) \
                          -o $@ -c $(@:$(OBJ)/%.o=%.cpp)

.PHONY: clean

clean:
	$(RM) $(TSTOFILES)
	$(RM) $(OBJ)/$(TARGET)
	$(RM) $(OBJ)/make.depend
	$(RM) -r $(OBJ)
	$(RM) mkerr olderr *~
	$(MAKE) -C $(CSABASEDIR) clean

# -----------------------------------------------------------------------------

export BDE_VERIFY_DIR := $(shell /bin/pwd)

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

check: $(OBJ)/$(TARGET) $(CNAMES)
check-current: $(OBJ)/$(TARGET) $(CCURNAME)

$(CNAMES):
	$(VERBOSE) $(MAKE) -s -C $(@D) check

run: $(OBJ)/$(TARGET) $(RNAMES)
run-current: $(OBJ)/$(TARGET) $(RCURNAME)

$(RNAMES):
	$(VERBOSE) $(MAKE) -s -C $(@D) run


# -----------------------------------------------------------------------------

depend $(OBJ)/make.depend:
	@if [ ! -d $(OBJ) ]; then mkdir $(OBJ); fi
	@echo analysing dependencies
	$(VERBOSE) $(CXX) $(INCFLAGS) $(DEFFLAGS) -M $(TSTCXXFILES) \
           | scripts/fixdepend $(OBJ) > $(OBJ)/make.depend

           include $(OBJ)/make.depend
