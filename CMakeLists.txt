cmake_minimum_required(VERSION 3.1)

project(bde_verify)

if(MSVC)
    add_definitions(
        "/wd4146"
        "/wd4244"
        "/wd4291"
        "/wd4624"
        "/wd4800"
        "/wd4996"
    )
    set(SCRIPT ".pl")
    set(CPACK_GENERATOR "NSIS")
    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
    set(CPACK_NSIS_MODIFY_PATH ON)
    set(CPACK_NSIS_MUI_ICON
        "${CMAKE_CURRENT_SOURCE_DIR}/Windows/BDE_Verify.ico")
    set(CPACK_NSIS_DISPLAY_NAME "BDE Verify")
    set(CPACK_NSIS_PACKAGE_NAME "BDE Verify")
else()
    if(NOT $ENV{PREFIX})
        if(IS_DIRECTORY /opt/bb)
            set($ENV{PREFIX} /opt/bb)
        elseif(IS_DIRECTORY /usr)
            set($ENV{PREFIX} /usr)
        endif()
    endif()

    if(NOT $ENV{LLVMDIR})
        set($ENV{LLVMDIR} $ENV{PREFIX})
    endif()

    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        execute_process(COMMAND "which" "${CMAKE_CXX_COMPILER}"
                        OUTPUT_VARIABLE GCCDIR)
    else()
        execute_process(COMMAND "which" "g++"
                        OUTPUT_VARIABLE GCCDIR)
    endif()
    string(REGEX REPLACE "^(.*)/bin/[^/]*$" "\\1" GCCDIR ${GCCDIR})

    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        add_definitions("--gcc-toolchain=${GCCDIR}")
        add_definitions("-Wno-mismatched-tags")
    endif()

    add_definitions(-D__STDC_LIMIT_MACROS)
    add_definitions(-D__STDC_CONSTANT_MACROS)
    add_definitions(-DSPELL_CHECK=1)

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-common")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-strict-aliasing")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m64")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-local-typedefs")
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "SunOS")
    add_definitions(-DBYTE_ORDER=BIG_ENDIAN)
    foreach(L
        ${GCCDIR}/lib/sparcv9
        $ENV{PREFIX}/lib64
        /opt/swt/lib64
        /usr/lib/sparcv9
    )
        set(CMAKE_EXE_LINKER_FLAGS
            "${CMAKE_EXE_LINKER_FLAGS} -Wl,-L,${L},-R,${L}")
    endforeach(L)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    foreach(L
        ${GCCDIR}/lib64
        $ENV{PREFIX}/lib64
        /opt/swt/lib64
        /usr/lib64
    )
        set(CMAKE_EXE_LINKER_FLAGS
            "${CMAKE_EXE_LINKER_FLAGS} -Wl,-L,${L},-rpath,${L}")
    endforeach(L)
endif()

set(G ${CMAKE_CURRENT_SOURCE_DIR}/groups/csa)

include_directories(
    ${G}/csabase
    $ENV{LLVM}/include
    /opt/bb/include
    /opt/swt/include
)

add_library(csabase
    ${G}/csabase/csabase_abstractvisitor.cpp
    ${G}/csabase/csabase_analyse.cpp
    ${G}/csabase/csabase_analyser.cpp
    ${G}/csabase/csabase_attachments.cpp
    ${G}/csabase/csabase_checkregistry.cpp
    ${G}/csabase/csabase_clang.cpp
    ${G}/csabase/csabase_config.cpp
    ${G}/csabase/csabase_debug.cpp
    ${G}/csabase/csabase_diagnostic_builder.cpp
    ${G}/csabase/csabase_diagnosticfilter.cpp
    ${G}/csabase/csabase_filenames.cpp
    ${G}/csabase/csabase_format.cpp
    ${G}/csabase/csabase_location.cpp
    ${G}/csabase/csabase_ppobserver.cpp
    ${G}/csabase/csabase_registercheck.cpp
    ${G}/csabase/csabase_report.cpp
    ${G}/csabase/csabase_tool.cpp
    ${G}/csabase/csabase_util.cpp
    ${G}/csabase/csabase_visitor.cpp
)

target_compile_features(csabase PUBLIC cxx_auto_type)

if(MSVC)
    set(RC ${CMAKE_CURRENT_SOURCE_DIR}/Windows/bde_verify_bin.rc)
endif()

add_executable(bde_verify_bin
    ${RC}
    ${G}/csaaq/csaaq_cppinexternc.cpp
    ${G}/csaaq/csaaq_freefunctionsdepend.cpp
    ${G}/csaaq/csaaq_friendsinheaders.cpp
    ${G}/csaaq/csaaq_includeinexternc.cpp
    ${G}/csaaq/csaaq_inentns.cpp
    ${G}/csaaq/csaaq_runtimeinit.cpp
    ${G}/csaaq/csaaq_transitiveincludes.cpp
    ${G}/csabbg/csabbg_allocatorforward.cpp
    ${G}/csabbg/csabbg_allocatornewwithpointer.cpp
    ${G}/csabbg/csabbg_assertassign.cpp
    ${G}/csabbg/csabbg_bslovrdstl.cpp
    ${G}/csabbg/csabbg_cmporder.cpp
    ${G}/csabbg/csabbg_deprecated.cpp
    ${G}/csabbg/csabbg_enumvalue.cpp
    ${G}/csabbg/csabbg_functioncontract.cpp
    ${G}/csabbg/csabbg_midreturn.cpp
    ${G}/csabbg/csabbg_testdriver.cpp
    ${G}/csabde/csabde_tool.cpp
    ${G}/csafmt/csafmt_banner.cpp
    ${G}/csafmt/csafmt_comments.cpp
    ${G}/csafmt/csafmt_headline.cpp
    ${G}/csafmt/csafmt_indent.cpp
    ${G}/csafmt/csafmt_longlines.cpp
    ${G}/csafmt/csafmt_nonascii.cpp
    ${G}/csafmt/csafmt_whitespace.cpp
    ${G}/csamisc/csamisc_anonymousnamespaceinheader.cpp
    ${G}/csamisc/csamisc_arrayargument.cpp
    ${G}/csamisc/csamisc_arrayinitialization.cpp
    ${G}/csamisc/csamisc_boolcomparison.cpp
    ${G}/csamisc/csamisc_charclassrange.cpp
    ${G}/csamisc/csamisc_charvsstring.cpp
    ${G}/csamisc/csamisc_constantreturn.cpp
    ${G}/csamisc/csamisc_contiguousswitch.cpp
    ${G}/csamisc/csamisc_cstylecastused.cpp
    ${G}/csamisc/csamisc_donotuseendl.cpp
    ${G}/csamisc/csamisc_dumpast.cpp
    ${G}/csamisc/csamisc_funcalpha.cpp
    ${G}/csamisc/csamisc_hashptr.cpp
    ${G}/csamisc/csamisc_longinline.cpp
    ${G}/csamisc/csamisc_memberdefinitioninclassdefinition.cpp
    ${G}/csamisc/csamisc_namespacetags.cpp
    ${G}/csamisc/csamisc_opvoidstar.cpp
    ${G}/csamisc/csamisc_spellcheck.cpp
    ${G}/csamisc/csamisc_strictaliasing.cpp
    ${G}/csamisc/csamisc_stringadd.cpp
    ${G}/csamisc/csamisc_swapab.cpp
    ${G}/csamisc/csamisc_swapusing.cpp
    ${G}/csamisc/csamisc_thatwhich.cpp
    ${G}/csamisc/csamisc_thrownonstdexception.cpp
    ${G}/csamisc/csamisc_unnamed_temporary.cpp
    ${G}/csamisc/csamisc_verifysameargumentnames.cpp
    ${G}/csastil/csastil_externalguards.cpp
    ${G}/csastil/csastil_implicitctor.cpp
    ${G}/csastil/csastil_includeorder.cpp
    ${G}/csastil/csastil_leakingmacro.cpp
    ${G}/csastil/csastil_templatetypename.cpp
    ${G}/csastil/csastil_uppernames.cpp
    ${G}/csatr/csatr_componentheaderinclude.cpp
    ${G}/csatr/csatr_componentprefix.cpp
    ${G}/csatr/csatr_entityrestrictions.cpp
    ${G}/csatr/csatr_files.cpp
    ${G}/csatr/csatr_friendship.cpp
    ${G}/csatr/csatr_globalfunctiononlyinsource.cpp
    ${G}/csatr/csatr_globaltypeonlyinsource.cpp
    ${G}/csatr/csatr_groupname.cpp
    ${G}/csatr/csatr_includeguard.cpp
    ${G}/csatr/csatr_nesteddeclarations.cpp
    ${G}/csatr/csatr_packagename.cpp
    ${G}/csatr/csatr_usingdeclarationinheader.cpp
    ${G}/csatr/csatr_usingdirectiveinheader.cpp
)

target_compile_features(bde_verify_bin PUBLIC cxx_auto_type)

set(CMAKE_SKIP_RPATH ON)

set(CMAKE_LIBRARY_PATH
    $ENV{LLVM}/lib
    /opt/swt/lib64
    /opt/bb/lib64
    /usr/lib/sparcv9
    /usr/lib64
)

function(llvmlib name)
    find_library(LIB${name}
                 NAMES ${name}
                 PATHS $ENV{LLVM}/lib
                       /opt/swt/lib64
                       /opt/bb/lib64
                       /usr/lib/sparcv9
                       /usr/lib64
    )
    if(LIB${name})
        target_link_libraries(bde_verify_bin ${LIB${name}})
    endif()
endfunction(llvmlib)

target_link_libraries(bde_verify_bin csabase)

llvmlib(LLVMX86Info)
llvmlib(LLVMSparcInfo)
llvmlib(clangFrontendTool)
llvmlib(clangCodeGen)
llvmlib(LLVMIRReader)
llvmlib(LLVMLinker)
llvmlib(LLVMipo)
llvmlib(LLVMX86AsmParser)
llvmlib(LLVMSparcAsmParser)
llvmlib(LLVMX86CodeGen)
llvmlib(LLVMSparcCodeGen)
llvmlib(LLVMSelectionDAG)
llvmlib(LLVMCodeGen)
llvmlib(LLVMScalarOpts)
llvmlib(LLVMInstCombine)
llvmlib(LLVMVectorize)
llvmlib(LLVMInstrumentation)
llvmlib(LLVMObjCARCOpts)
llvmlib(LLVMTransformUtils)
llvmlib(LLVMipa)
llvmlib(LLVMAnalysis)
llvmlib(clangStaticAnalyzerFrontend)
llvmlib(clangRewriteFrontend)
llvmlib(clangARCMigrate)
llvmlib(clangFrontend)
llvmlib(clangSerialization)
llvmlib(LLVMProfileData)
llvmlib(LLVMX86Desc)
llvmlib(LLVMSparcDesc)
llvmlib(LLVMObject)
llvmlib(LLVMBitReader)
llvmlib(LLVMTarget)
llvmlib(LLVMAsmParser)
llvmlib(LLVMBitWriter)
llvmlib(LLVMX86AsmPrinter)
llvmlib(LLVMSparcAsmPrinter)
llvmlib(LLVMX86Utils)
llvmlib(LLVMCore)
llvmlib(clangParse)
llvmlib(LLVMMCParser)
llvmlib(LLVMMCDisassembler)
llvmlib(clangSema)
llvmlib(clangStaticAnalyzerCheckers)
llvmlib(clangStaticAnalyzerCore)
llvmlib(clangASTMatchers)
llvmlib(clangEdit)
llvmlib(clangAnalysis)
llvmlib(clangAST)
llvmlib(clangToolingCore)
llvmlib(clangRewrite)
llvmlib(clangLex)
llvmlib(clangDriver)
llvmlib(clangBasic)
llvmlib(LLVMMC)
llvmlib(LLVMOption)
llvmlib(LLVMSupport)

llvmlib(ncurses)
llvmlib(pthread)
llvmlib(dl)
llvmlib(z)
llvmlib(aspell)
llvmlib(rt)
llvmlib(tinfo)
llvmlib(malloc)

string(REGEX REPLACE
    "^.*[/\\]([^/\\]*)$"
    "${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_SYSTEM_NAME}-\\1"
    CMAKE_INSTALL_PREFIX
    ${CMAKE_CXX_COMPILER}
)
string(REGEX REPLACE
    "[.].*$"
    ""
    CMAKE_INSTALL_PREFIX
    ${CMAKE_INSTALL_PREFIX}
)

install(TARGETS
    csabase
    bde_verify_bin
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION libexec/bde-verify
)

install(PROGRAMS
    scripts/bde_verify${SCRIPT}
    scripts/bb_cppverify${SCRIPT}
    DESTINATION bin)

install(FILES
    bde_verify.cfg
    bb_cppverify.cfg
    DESTINATION etc/bde-verify)

file(GLOB headers "${G}/csabase/csabase_*.h")

install(FILES ${headers} DESTINATION include/bde-verify)

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "BDE Verify - Static C++ code analysis")
set(PACK_PACKAGE_VENDOR "Bloomberg Finance L.P.")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "BDE Verify")
set(CPACK_PACKAGE_VERSION_MAJOR "1")
set(CPACK_PACKAGE_VERSION_MINOR "1")
set(CPACK_PACKAGE_VERSION_PATCH "0")

include(CPack)

## ----------------------------------------------------------------------------
## Copyright (C) 2015 Bloomberg Finance L.P.
##
## Permission is hereby granted, free of charge, to any person obtaining a copy
## of this software and associated documentation files (the "Software"), to
## deal in the Software without restriction, including without limitation the
## rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
## sell copies of the Software, and to permit persons to whom the Software is
## furnished to do so, subject to the following conditions:
##
## The above copyright notice and this permission notice shall be included in
## all copies or substantial portions of the Software.
##
## THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
## IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
## FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
## AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
## LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
## FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
## IN THE SOFTWARE.
## ----------------------------- END-OF-FILE ----------------------------------
