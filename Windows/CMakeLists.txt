option(BUILD_BDEVERIFY_VS_PLUGIN "Build bde-verify VS plugin" OFF)
if (BUILD_BDEVERIFY_VS_PLUGIN)
  add_custom_target(bde_verify_exe_for_vsix
      ${CMAKE_COMMAND} -E copy_if_different
      "${LLVM_TOOLS_BINARY_DIR}/bde-verify.exe"
      "${CMAKE_CURRENT_SOURCE_DIR}/BdeVerify/bde-verify.exe"
      DEPENDS bde-verify)

  add_custom_target(bde_verify_license
      ${CMAKE_COMMAND} -E copy_if_different
      "${CLANG_SOURCE_DIR}/LICENSE.TXT"
      "${CMAKE_CURRENT_SOURCE_DIR}/BdeVerify/license.txt")

  # Build number added to Clang version to ensure that new VSIX can be upgraded
  string(TIMESTAMP BDEVERIFY_VSIX_BUILD %y%m%d%H%M UTC)

  if (NOT BDEVERIFY_VS_VERSION)
    set(BDEVERIFY_VS_VERSION "${LLVM_VERSION_MAJOR}.${LLVM_VERSION_MINOR}.${LLVM_VERSION_PATCH}.${BDEVERIFY_VSIX_BUILD}")
  endif()

  configure_file("source.extension.vsixmanifest.in"
      "${CMAKE_CURRENT_SOURCE_DIR}/BdeVerify/source.extension.vsixmanifest")

  find_program(NUGET_EXE nuget PATHS ${NUGET_EXE_DIR})
  if (NOT NUGET_EXE)
    message(FATAL_ERROR "Could not find nuget.exe. Download from https://www.nuget.org/nuget.exe"
                        " and add parent directory to PATH or pass it via NUGET_EXE_DIR var.")
  endif()

  add_custom_command(
      OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/BdeVerify/bin/Release/BdeVerify.vsix"
      COMMAND ${NUGET_EXE} restore "${CMAKE_CURRENT_SOURCE_DIR}/BdeVerify.sln"
      COMMAND devenv "${CMAKE_CURRENT_SOURCE_DIR}/BdeVerify.sln" /Build Release
      DEPENDS bde_verify_exe_for_vsix "${CMAKE_CURRENT_SOURCE_DIR}/BdeVerify/source.extension.vsixmanifest"
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "${CMAKE_CURRENT_SOURCE_DIR}/BdeVerify/bin/Release/BdeVerify.vsix"
      "${LLVM_TOOLS_BINARY_DIR}/BdeVerify.vsix"
      DEPENDS bde_verify_exe_for_vsix bde_verify_license)

  add_custom_target(bde_verify_vsix ALL
      DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/BdeVerify/bin/Release/BdeVerify.vsix")

  install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/BdeVerify/bin/Release/BdeVerify.vsix"
          DESTINATION share/clang
          COMPONENT bde-verify)
endif()
