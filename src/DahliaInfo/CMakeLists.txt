# DahliaInfo demo application, built into build/DahliaInfo
#
#   DahliaInfo      plain executable (default build)
#   DahliaInfoApp   macOS only: DahliaInfo.app with the Dahlia icon and libDahlia embedded
#                   (cmake --build build --target DahliaInfoApp)

#   dahlia_add_info_executable(<target> [add_executable options and extra sources...])
function(dahlia_add_info_executable TARGET)
  add_executable(${TARGET} ${ARGN}
    Application.cpp
    Application.h
  )
  target_include_directories(${TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
  target_link_libraries(${TARGET} PRIVATE
    Dahlia
    ImGui
    DahliaTools
  )
  dahlia_target_defaults(${TARGET})
endfunction()

# ---- DahliaInfo -----------------------------------------------------------------------------------
dahlia_add_info_executable(DahliaInfo)

if(WIN32)
  # Windows has no rpath: put Dahlia.dll (and any other runtime DLLs) next to the executable
  add_custom_command(TARGET DahliaInfo POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_RUNTIME_DLLS:DahliaInfo> $<TARGET_FILE_DIR:DahliaInfo>
    COMMAND_EXPAND_LISTS
  )
endif()

# ---- DahliaInfo.app -------------------------------------------------------------------------------
if(APPLE)
  set(DH_INFO_ICON ${PROJECT_SOURCE_DIR}/Icons/Dahlia.icns)
  set_source_files_properties(${DH_INFO_ICON} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

  dahlia_add_info_executable(DahliaInfoApp MACOSX_BUNDLE EXCLUDE_FROM_ALL ${DH_INFO_ICON})
  set_target_properties(DahliaInfoApp PROPERTIES
    OUTPUT_NAME DahliaInfo
    MACOSX_BUNDLE_INFO_PLIST ${PROJECT_SOURCE_DIR}/cmake/DahliaInfo.plist.in
    MACOSX_BUNDLE_BUNDLE_NAME DahliaInfo
    MACOSX_BUNDLE_GUI_IDENTIFIER com.kingfishera.DahliaInfo
    MACOSX_BUNDLE_ICON_FILE Dahlia.icns
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    # Load libDahlia from the bundle's Frameworks folder rather than the build tree
    BUILD_WITH_INSTALL_RPATH ON
    INSTALL_RPATH @executable_path/../Frameworks
  )

  set(DH_BUNDLE_FRAMEWORKS $<TARGET_BUNDLE_CONTENT_DIR:DahliaInfoApp>/Frameworks)
  add_custom_command(TARGET DahliaInfoApp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${DH_BUNDLE_FRAMEWORKS}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Dahlia> ${DH_BUNDLE_FRAMEWORKS}
    # Ad-hoc sign the bundle so its contents (Info.plist, icon, embedded dylib) are sealed
    COMMAND codesign --force --sign - $<TARGET_BUNDLE_DIR:DahliaInfoApp>
    VERBATIM
  )
endif()
