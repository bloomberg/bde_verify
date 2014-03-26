#   Makefile                                                     -*-makefile-*-
#  ----------------------------------------------------------------------------
#  Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
#  Modified  2013 Hyman Rosen (hrosen4@bloomberg.net)
#  Distributed under the Boost Software License, Version 1.0. (See file  
#  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
#  ----------------------------------------------------------------------------
# $Id$

TARGET   = bde_verify

default: $(TARGET)

SYSTEM   = $(shell uname -s)

COMPILER = gcc
#COMPILER = clang

STD      = CXX2011

ifeq ($(SYSTEM),Linux)
    ifeq    ($(COMPILER),gcc)
VERSION  = 4.8.1
CCDIR    = /opt/swt/install/gcc-4.8.1
CXX      = $(CCDIR)/bin/g++
         ifeq        ($(STD),CXX2011)
CXXFLAGS += -std=c++0x
         endif
LINK     = $(CXX)
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
    ifeq    ($(COMPILER),gcc)
VERSION  = 4.8.1
CCDIR    = /opt/swt/install/gcc-4.8.1
CXX      = $(CCDIR)/bin/g++
         ifeq        ($(STD),CXX2011)
CXXFLAGS += -std=c++0x
         endif
LINK     = $(CXX)
LDFLAGS  += -Wl,-R,$(CCDIR)/lib
CXXFLAGS += -Wno-unused-local-typedefs
    endif
#ASPELL   = /opt/swt/install/aspell-0.60.6.1-64
#CXXFLAGS += -DSPELL_CHECK=1
#INCFLAGS += -I$(ASPELL)/include
#LDFLAGS  += -L$(ASPELL)/lib64 -laspell
EXTRALIBS += -lrt
EXTRALIBS += -lmalloc
endif

OBJ      = $(SYSTEM)-$(COMPILER)-$(VERSION)

LLVM     = /home/hrosen4/mbig/llvm-3.4/install-$(SYSTEM)
INCFLAGS += -I$(LLVM)/include
LDFLAGS  += -L$(LLVM)/lib

#VERBOSE  =
VERBOSE  = @

#  ----------------------------------------------------------------------------

TSTCXXFILES +=                                                                \
        groups/csa/csabbg/csabbg_allocatorforward.cpp                         \
        groups/csa/csabbg/csabbg_allocatornewwithpointer.cpp                  \
        groups/csa/csabbg/csabbg_enumvalue.cpp                                \
        groups/csa/csabbg/csabbg_functioncontract.cpp                         \
        groups/csa/csabbg/csabbg_midreturn.cpp                                \
        groups/csa/csabbg/csabbg_testdriver.cpp                               \
        groups/csa/csafmt/csafmt_headline.cpp                                 \
        groups/csa/csafmt/csafmt_banner.cpp                                   \
        groups/csa/csafmt/csafmt_comments.cpp                                 \
        groups/csa/csafmt/csafmt_longlines.cpp                                \
        groups/csa/csafmt/csafmt_nonascii.cpp                                 \
        groups/csa/csafmt/csafmt_whitespace.cpp                               \
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
        groups/csa/csamisc/csamisc_funcalpha.cpp                              \
        groups/csa/csamisc/csamisc_hashptr.cpp                                \
        groups/csa/csamisc/csamisc_longinline.cpp                             \
        groups/csa/csamisc/csamisc_memberdefinitioninclassdefinition.cpp      \
        groups/csa/csamisc/csamisc_opvoidstar.cpp                             \
        groups/csa/csamisc/csamisc_thrownonstdexception.cpp                   \
        groups/csa/csamisc/csamisc_verifysameargumentnames.cpp                \
        groups/csa/csamisc/csamisc_stringadd.cpp                              \
        groups/csa/csamisc/csamisc_swapab.cpp                                 \
        groups/csa/csamisc/csamisc_spellcheck.cpp                             \

LIBCXXFILES +=                                                                \
        groups/csa/csabase/csabase_abstractvisitor.cpp                        \
        groups/csa/csabase/csabase_analyser.cpp                               \
        groups/csa/csabase/csabase_attachments.cpp                            \
        groups/csa/csabase/csabase_checkregistry.cpp                          \
        groups/csa/csabase/csabase_config.cpp                                 \
        groups/csa/csabase/csabase_analyse.cpp                                \
        groups/csa/csabase/csabase_debug.cpp                                  \
        groups/csa/csabase/csabase_diagnosticfilter.cpp                       \
        groups/csa/csabase/csabase_filenames.cpp                              \
        groups/csa/csabase/csabase_format.cpp                                 \
        groups/csa/csabase/csabase_location.cpp                               \
        groups/csa/csabase/csabase_ppobserver.cpp                             \
        groups/csa/csabase/csabase_registercheck.cpp                          \
        groups/csa/csabase/csabase_tool.cpp                                   \
        groups/csa/csabase/csabase_util.cpp                                   \
        groups/csa/csabase/csabase_visitor.cpp                                \
        $(TSTCXXFILES)

TODO =                                                                        \
        groups/csa/csamisc/csamisc_calls.cpp                                  \
        groups/csa/csamisc/csamisc_includeguard.cpp                           \
        groups/csa/csamisc/csamisc_selfinitialization.cpp                     \
        groups/csa/csamisc/csamisc_superfluoustemporary.cpp                   \
        groups/csa/csadep/csadep_dependencies.cpp                             \
        groups/csa/csadep/csadep_types.cpp                                    \

# -----------------------------------------------------------------------------

#DEBUG    = on
DEBUG    = off

REDIRECT = $(VERBOSE:@=>/dev/null 2>&1)

INCFLAGS += -I.
#DEFFLAGS += -D_DEBUG
#DEFFLAGS += -D_GNU_SOURCE
DEFFLAGS += -D__STDC_LIMIT_MACROS
DEFFLAGS += -D__STDC_CONSTANT_MACROS
INCFLAGS += -Igroups/csa/csabase
INCFLAGS += -Igroups/csa/csadep
INCFLAGS += -Iinclude
CPPFLAGS += $(INCFLAGS) $(DEFFLAGS) $(STDFLAGS)
CXXFLAGS += -g -fno-common -fno-strict-aliasing -fno-exceptions -fno-rtti
LDFLAGS += -g

OFILES = $(LIBCXXFILES:%.cpp=$(OBJ)/%.o)

LIBS     =                                                                    \
              -lLLVMX86AsmParser                                              \
              -lclangFrontendTool                                             \
              -lclangCodeGen                                                  \
              -lLLVMIRReader                                                  \
              -lLLVMLinker                                                    \
              -lLLVMipo                                                       \
              -lLLVMX86CodeGen                                                \
              -lLLVMSparcCodeGen                                                \
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
              -lLLVMSparcDesc                                                   \
              -lLLVMX86Info                                                   \
              -lLLVMSparcInfo                                                   \
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

$(TARGET): $(OBJ)/$(TARGET)

$(OBJ)/$(TARGET): $(OFILES)
	@echo linking executable
	$(VERBOSE) $(LINK) $(LDFLAGS) -o $@ $(OFILES) $(LIBS)

$(OBJ)/%.o: %.cpp
	@if [ ! -d $(@D) ]; then scripts/mkdirhier $(@D); fi
	@echo compiling $(@:$(OBJ)/%.o=%.cpp)
	$(VERBOSE) $(CXX) $(CPPFLAGS) $(CXXFLAGS) $(WARNFLAGS) \
                          -o $@ -c $(@:$(OBJ)/%.o=%.cpp)

clean:
	$(RM) $(OFILES)
	$(RM) $(OBJ)/$(TARGET)
	$(RM) $(OBJ)/make.depend
	$(RM) -r $(OBJ)
	$(RM) mkerr olderr *~

# -----------------------------------------------------------------------------

CURRENT  = csafmt/csafmt_nonascii.t.cpp

VERIFY   = $(OBJ)/$(TARGET) -plugin bde_verify
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

CHECK_NAME  = $$(echo | \
                 sed -n 's/.*check_name("\([^"]*\)".*/\1/p' \
                    $(SOURCE) 2>/dev/null)

PFLAGS      += $$($(CXX) -xc++ -E -v /dev/null 2>&1 | \
                  sed -n '/^ [/][^ ]*$$/s/ //p' | \
                  sed 's/^/-I/')

BB = /bb/build/share/packages/refroot/amd64/unstable/bb
PFLAGS += -I $(BB)/include
PFLAGS += -I $(BB)/include/stlport

PFLAGS += -std=c++0x
PFLAGS += -Wno-string-plus-int
PFLAGS += -fcxx-exceptions
PFLAGS += -fcolor-diagnostics

check: check-all
	@echo '*** SUCCESS ***'

current: $(OBJ)/$(TARGET)
	$(VERBOSE) \
    f=groups/csa/$(CURRENT); \
    if echo $(TODO) | grep -q $(SOURCE); then echo skipping $$f; else \
      trap "rm -f $$$$" EXIT; \
      (echo namespace bde_verify; \
       echo all on) > $$$$; \
      if [ -f "$(SOURCE)" -a ! -z "$(CHECK_NAME)" ]; then \
        (echo namespace bde_verify; \
         echo all off; \
         echo check $(CHECK_NAME) on) > $$$$; \
      fi; \
      $(VERIFY) \
        -plugin-arg-bde_verify debug-$(DEBUG) \
        -plugin-arg-bde_verify config=$$$$ \
        $(CPPFLAGS) $(PFLAGS) $$f; \
    fi

check-current: $(OBJ)/$(TARGET)
	$(VERBOSE) \
    success=1; \
    f=groups/csa/$(CURRENT); \
    if echo $(TODO) | grep -q $(SOURCE); then echo skipping $$f; else \
      trap "rm -f $$$$" EXIT; \
      (echo namespace bde_verify; \
       echo all on) > $$$$; \
      if [ -f "$(SOURCE)" -a ! -z "$(CHECK_NAME)" ]; then \
        (echo namespace bde_verify; \
         echo all off; \
         echo check $(CHECK_NAME) on) > $$$$; \
      fi; \
      if $(VERIFY) -plugin-arg-bde_verify config=$$$$ \
        $(CPPFLAGS) $(PFLAGS) $$f 2>&1 \
          | $(POSTPROCESS) \
          | diff - $(EXPECT) $(REDIRECT); \
      then \
        echo OK; \
      else \
        success=0; \
        cat $$$$; \
        $(VERIFY) -plugin-arg-bde_verify config=$$$$ \
          $(CPPFLAGS) $(PFLAGS) $$f 2>&1 \
            | $(POSTPROCESS) \
            | diff - $(EXPECT); \
        echo -e "\x1b[31mfail\x1b[0m"; \
      fi; \
    fi; \
    [ $$success = 1 ]

check-all: $(OBJ)/$(TARGET)
	$(VERBOSE) \
    success=1; \
    for f in $$(find groups -name \*.[vt].cpp -or -name \*test.cpp); \
    do \
      if echo $(TODO) | grep -q $(SOURCE); \
        then echo skipping $$f; continue; fi; \
      echo "testing $$f "; \
      trap "rm -f $$$$" EXIT; \
      (echo namespace bde_verify; \
       echo all on) > $$$$; \
      if [ -f "$(SOURCE)" -a ! -z "$(CHECK_NAME)" ]; then \
        (echo namespace bde_verify; \
         echo all off; \
         echo check $(CHECK_NAME) on) > $$$$; \
      fi; \
      if $(VERIFY) -plugin-arg-bde_verify config=$$$$ \
        $(CPPFLAGS) $(PFLAGS) $$f 2>&1 \
          | $(POSTPROCESS) \
          | diff - $(EXPECT) $(REDIRECT); \
      then \
        echo OK; \
      else \
        success=0; \
        cat $$$$; \
        $(VERIFY) -plugin-arg-bde_verify config=$$$$ \
          $(CPPFLAGS) $(PFLAGS) $$f 2>&1 \
            | $(POSTPROCESS) \
            | diff - $(EXPECT); \
        echo -e "\x1b[31mfail\x1b[0m"; \
      fi; \
    done; \
    [ $$success = 1 ]

# -----------------------------------------------------------------------------

depend $(OBJ)/make.depend:
	@if [ ! -d $(OBJ) ]; then mkdir $(OBJ); fi
	@echo analysing dependencies
	$(VERBOSE) $(CXX) $(CPPFLAGS) -M $(LIBCXXFILES) \
           | scripts/fixdepend $(OBJ) > $(OBJ)/make.depend

           include $(OBJ)/make.depend
