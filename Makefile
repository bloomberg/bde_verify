#   Makefile                                                     -*-makefile-*-
#  ----------------------------------------------------------------------------
#  Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
#  Distributed under the Boost Software License, Version 1.0. (See file  
#  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
#  ----------------------------------------------------------------------------
# $Id$

default:  check-current
CLANGVER = 3.1
LLVM     = /opt/swt/install/llvm-$(CLANGVER)-64
COMPILER = g++

CURRENT  = csastil/csastil_implicitctor.t.cpp

#  ----------------------------------------------------------------------------

TARGET = coolyser

TSTCXXFILES +=                                                                \
        groups/csa/csabbg/csabbg_allocatorforward.cpp                         \
        groups/csa/csabbg/csabbg_allocatornewwithpointer.cpp                  \
        groups/csa/csafmt/csafmt_headline.cpp                                 \
        groups/csa/csatr/csatr_groupname.cpp                                  \
        groups/csa/csatr/csatr_componentprefix.cpp                            \
        groups/csa/csatr/csatr_packagename.cpp                                \
        groups/csa/csatr/csatr_files.cpp                                      \
        groups/csa/csatr/csatr_friendship.cpp                                 \
        groups/csa/csatr/csatr_globaltypeonlyinsource.cpp                     \
        groups/csa/csatr/csatr_globalfunctiononlyinsource.cpp                 \
        groups/csa/csatr/csatr_includeguard.cpp                               \
        groups/csa/csatr/csatr_componentheaderinclude.cpp                     \
        groups/csa/csatr/csatr_nesteddeclarations.cpp                         \
        groups/csa/csatr/csatr_usingdeclarationinheader.cpp                   \
        groups/csa/csatr/csatr_usingdirectiveinheader.cpp                     \
        groups/csa/csatr/csatr_entityrestrictions.cpp                         \
        groups/csa/csastil/csastil_implicitctor.cpp                           \
        groups/csa/csastil/csastil_templatetypename.cpp                       \
        groups/csa/csamisc/csamisc_charvsstring.cpp                           \
        groups/csa/csamisc/csamisc_arrayinitialization.cpp                    \
        groups/csa/csamisc/csamisc_anonymousnamespaceinheader.cpp             \
        groups/csa/csamisc/csamisc_boolcomparison.cpp                         \
        groups/csa/csamisc/csamisc_cstylecastused.cpp                         \
        groups/csa/csamisc/csamisc_constantreturn.cpp                         \
        groups/csa/csamisc/csamisc_contiguousswitch.cpp                       \
        groups/csa/csamisc/csamisc_memberdefinitioninclassdefinition.cpp      \
        groups/csa/csamisc/csamisc_thrownonstdexception.cpp                   \
        groups/csa/csamisc/csamisc_verifysameargumentnames.cpp                \
        groups/csa/csamisc/csamisc_stringadd.cpp                              \

TODO =                                                                        \
        groups/csa/csamisc/csamisc_calls.cpp                                  \
        groups/csa/csamisc/csamisc_selfinitialization.cpp                     \
        groups/csa/csamisc/csamisc_includeguard.cpp                           \
        groups/csa/csamisc/csamisc_superfluoustemporary.cpp                   \
        groups/csa/csadep/csadep_dependencies.cpp                             \
        groups/csa/csadep/csadep_types.cpp                                    \

LIBCXXFILES +=                                                                \
        groups/csa/csabase/csabase_abstractvisitor.cpp                        \
        groups/csa/csabase/csabase_analyser.cpp                               \
        groups/csa/csabase/csabase_attachments.cpp                            \
        groups/csa/csabase/csabase_checkregistry.cpp                          \
        groups/csa/csabase/csabase_config.cpp                                 \
        groups/csa/csabase/csabase_coolyse.cpp                                \
        groups/csa/csabase/csabase_debug.cpp                                  \
        groups/csa/csabase/csabase_diagnosticfilter.cpp                       \
        groups/csa/csabase/csabase_format.cpp                                 \
        groups/csa/csabase/csabase_location.cpp                               \
        groups/csa/csabase/csabase_ppobserver.cpp                             \
        groups/csa/csabase/csabase_registercheck.cpp                          \
        groups/csa/csabase/csabase_visitor.cpp                                \
        $(TSTCXXFILES)

# -----------------------------------------------------------------------------

SYSTEM   = $(shell uname -s)
ifeq ($(SYSTEM),Darwin)
  COMPILER = clang
endif
ECHON    = echo
# CLANGVER = SVN

ifeq ($(CLANGVER),2.9)
  CPPFLAGS += -DCLANG_29
endif
ifeq ($(CLANGVER),3.1)
  CPPFLAGS += -DCLANG_31
endif
ifeq ($(SYSTEM),Darwin)
  LLVM     = /opt/llvm
  ifeq ($(CLANGVER),2.9)
    LLVM     = /opt/llvm-2.9
  endif
  ifeq ($(CLANGVER),3.1)
    LLVM     = /opt/llvm-3.1
    PFLAGS   = -Wno-string-plus-int
  endif
  ifeq ($(CLANGVER),SVN)
    CPPFLAGS += -DCLANG_SVN
    CXXFLAGS += -fvisibility-inlines-hidden 
    LDLIBS   += -lclangEdit
    PFLAGS   = -Wno-string-plus-int
  endif
endif

CLANG    = $(LLVM)/bin/clang
SOSUFFIX = so
CXX      = g++
ifeq ($(COMPILER),clang)
CXX      = $(CLANG)
endif
LINK     = $(CXX)
OBJ      = $(SYSTEM)-$(COMPILER)-$(CLANGVER)
DEBUG    = off
ifeq ($(DEBUG),off)
  VERBOSE  = @
endif
REDIRECT = $(VERBOSE:@=>/dev/null 2>&1)

INCFLAGS = -I$(LLVM)/include -I.
DEFFLAGS = -D_DEBUG -D_GNU_SOURCE -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS
ifeq ($(STD),CXX2011)
  STDFLAGS = -std=c++0x -DCOOL_CXX2011
endif
CPPFLAGS += $(INCFLAGS) $(DEFFLAGS) $(STDFLAGS)
CPPFLAGS += -Igroups/csa/csabase -Igroups/csa/csadep
# PFLAGS   += -fdiagnostics-show-option
PFLAGS   += -fcxx-exceptions
CXXFLAGS += -g -fno-exceptions -fno-rtti -fno-common -fno-strict-aliasing
WARNFLAGS = \
        -Wcast-qual \
        -Wno-long-long \
        -Wall \
        -W \
        -Wno-unused-parameter \
        -Wno-overloaded-virtual \
        -Wwrite-strings
LDFLAGS = -L$(LLVM)/lib

ifeq ($(CLANGVER),3.1)
LDLIBS += -lclangEdit
endif
LDLIBS += \
        -lclangFrontend \
        -lclangDriver \
        -lclangSerialization \
        -lclangSema \
        -lclangAnalysis \
        -lclangParse \
        -lclangAST \
        -lclangLex \
        -lclangBasic \
        -lLLVMSupport \
        -lLLVMMC \
        -lpthread \
        -lm 

ifeq ($(SYSTEM),Linux)
  CXXFLAGS += -fpic
  LDFLAGS  += -shared
  LDLIBS   += -ldl
endif
ifeq ($(SYSTEM),SunOS)
  CXXFLAGS += -R -fpic
  LDFLAGS  += -Wl,-undefined -m64 -Wl,-G
endif
ifeq ($(SYSTEM),Darwin)
  SOSUFFIX = dylib
  ECHON    = /bin/echo -n
  LDFLAGS  += -Wl,-undefined,dynamic_lookup -dynamiclib
  LDFLAGS  += -mmacosx-version-min=10.6
endif

PLUGIN   = -cc1 -load $(OBJ)/$(TARGET).$(SOSUFFIX) \
           -plugin coolyse -plugin-arg-coolyse config=test.cfg
OFILES = $(LIBCXXFILES:%.cpp=$(OBJ)/%.o)
POSTPROCESS = sed -e 's/\([^:]*:[0-9][0-9]*\):[^:]*:/\1:0:/' \
            | sed -e '/\^/s/ //g' \
            | sed -e 's/~~~~~\(~*\)/~~~~~/g' \
            | sed -e '/^$$/d'

#EXPECT      = `echo $$f | sed -E 's/((test)|(\.[vt]))\.cpp$$/.exp/'`
EXPECT      = `echo $$f | \
               sed -e 's/test\.cpp$$/.exp/' | \
               sed -e 's/\.t.cpp$$/.exp/' | \
               sed -e 's/\.v.cpp$$/.exp/'`

# -----------------------------------------------------------------------------

check-current: $(OBJ)/$(TARGET).$(SOSUFFIX)
	$(CLANG) $(PLUGIN) \
	    -plugin-arg-coolyse debug-$(DEBUG) \
	    -plugin-arg-coolyse config=test.cfg \
	    -plugin-arg-coolyse tool=bdechk \
	    $(CPPFLAGS) $(PFLAGS) groups/csa/$(CURRENT)

check: check-all
	@echo '*** SUCCESS ***'

check-all: $(OBJ)/$(TARGET).$(SOSUFFIX)
	@ \
	success=1; \
	for f in `find groups -name \*.[vt].cpp -or -name \*test.cpp`; \
	do \
	  $(ECHON) "testing $$f "; \
	  if $(CLANG) $(PLUGIN) $(CPPFLAGS) $(PFLAGS) $$f 2>&1 \
             | $(POSTPROCESS) \
             | diff - $(EXPECT) $(REDIRECT); \
	  then \
	    echo OK; \
	  else \
	    success=0; \
            echo "\x1b[31mfail\x1b[0m"; \
	  fi; \
	done; \
	[ $$success = 1 ]

plugin: $(OBJ)/$(TARGET).$(SOSUFFIX)

$(OBJ)/$(TARGET).$(SOSUFFIX): $(OFILES)
	@echo linking shared library
	$(VERBOSE) $(CXX) $(LDFLAGS) -o $@ $(OFILES) $(LDLIBS)

$(OBJ)/%.o: %.cpp
	@if [ ! -d $(@D) ]; then scripts/mkdirhier $(@D); fi
	@echo compiling $(@:$(OBJ)/%.o=%.cpp)
	$(VERBOSE) $(CXX) $(CPPFLAGS) $(CXXFLAGS) $(WARNFLAGS) \
                          -o $@ -c $(@:$(OBJ)/%.o=%.cpp)

clean:
	$(RM) $(OFILES)
	$(RM) $(OBJ)/$(TARGET).$(SOSUFFIX)
	$(RM) $(OBJ)/make.depend
	$(RM) -r $(OBJ)
	$(RM) mkerr olderr *~

# -----------------------------------------------------------------------------

depend $(OBJ)/make.depend:
	@if [ ! -d $(OBJ) ]; then mkdir $(OBJ); fi
	@echo analysing dependencies
	$(VERBOSE) $(CXX) $(CPPFLAGS) -M $(LIBCXXFILES) \
           | scripts/fixdepend $(OBJ) > $(OBJ)/make.depend

include $(OBJ)/make.depend
