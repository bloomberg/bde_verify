cmake_minimum_required(VERSION 3.1)

project(bde_verify)

set(BDE_VERIFY_VERSION_MAJOR "1")
set(BDE_VERIFY_VERSION_MINOR "3")
set(BDE_VERIFY_VERSION_PATCH "11")
set(BDE_VERIFY_VERSION "${BDE_VERIFY_VERSION_MAJOR}.${BDE_VERIFY_VERSION_MINOR}.${BDE_VERIFY_VERSION_PATCH}")

add_subdirectory(bde-verify-vs)

find_package(LLVM REQUIRED)
set(CLANG_RESOURCE_DIR
    "${LLVM_INSTALL_PREFIX}/lib/clang/${LLVM_PACKAGE_VERSION}"
    CACHE PATH "Clang resource directory")
file(GLOB CLANG_RESOURCE_FILES "${CLANG_RESOURCE_DIR}/include/*.*")

if(MSVC)
    add_definitions(
        "/wd4141"
        "/wd4146"
        "/wd4244"
        "/wd4291"
        "/wd4624"
        "/wd4800"
        "/wd4996"
    )
    set(system_libs version)
    set(SCRIPT ".pl")
    set(CPACK_GENERATOR "NSIS")
    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL "ON")
    set(CPACK_NSIS_MODIFY_PATH "ON")
    set(CPACK_NSIS_MUI_ICON
        "${CMAKE_CURRENT_SOURCE_DIR}/bde-verify-vs/BDE_Verify.ico")
    set(CPACK_NSIS_DISPLAY_NAME "BdeVerify")
    set(CPACK_NSIS_PACKAGE_NAME "BdeVerify")
    set(APREGKEY
        "Software\\\\Microsoft\\\\Windows\\\\CurrentVersion\\\\App Paths\\\\")
    set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "
        WriteRegStr SHCTX   '${APREGKEY}bde_verify${SCRIPT}'   '' \\
                    '$INSTDIR\\\\bin\\\\bde_verify${SCRIPT}'
        WriteRegStr SHCTX   '${APREGKEY}bb_cppverify${SCRIPT}' '' \\
                    '$INSTDIR\\\\bin\\\\bb_cppverify${SCRIPT}'
        WriteRegStr SHCTX   '${APREGKEY}check_bos${SCRIPT}'    '' \\
                    '$INSTDIR\\\\bin\\\\check_bos${SCRIPT}'
        WriteRegStr SHCTX   '${APREGKEY}bde_verify_bin.exe'    '' \\
                    '$INSTDIR\\\\libexec\\\\bde-verify\\\\bde_verify_bin.exe'
        System::Call 'Shell32::SHChangeNotify(i 0x8000000, i 0, i 0, i 0)'
    ")
    set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "
        DeleteRegKey SHCTX '${APREGKEY}bde_verify${SCRIPT}'
        DeleteRegKey SHCTX '${APREGKEY}bb_cppverify${SCRIPT}'
        DeleteRegKey SHCTX '${APREGKEY}check_bos${SCRIPT}'
        DeleteRegKey SHCTX '${APREGKEY}bde_verify_bin.exe'
        System::Call 'Shell32::SHChangeNotify(i 0x8000000, i 0, i 0, i 0)'
    ")
else()
    if(NOT $ENV{PREFIX})
        if(IS_DIRECTORY /opt/bb)
            set($ENV{PREFIX} /opt/bb)
        elseif(IS_DIRECTORY /usr)
            set($ENV{PREFIX} /usr)
        endif()
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
    ${G}/csaglb
    ${LLVM_INCLUDE_DIRS}
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
target_compile_definitions(csabase PRIVATE
                           BDE_VERIFY_VERSION="${BDE_VERIFY_VERSION}")

if(MSVC)
    set(RC ${CMAKE_CURRENT_SOURCE_DIR}/bde-verify-vs/bde_verify_bin.rc)
endif()

add_executable(bde_verify_bin
    ${RC}
    ${G}/csaaq/csaaq_cppinexternc.cpp
    ${G}/csaaq/csaaq_freefunctionsdepend.cpp
    ${G}/csaaq/csaaq_friendsinheaders.cpp
    ${G}/csaaq/csaaq_globaldata.cpp
    ${G}/csaaq/csaaq_includeinexternc.cpp
    ${G}/csaaq/csaaq_inentns.cpp
    ${G}/csaaq/csaaq_runtimeinit.cpp
    ${G}/csaaq/csaaq_transitiveincludes.cpp
    ${G}/csabbg/csabbg_allocatorforward.cpp
    ${G}/csabbg/csabbg_allocatornewwithpointer.cpp
    ${G}/csabbg/csabbg_assertassign.cpp
    ${G}/csabbg/csabbg_bslovrdstl.cpp
    ${G}/csabbg/csabbg_bslstdstring.cpp
    ${G}/csabbg/csabbg_classsections.cpp
    ${G}/csabbg/csabbg_cmporder.cpp
    ${G}/csabbg/csabbg_deprecated.cpp
    ${G}/csabbg/csabbg_enumvalue.cpp
    ${G}/csabbg/csabbg_functioncontract.cpp
    ${G}/csabbg/csabbg_managedptr.cpp
    ${G}/csabbg/csabbg_membernames.cpp
    ${G}/csabbg/csabbg_midreturn.cpp
    ${G}/csabbg/csabbg_movecontract.cpp
    ${G}/csabbg/csabbg_testdriver.cpp
    ${G}/csabde/csabde_tool.cpp
    ${G}/csafmt/csafmt_banner.cpp
    ${G}/csafmt/csafmt_comments.cpp
    ${G}/csafmt/csafmt_headline.cpp
    ${G}/csafmt/csafmt_indent.cpp
    ${G}/csafmt/csafmt_longlines.cpp
    ${G}/csafmt/csafmt_nonascii.cpp
    ${G}/csafmt/csafmt_whitespace.cpp
    ${G}/csaglb/csaglb_comments.cpp
    ${G}/csaglb/csaglb_includes.cpp
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
    ${G}/csamisc/csamisc_movablerefref.cpp
    ${G}/csamisc/csamisc_namespacetags.cpp
    ${G}/csamisc/csamisc_opvoidstar.cpp
    ${G}/csamisc/csamisc_shortcompare.cpp
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
    ${G}/csaxform/csaxform_refactor.cpp
    ${G}/csaxform/csaxform_refactor_config.cpp
)

target_compile_features(bde_verify_bin PUBLIC cxx_auto_type)

set(CMAKE_SKIP_RPATH ON)

set(CMAKE_LIBRARY_PATH
    ${LLVM_LIBRARY_DIRS}
    /opt/swt/lib64
    /opt/bb/lib64
    /usr/lib/sparcv9
    /usr/lib64
)

function(llvmlib name)
    find_library(LIB${name}
                 NAMES ${name}
                 PATHS ${LLVM_LIBRARY_DIRS}
                       /lib64
                       /opt/bb/lib64
                       /opt/swt/lib64
                       /usr/lib/sparcv9
                       /usr/lib64
    )
    if(LIB${name})
        target_link_libraries(bde_verify_bin ${LIB${name}})
    endif()
endfunction(llvmlib)

target_link_libraries(bde_verify_bin csabase ${system_libs})

llvmlib(clangFrontendTool)
llvmlib(clangCodeGen)
llvmlib(clangARCMigrate)
llvmlib(clangRewriteFrontend)
llvmlib(clangStaticAnalyzerFrontend)
llvmlib(clangFrontend)
llvmlib(clangSerialization)
llvmlib(clangDriver)
llvmlib(clangParse)
llvmlib(clangSema)
llvmlib(clangEdit)
llvmlib(clangStaticAnalyzerCheckers)
llvmlib(clangStaticAnalyzerCore)
llvmlib(clangAnalysis)
llvmlib(clangTooling)
llvmlib(clangToolingCore)
llvmlib(clangASTMatchers)
llvmlib(clangRewrite)
llvmlib(clangAST)
llvmlib(clangLex)
llvmlib(clangBasic)
llvmlib(clangCrossTU)
llvmlib(clangIndex)
llvmlib(LLVMLTO)
llvmlib(LLVMPasses)
llvmlib(LLVMAggressiveInstCombine)
llvmlib(LLVMObjCARCOpts)
llvmlib(LLVMDebugInfoPDB)
llvmlib(LLVMCoverage)
llvmlib(LLVMSparcCodeGen)
llvmlib(LLVMSparcAsmParser)
llvmlib(LLVMSparcDesc)
llvmlib(LLVMSparcInfo)
llvmlib(LLVMSparcAsmPrinter)
llvmlib(LLVMOption)
llvmlib(LLVMX86AsmParser)
llvmlib(LLVMX86CodeGen)
llvmlib(LLVMGlobalISel)
llvmlib(LLVMSelectionDAG)
llvmlib(LLVMAsmPrinter)
llvmlib(LLVMDebugInfoCodeView)
llvmlib(LLVMDebugInfoMSF)
llvmlib(LLVMX86Desc)
llvmlib(LLVMMCDisassembler)
llvmlib(LLVMX86Info)
llvmlib(LLVMX86AsmPrinter)
llvmlib(LLVMX86Utils)
llvmlib(LLVMCodeGen)
llvmlib(LLVMTarget)
llvmlib(LLVMCoroutines)
llvmlib(LLVMipo)
llvmlib(LLVMInstrumentation)
llvmlib(LLVMVectorize)
llvmlib(LLVMScalarOpts)
llvmlib(LLVMLinker)
llvmlib(LLVMIRReader)
llvmlib(LLVMAsmParser)
llvmlib(LLVMInstCombine)
llvmlib(LLVMTransformUtils)
llvmlib(LLVMBitWriter)
llvmlib(LLVMAnalysis)
llvmlib(LLVMObject)
llvmlib(LLVMMCParser)
llvmlib(LLVMMC)
llvmlib(LLVMBitReader)
llvmlib(LLVMProfileData)
llvmlib(LLVMCore)
llvmlib(LLVMBinaryFormat)
llvmlib(LLVMSupport)
llvmlib(LLVMDemangle)
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
    scripts/check_bos${SCRIPT}
    DESTINATION bin)

install(FILES
    bde.cfg
    bde_verify.cfg
    bb_cppverify.cfg
    DESTINATION etc/bde-verify)

file(GLOB headers "${G}/csabase/csabase_*.h" "${G}/csaglb/csaglb_*.h")

install(FILES ${headers} DESTINATION include/bde-verify)
install(FILES ${CLANG_RESOURCE_FILES}
        DESTINATION include/bde-verify/clang/include)

set(CPACK_PACKAGE_INSTALL_DIRECTORY "BdeVerify")
set(CPACK_PACKAGE_VENDOR "Bloomberg Finance L.P.")
set(CPACK_PACKAGE_VERSION_MAJOR ${BDE_VERIFY_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${BDE_VERIFY_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${BDE_VERIFY_VERSION_PATCH})
set(CPACK_PACKAGE_VERSION       ${BDE_VERIFY_VERSION})
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_NSIS_COMPRESSOR "/SOLID lzma \r\n SetCompressorDictSize 32")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "BDE Verify - Static C++ code analysis")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README")

include(CPack)

## ----------------------------------------------------------------------------
## Copyright (C) 2015 Bloomberg Finance L.P.
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
