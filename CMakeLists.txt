cmake_minimum_required(VERSION 3.16)

project(bde_verify)

set(BDE_VERIFY_VERSION_MAJOR "1")
set(BDE_VERIFY_VERSION_MINOR "3")
set(BDE_VERIFY_VERSION_PATCH "18")
set(BDE_VERIFY_VERSION "${BDE_VERIFY_VERSION_MAJOR}.${BDE_VERIFY_VERSION_MINOR}.${BDE_VERIFY_VERSION_PATCH}")

if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    message(FATAL_ERROR "No support for 32 bit build.")
endif()

# Discovering llvm, clang and clang tools.
if (DEFINED ENV{DISTRIBUTION_REFROOT})
    set(LLVM_INSTALL_PATH "$ENV{DISTRIBUTION_REFROOT}/opt/bb/lib/llvm-11.0"
        CACHE STRING "Path to llvm installation")
elseif(DEFINED ENV{LLVM_INSTALL_PATH})
    set(LLVM_INSTALL_PATH "$ENV{LLVM_INSTALL_PATH}"
        CACHE STRING "Path to llvm installation")
elseif()
    message(FATAL_ERROR "No LLVM installation found")
endif()

list(APPEND CMAKE_PREFIX_PATH "${LLVM_INSTALL_PATH}/lib64/cmake/clang")

find_package(Clang REQUIRED)
find_package(clangmetatool REQUIRED)

# Clang will transitively find llvm.
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
message(STATUS "Found Clang ${CLANG_PACKAGE_VERSION}")
message(STATUS "Found Clang include ${CLANG_INCLUDE_DIRS}")

if (NOT ${LLVM_ENABLE_EH})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions")
endif()

if (NOT ${LLVM_ENABLE_RTTI})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
endif()

add_definitions(${LLVM_DEFINITIONS})

set(CLANG_RESOURCE_DIR
    "${LLVM_INSTALL_PREFIX}/lib64/clang/${LLVM_PACKAGE_VERSION}"
    CACHE PATH "Clang resource directory")
file(GLOB CLANG_RESOURCE_FILES "${CLANG_RESOURCE_DIR}/include/*.*")

if(MSVC)
    add_subdirectory(bde-verify-vs)

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

    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
        set(CMAKE_CXX_COMPILER "clang++")
        add_compile_options("-Wno-mismatched-tags")
    elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        add_compile_options("--gcc-toolchain=${GCCDIR}")
        add_compile_options("-Wno-mismatched-tags")
    endif()

endif()

set(G ${CMAKE_CURRENT_SOURCE_DIR}/groups/csa)

include_directories(
    ${G}/csabase
    ${G}/csaglb
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

target_compile_definitions(csabase PUBLIC BDE_VERIFY_VERSION="${BDE_VERIFY_VERSION}")
target_include_directories(csabase PUBLIC ${CLANG_INCLUDE_DIRS})

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

set(CMAKE_LIBRARY_PATH
    ${LLVM_LIBRARY_DIRS}
    /opt/bb/lib64
    /usr/lib/sparcv9
    /usr/lib64
)


#target_link_libraries(bde_verify_bin csabase ${system_libs})
target_include_directories(csabase PUBLIC ${CLANG_INCLUDE_DIRS})
target_link_libraries(bde_verify_bin 
                          csabase
                          clangTooling
                          clangFrontend
                          clangFrontendTool)

# Find aspell library.
if (DEFINED ENV{DISTRIBUTION_REFROOT})
    find_library(aspell_LIB
                 NAMES aspell
                 PATHS "$ENV{DISTRIBUTION_REFROOT}/opt/bb/lib64")
else()
    find_library(aspell_LIB
                 NAMES aspell)
endif()

if (aspell_LIB)
    message(STATUS "Found aspell library. Spell checks enabled.")
    add_definitions(-DSPELL_CHECK=1)
    if (DEFINED ENV{DISTRIBUTION_REFROOT})
        target_include_directories(bde_verify_bin PUBLIC "$ENV{DISTRIBUTION_REFROOT}/opt/bb/include")
    endif()
    target_link_libraries(bde_verify_bin ${aspell_LIB})
endif()

message("install prefix = ${CMAKE_INSTALL_PREFIX}")
string(REGEX REPLACE
    "^.*[/\\]([^/\\]*)$"
    "${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_SYSTEM_NAME}-\\1"
    CMAKE_INSTALL_PREFIX
    "/${CMAKE_CXX_COMPILER}"
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
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.md")

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
