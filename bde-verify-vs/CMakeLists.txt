option(BUILD_BDE_VERIFY_VS_PLUGIN "Build bde-verify VS plugin" OFF)
if (BUILD_BDE_VERIFY_VS_PLUGIN)
  add_custom_target(bde_verify_exe_for_vsix
      ${CMAKE_COMMAND} -E copy_if_different
      "${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}/bde_verify_bin.exe"
      "${CMAKE_CURRENT_SOURCE_DIR}/BdeVerify/bde_verify_bin.exe"
      DEPENDS bde-verify)

  add_custom_target(bde_verify_license
      ${CMAKE_COMMAND} -E copy_if_different
      "${CMAKE_CURRENT_SOURCE_DIR}/../LICENSE"
      "${CMAKE_CURRENT_SOURCE_DIR}/BdeVerify/license.txt")

  add_custom_target(bde_verify_config
      ${CMAKE_COMMAND} -E copy_if_different
      "${CMAKE_CURRENT_SOURCE_DIR}/../bde_verify.cfg"
      "${CMAKE_CURRENT_SOURCE_DIR}/BdeVerify/bde_verify.cfg")

  # Build number added to version to ensure that new VSIX can be upgraded
  string(TIMESTAMP BDE_VERIFY_VSIX_BUILD %y%m%d%H%M UTC)

  if (NOT BDE_VERIFY_VS_VERSION)
    set(BDE_VERIFY_VS_VERSION "${BDE_VERIFY_VERSION}.${BDE_VERIFY_VSIX_BUILD}")
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
      "${CMAKE_CURRENT_BINARY_DIR}/BdeVerify.vsix"
      DEPENDS bde_verify_exe_for_vsix bde_verify_license bde_verify_config)

  add_custom_target(bde_verify_vsix ALL
      DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/BdeVerify/bin/Release/BdeVerify.vsix")

  install(FILES "${CMAKE_CURRENT_BINARY_DIR}/BdeVerify.vsix"
          DESTINATION share/bde-verify)
endif()
