add_executable("Example"
  "src/Example/Application.cpp"
  "src/Example/Application.h"
)
if(CMAKE_BUILD_TYPE STREQUAL Debug)
  add_dependencies("Example"
    "GraphicsEngine"
  )
  set_target_properties("Example" PROPERTIES
    OUTPUT_NAME "Example"
    ARCHIVE_OUTPUT_DIRECTORY "/media/charlie/Data/dev/C++/GraphicsEngine/bin/linux/Debug-x64"
    LIBRARY_OUTPUT_DIRECTORY "/media/charlie/Data/dev/C++/GraphicsEngine/bin/linux/Debug-x64"
    RUNTIME_OUTPUT_DIRECTORY "/media/charlie/Data/dev/C++/GraphicsEngine/bin/linux/Debug-x64"
  )
endif()
target_include_directories("Example" PRIVATE
  $<$<CONFIG:Debug>:/media/charlie/Data/dev/C++/GraphicsEngine/src/Example>
  $<$<CONFIG:Debug>:/media/charlie/Data/dev/C++/GraphicsEngine/src>
  $<$<CONFIG:Debug>:/usr/include>
  $<$<CONFIG:Debug>:/usr/local/include>
  $<$<CONFIG:Debug>:/media/charlie/Data/dev/Libraries/glm>
  $<$<CONFIG:Debug>:/media/charlie/Data/dev/Libraries/ImGui>
  $<$<CONFIG:Debug>:/media/charlie/Data/dev/Libraries>
  $<$<CONFIG:Debug>:/usr/include/freetype2>
  $<$<CONFIG:Debug>:/usr/include/libpng16>
)
target_compile_definitions("Example" PRIVATE
  $<$<CONFIG:Debug>:OS_LINUX>
  $<$<CONFIG:Debug>:GENG_STATIC>
  $<$<CONFIG:Debug>:CONFIG_DEBUG>
)
target_link_directories("Example" PRIVATE
  $<$<CONFIG:Debug>:/media/charlie/Data/dev/Libraries/ImGui/bin/linux/Release-x64>
)
target_link_libraries("Example"
  $<$<CONFIG:Debug>:GraphicsEngine>
)
target_compile_options("Example" PRIVATE
  $<$<AND:$<CONFIG:Debug>,$<COMPILE_LANGUAGE:C>>:-m64>
  $<$<AND:$<CONFIG:Debug>,$<COMPILE_LANGUAGE:C>>:-O0>
  $<$<AND:$<CONFIG:Debug>,$<COMPILE_LANGUAGE:C>>:-g>
  $<$<AND:$<CONFIG:Debug>,$<COMPILE_LANGUAGE:CXX>>:-m64>
  $<$<AND:$<CONFIG:Debug>,$<COMPILE_LANGUAGE:CXX>>:-O0>
  $<$<AND:$<CONFIG:Debug>,$<COMPILE_LANGUAGE:CXX>>:-g>
  $<$<AND:$<CONFIG:Debug>,$<COMPILE_LANGUAGE:CXX>>:-std=c++20>
)
if(CMAKE_BUILD_TYPE STREQUAL Debug)
  set_target_properties("Example" PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    POSITION_INDEPENDENT_CODE False
    INTERPROCEDURAL_OPTIMIZATION False
  )
endif()
if(CMAKE_BUILD_TYPE STREQUAL Release)
  add_dependencies("Example"
    "GraphicsEngine"
  )
  set_target_properties("Example" PROPERTIES
    OUTPUT_NAME "Example"
    ARCHIVE_OUTPUT_DIRECTORY "/media/charlie/Data/dev/C++/GraphicsEngine/bin/linux/Release-x64"
    LIBRARY_OUTPUT_DIRECTORY "/media/charlie/Data/dev/C++/GraphicsEngine/bin/linux/Release-x64"
    RUNTIME_OUTPUT_DIRECTORY "/media/charlie/Data/dev/C++/GraphicsEngine/bin/linux/Release-x64"
  )
endif()
target_include_directories("Example" PRIVATE
  $<$<CONFIG:Release>:/media/charlie/Data/dev/C++/GraphicsEngine/src/Example>
  $<$<CONFIG:Release>:/media/charlie/Data/dev/C++/GraphicsEngine/src>
  $<$<CONFIG:Release>:/usr/include>
  $<$<CONFIG:Release>:/usr/local/include>
  $<$<CONFIG:Release>:/media/charlie/Data/dev/Libraries/glm>
  $<$<CONFIG:Release>:/media/charlie/Data/dev/Libraries/ImGui>
  $<$<CONFIG:Release>:/media/charlie/Data/dev/Libraries>
  $<$<CONFIG:Release>:/usr/include/freetype2>
  $<$<CONFIG:Release>:/usr/include/libpng16>
)
target_compile_definitions("Example" PRIVATE
  $<$<CONFIG:Release>:OS_LINUX>
  $<$<CONFIG:Release>:GENG_STATIC>
  $<$<CONFIG:Release>:CONFIG_RELEASE>
)
target_link_directories("Example" PRIVATE
  $<$<CONFIG:Release>:/media/charlie/Data/dev/Libraries/ImGui/bin/linux/Release-x64>
)
target_link_libraries("Example"
  $<$<CONFIG:Release>:GraphicsEngine>
)
target_compile_options("Example" PRIVATE
  $<$<AND:$<CONFIG:Release>,$<COMPILE_LANGUAGE:C>>:-m64>
  $<$<AND:$<CONFIG:Release>,$<COMPILE_LANGUAGE:C>>:-O2>
  $<$<AND:$<CONFIG:Release>,$<COMPILE_LANGUAGE:CXX>>:-m64>
  $<$<AND:$<CONFIG:Release>,$<COMPILE_LANGUAGE:CXX>>:-O2>
  $<$<AND:$<CONFIG:Release>,$<COMPILE_LANGUAGE:CXX>>:-std=c++20>
)
if(CMAKE_BUILD_TYPE STREQUAL Release)
  set_target_properties("Example" PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    POSITION_INDEPENDENT_CODE False
    INTERPROCEDURAL_OPTIMIZATION False
  )
endif()