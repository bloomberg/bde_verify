cmake_minimum_required(VERSION 3.16)

project(bde_verify)

set(BDE_VERIFY_VERSION_MAJOR "1")
set(BDE_VERIFY_VERSION_MINOR "3")
set(BDE_VERIFY_VERSION_PATCH "20")
set(BDE_VERIFY_VERSION "${BDE_VERIFY_VERSION_MAJOR}.${BDE_VERIFY_VERSION_MINOR}.${BDE_VERIFY_VERSION_PATCH}")

# Only support 64 bit build.
if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    message(FATAL_ERROR "No support for 32 bit build.")
endif()

if(NOT DEFINED DISTRIBUTION_REFROOT)
    if(DEFINED ENV{DISTRIBUTION_REFROOT})
        set(DISTRIBUTION_REFROOT "$ENV{DISTRIBUTION_REFROOT}/" CACHE STRING "BB Dpkg root set from environment variable.")
    else()
        get_filename_component(REFROOT ${CMAKE_CURRENT_LIST_DIR}/../../../../ REALPATH)
        set(DISTRIBUTION_REFROOT ${REFROOT}/ CACHE STRING "BB Dpkg root set from toolchain file location.")
    endif()
endif()

set_property(GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS True)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Note that Clang transitively brings in LLVM config as REQUIRED.
find_package(Clang REQUIRED)

message(STATUS "Found LLVM/Clang ${LLVM_PACKAGE_VERSION}")

# Bring in LLVM modules that will help to setup same compilation 
# flags and link with LLVM libraries.
list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(AddLLVM)

link_directories(${LLVM_LIBRARY_DIRS})

set(CLANG_RESOURCE_DIR
    "${Clang_DIR}/../../clang/${LLVM_PACKAGE_VERSION}"
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
endif()

# Configure csabase (aka bde-verify) library
set(G ${CMAKE_CURRENT_SOURCE_DIR}/groups/csa)

add_library(bde-verify
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

target_compile_definitions(bde-verify PUBLIC BDE_VERIFY_VERSION="${BDE_VERIFY_VERSION}")
target_include_directories(bde-verify SYSTEM PRIVATE ${CLANG_INCLUDE_DIRS})
llvm_update_compile_flags(bde-verify)

if(MSVC)
    set(RC ${CMAKE_CURRENT_SOURCE_DIR}/bde-verify-vs/bde_verify_bin.rc)
endif()

include_directories(
    ${G}/csabase
    ${G}/csaglb
)

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
    ${G}/csaxform/csaxform_precon.cpp
    ${G}/csaxform/csaxform_refactor.cpp
    ${G}/csaxform/csaxform_refactor_config.cpp
)

target_include_directories(bde_verify_bin SYSTEM PRIVATE ${CLANG_INCLUDE_DIRS})

target_link_libraries(bde_verify_bin
    bde-verify
    clangTooling
    clangFrontend
    clangFrontendTool
)
llvm_update_compile_flags(bde_verify_bin)

# Find aspell library.
find_library(libaspell_LIB NAMES aspell HINTS ${CMAKE_SYSTEM_PREFIX_PATH})
find_file(libaspell_INC NAMES aspell.h HINTS ${CMAKE_SYSTEM_PREFIX_PATH})

if (libaspell_LIB AND libaspell_INC)
    message(STATUS "Found aspell library. Spell checks enabled : ${libaspell_LIB}")

    get_filename_component(libaspell_INCLUDE_DIR ${libaspell_INC} DIRECTORY)

    add_definitions(-DSPELL_CHECK=1)
    target_include_directories(bde_verify_bin SYSTEM PRIVATE ${libaspell_INCLUDE_DIR})
    target_link_libraries(bde_verify_bin ${libaspell_LIB})
endif()


# For this application we need to setup RPATH for build and install runtime respectively
set_property(TARGET bde_verify_bin
    APPEND PROPERTY BUILD_RPATH "${DISTRIBUTION_REFROOT}/opt/bb/lib64")
set_property(TARGET bde_verify_bin
    APPEND PROPERTY INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib64")

message(STATUS "Install prefix = ${CMAKE_INSTALL_PREFIX}")

install(TARGETS bde-verify
        ARCHIVE DESTINATION lib64
        COMPONENT bde-verify-dev
)

file(GLOB headers "${G}/csabase/csabase_*.h" "${G}/csaglb/csaglb_*.h")
install(FILES ${headers}
        DESTINATION include/bde-verify
        COMPONENT bde-verify-dev
)
file(GLOB headers "${G}/csabase/utils/*.hpp")
install(FILES ${headers}
        DESTINATION include/bde-verify/utils
        COMPONENT bde-verify-dev
)

install(TARGETS bde_verify_bin
        RUNTIME DESTINATION libexec/bde-verify
        COMPONENT bde-verify
)

install(PROGRAMS scripts/bde_verify${SCRIPT} scripts/check_bos${SCRIPT}
        DESTINATION bin
        COMPONENT bde-verify
)

install(FILES bde.cfg bde_verify.cfg
        DESTINATION etc/bde-verify
        COMPONENT bde-verify
)

install(FILES ${CLANG_RESOURCE_FILES}
        DESTINATION include/bde-verify/clang/include
        COMPONENT bde-verify
)

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
## Copyright (C) 2021 Bloomberg Finance L.P.
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
