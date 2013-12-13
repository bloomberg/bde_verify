#   Makefile                                                     -*-makefile-*-
#  ----------------------------------------------------------------------------
#  Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
#  Modified  2013 Hyman Rosen (hrosen4@bloomberg.net)
#  Distributed under the Boost Software License, Version 1.0. (See file  
#  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
#  ----------------------------------------------------------------------------
# $Id$

default:  check-current

LLVM     = /home/hrosen4/mbig/llvm-svn/install
LLVMINC  = -I$(LLVM)/include
LLVMLIB  = $(LLVM)/lib
CLANG    = $(LLVM)/bin/clang
CLANGVER = 3.5

COMPILER = clang

CURRENT  = csafmt/csafmt_nonascii.t.cpp

#  ----------------------------------------------------------------------------

TARGET = coolyser

TSTCXXFILES +=                                                                \
        groups/csa/csabbg/csabbg_allocatorforward.cpp                         \
        groups/csa/csabbg/csabbg_allocatornewwithpointer.cpp                  \
        groups/csa/csabbg/csabbg_enumvalue.cpp                                \
        groups/csa/csabbg/csabbg_functioncontract.cpp                         \
        groups/csa/csabbg/csabbg_midreturn.cpp                                \
        groups/csa/csafmt/csafmt_headline.cpp                                 \
        groups/csa/csafmt/csafmt_nonascii.cpp                                 \
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
        groups/csa/csastil/csastil_includeorder.cpp                           \
        groups/csa/csastil/csastil_templatetypename.cpp                       \
        groups/csa/csastil/csastil_leakingmacro.cpp                           \
        groups/csa/csastil/csastil_externalguards.cpp                         \
        groups/csa/csamisc/csamisc_charvsstring.cpp                           \
        groups/csa/csamisc/csamisc_arrayinitialization.cpp                    \
        groups/csa/csamisc/csamisc_anonymousnamespaceinheader.cpp             \
        groups/csa/csamisc/csamisc_boolcomparison.cpp                         \
        groups/csa/csamisc/csamisc_cstylecastused.cpp                         \
        groups/csa/csamisc/csamisc_constantreturn.cpp                         \
        groups/csa/csamisc/csamisc_contiguousswitch.cpp                       \
        groups/csa/csamisc/csamisc_longinline.cpp                             \
        groups/csa/csamisc/csamisc_memberdefinitioninclassdefinition.cpp      \
        groups/csa/csamisc/csamisc_thrownonstdexception.cpp                   \
        groups/csa/csamisc/csamisc_verifysameargumentnames.cpp                \
        groups/csa/csamisc/csamisc_stringadd.cpp                              \

TODO =                                                                        \
        groups/csa/csamisc/csamisc_calls.cpp                                  \
        groups/csa/csamisc/csamisc_includeguard.cpp                           \
        groups/csa/csamisc/csamisc_selfinitialization.cpp                     \
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

BB = /bb/build/share/packages/refroot/amd64/unstable/bb
PFLAGS += -Igroups/csa/csabase
PFLAGS += -Igroups/csa/csadep
PFLAGS += -isystem $(BB)/include
PFLAGS += -isystem $(BB)/include/stlport
PFLAGS += -isystem /usr/include
PFLAGS += -isystem /usr/include/c++/4.4.4
PFLAGS += -isystem /usr/include/c++/4.4.4/backward
PFLAGS += -isystem /usr/include/c++/4.4.4/x86_64-redhat-linux6E/32
PFLAGS += -isystem /usr/lib/gcc/x86_64-redhat-linux6E/4.4.4/include
PFLAGS += -isystem /usr/lib/gcc/x86_64-redhat-linux/4.1.2/include
PFLAGS += -DBB_THREADED
PFLAGS += -DBDE_BUILD_TARGET_DBG
PFLAGS += -DBDE_BUILD_TARGET_EXC
PFLAGS += -DBDE_BUILD_TARGET_MT
PFLAGS += -DBSL_OVERRIDES_STD
PFLAGS += -D_LINUX_SOURCE
PFLAGS += -D_REENTRANT
PFLAGS += -D_SYS_SYSMACROS_H
PFLAGS += -D_THREAD_SAFE

SYSTEM   = $(shell uname -s)
ECHON    = echo
CPPFLAGS += -DCLANG_SVN
PFLAGS   += -Wno-string-plus-int
CXXFLAGS += -fvisibility-inlines-hidden 
LDLIBS   += -lclangEdit

SOSUFFIX = so
CXX      = $(CLANG)
LINK     = $(CXX)
OBJ      = $(SYSTEM)-$(COMPILER)-$(CLANGVER)
#DEBUG    = on
DEBUG    = off
#VERBOSE  =
VERBOSE  = @
# STD      = CXX2011
REDIRECT = $(VERBOSE:@=>/dev/null 2>&1)

INCFLAGS = $(LLVMINC) -I.
DEFFLAGS = -D_DEBUG -D_GNU_SOURCE -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS
ifeq ($(STD),CXX2011)
  STDFLAGS = -std=c++0x -DCOOL_CXX2011
else
  STDFLAGS = -Wno-c++11-extensions
endif
CPPFLAGS += $(INCFLAGS) $(DEFFLAGS) $(STDFLAGS)
CPPFLAGS += -Igroups/csa/csabase -Igroups/csa/csadep
CPPFLAGS += -Iinclude
PFLAGS   += -fcxx-exceptions
PFLAGS   += -fcolor-diagnostics
CXXFLAGS += -g -fno-common -fno-strict-aliasing -fno-exceptions -fno-rtti
WARNFLAGS = \
        -Wcast-qual \
        -Wno-long-long \
        -Wall \
        -W \
        -Wno-unused-parameter \
        -Wno-overloaded-virtual \
        -Wwrite-strings
LDFLAGS = -L$(LLVMLIB) -g

LDLIBS += -lclangEdit
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

CXXFLAGS += -fpic
LDFLAGS  += -shared
LDLIBS   += -ldl

PLUGIN   = -cc1 -load $(OBJ)/$(TARGET).$(SOSUFFIX) \
           -plugin coolyse -plugin-arg-coolyse config=test.cfg
OFILES = $(LIBCXXFILES:%.cpp=$(OBJ)/%.o)
POSTPROCESS = sed -e 's/\([^:]*:[0-9][0-9]*\):[^:]*:/\1:0:/' \
            | sed -e '/\^/s/ //g' \
            | sed -e 's/~~~~~\(~*\)/~~~~~/g' \
            | sed -e '/^$$/d' \
            | sed -e 's/\x1B[^m]*m//g'

EXPECT      = $$(echo $$f | \
               sed -e 's/test\.cpp$$/.exp/' | \
               sed -e 's/\.t.cpp$$/.exp/' | \
               sed -e 's/\.v.cpp$$/.exp/')

SOURCE      = $$(echo $$f | \
               sed -e 's/test\.cpp$$/.cpp/' | \
               sed -e 's/\.t.cpp$$/.cpp/' | \
               sed -e 's/\.v.cpp$$/.cpp/')

CHECK_NAME  = $$(echo | sed -n 's/.*check_name("\([^"]*\)".*/\1/p' $(SOURCE))

# -----------------------------------------------------------------------------

check-current: $(OBJ)/$(TARGET).$(SOSUFFIX)
	$(VERBOSE) \
    f=groups/csa/$(CURRENT); \
    if echo $(TODO) | grep -q $(SOURCE); then echo skipping $$f; else \
      trap "rm -f $$$$" EXIT; \
      (echo namespace cool; \
       echo all on) > $$$$; \
      if [ -f "$(SOURCE)" ]; then if [ ! -z "$(CHECK_NAME)" ]; then \
        (echo namespace cool; \
         echo all off; \
         echo check $(CHECK_NAME) on) > $$$$; \
      fi; fi; \
      $(CLANG) $(PLUGIN) \
        -plugin-arg-coolyse debug-$(DEBUG) \
        -plugin-arg-coolyse config=$$$$ \
        -plugin-arg-coolyse tool=bdechk \
        $(CPPFLAGS) $(PFLAGS) $$f; \
    fi

check: check-all
	@echo '*** SUCCESS ***'

check-all: $(OBJ)/$(TARGET).$(SOSUFFIX)
	$(VERBOSE) \
	success=1; \
	for f in $$(find groups -name \*.[vt].cpp -or -name \*test.cpp); \
	do \
      if echo $(TODO) | grep -q $(SOURCE); \
        then echo skipping $$f; continue; fi; \
	  $(ECHON) "testing $$f "; \
      trap "rm -f $$$$" EXIT; \
      (echo namespace cool; \
       echo all on) > $$$$; \
      if [ -f "$(SOURCE)" ]; then if [ ! -z "$(CHECK_NAME)" ]; then \
        (echo namespace cool; \
         echo all off; \
         echo check $(CHECK_NAME) on) > $$$$; \
      fi; fi; \
	  if $(CLANG) $(PLUGIN) -plugin-arg-coolyse \
          config=$$$$ $(CPPFLAGS) $(PFLAGS) $$f 2>&1 \
             | $(POSTPROCESS) \
             | diff - $(EXPECT) $(REDIRECT); \
	  then \
	    echo OK; \
	  else \
	    success=0; \
        cat $$$$; \
        $(CLANG) $(PLUGIN) -plugin-arg-coolyse config=$$$$ $(CPPFLAGS) $(PFLAGS) $$f 2>&1 \
          | $(POSTPROCESS) \
          | diff - $(EXPECT); \
        echo -e "\x1b[31mfail\x1b[0m"; \
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
