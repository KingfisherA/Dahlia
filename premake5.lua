workspace "GraphicsEngine"
	architecture "x64"

	configurations
	{
	"Debug",
	"Release"
	}

filter{"system:linux"}
	defines{"OS_LINUX"}

filter{"system:windows"}
	defines{"OS_WIN"}
	outputdir = "%{os.host()}/%{cfg.buildcfg}-x64/%{prj.name}"
	libDir = "%{wks.location}/libs/"

project "GraphicsEngine"
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"
	systemversion "latest"
	
	targetdir("bin/" .. outputdir)
	objdir("bin-obj/" .. outputdir)
	debugdir("bin/" .. outputdir)
	
	defines{"GENG_STATIC","GENG_BUILD"}
	
	files
	{
	"%{wks.location}/src/GraphicsEngine/**.h",
	"%{wks.location}/src/GraphicsEngine/**.hpp",
  	"%{wks.location}/src/GraphicsEngine/**.cpp",
	}
	
	filter{"system:linux"}
		includedirs
		{
		  "%{wks.location}/src/",
		  "/usr/include",
		  "/usr/local/include",
		  "../../Libraries/glm",
		  "../../Libraries/ImGui",
          "../../Libraries/",
          "/usr/include/freetype2",
          "/usr/include/libpng16"
		}
	
	filter{"system:windows"}
		defines {"GLEW_STATIC"}
		includedirs
		{
		  "%{wks.location}/src/",
		  "../../Libraries/glfw-3.3.2/include",
		  "../../Libraries/glew-2.1.0/include",
		  "../../Libraries/glm",
		  "../../Libraries/boost_1_83_0",
		  "../../Libraries/ImGui",
		  "../../Libraries/freetype-2.13.3/include",
		  "../../Libraries/"
		}

    filter {"configurations:Debug", "system:windows"}
        buildoptions{"/MDd"}

    filter {"configurations:Release", "system:windows"}
        buildoptions{"/MD"}
	
	
	filter { "configurations:Debug"}
		symbols "On"
		optimize "Off"
		defines { "CONFIG_DEBUG" }
		
		
	filter { "configurations:Release"}
		optimize "On"
		symbols "Off"
		defines { "CONFIG_RELEASE"}

project "GraphicsEngine_Shared"
	kind "SharedLib"
	language "C++"
	cppdialect "C++20"
	systemversion "latest"
	
	targetdir("bin/" .. outputdir)
	objdir("bin-obj/" .. outputdir)
	debugdir("bin/" .. outputdir)
	
	files
	{
	"%{wks.location}/src/GraphicsEngine/**.h",
	"%{wks.location}/src/GraphicsEngine/**.hpp",
  	"%{wks.location}/src/GraphicsEngine/**.cpp",
	}
	
	defines{"GENG_BUILD"}
	
	filter{"system:linux"}
		includedirs
		{
		  "%{wks.location}/src/",
		  "/usr/include",
		  "/usr/local/include",
		  "../../Libraries/glm",
		  "../../Libraries/ImGui",
          "../../Libraries/",
          "/usr/include/freetype2",
          "/usr/include/libpng16"
		}
        libdirs{
        "../../Libraries/ImGui/bin/linux/Release-x64"}
		links { "glfw3", "ImGui", "boost_filesystem", "GLEW.a", "GL", "m", "Xrandr", "Xi", "X11", "Xxf86vm", "pthread", "gtk-3", "z", "gdk-3", "glib-2.0" }
	
	
	filter{"system:windows"}
		defines{"GLEW_BUILD", "_GLFW_BUILD_DLL", "IMGUI_BUILD_DLL"}
		includedirs
		{
		  "%{wks.location}/src/Example",
		  "%{wks.location}/src/",
		  "../../Libraries/glfw-3.3.2/include",
		  "../../Libraries/glew-2.1.0/include",
		  "../../Libraries/glm",
		  "../../Libraries/boost_1_83_0",
		  "../../Libraries/ImGui",
		  "../../Libraries/freetype-2.13.3/include",
		  "../../Libraries/"
		}
		libdirs
        {
		    "../../Libraries/boost_1_83_0/stage/lib/",
		    "../../Libraries/ImGui/bin/windows/Release-x64/",
		    "../../Libraries/glfw-3.3.2/lib/x64",
		    "../../Libraries/glew-2.1.0/lib/Release/x64",
			"C:/Program Files (x86)/Windows Kits/10/Lib/10.0.22000.0/um/x64",
			"../../Libraries/freetype-2.13.3/objs",
        }
		links { "OpenGL32.lib", "glew32s", "glfw3", "freetype.lib", "ImGui"}
		linkoptions { '/NODEFAULTLIB:"LIBCMTD" /NODEFAULTLIB:"LIBCMT"' }
	
    filter {"configurations:Debug", "system:windows"}
        buildoptions{"/MDd"}
        links {"libboost_filesystem-vc143-mt-gd-x64-1_83"}

    filter {"configurations:Release", "system:windows"}
        buildoptions{"/MD"}
        links{"libboost_filesystem-vc143-mt-x64-1_83"}
	
	filter { "configurations:Debug"}
		symbols "On"
		optimize "Off"
		defines { "CONFIG_DEBUG" }
		
	filter { "configurations:Release"}
		optimize "On"
		symbols "Off"
		defines { "CONFIG_RELEASE"}


project "Example"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	systemversion "latest"

	targetdir("bin/" .. outputdir)
	objdir("bin-obj/" .. outputdir)
	debugdir("bin/" .. outputdir)
	
	defines {"GENG_STATIC"}
	
	files
	{
	  "%{wks.location}/src/Example/**.h",
	  "%{wks.location}/src/Example/**.hpp",
	  "%{wks.location}/src/Example/**.cpp"
	}

	filter{"system:linux"}
		includedirs
		{
		  "%{wks.location}/src/Example",
		  "%{wks.location}/src/",
		  "/usr/include",
		  "/usr/local/include",
		  "../../Libraries/glm",
		  "../../Libraries/ImGui",
          "../../Libraries/",
          "/usr/include/freetype2",
          "/usr/include/libpng16"
		}
        libdirs{
        "../../Libraries/ImGui/bin/linux/Release-x64"}
		links { "glfw3", "ImGui", "freetype", "boost_filesystem", "GLEW.a", "GL", "m", "Xrandr", "Xi", "X11", "Xxf86vm", "pthread", "gtk-3", "z", "gdk-3", "glib-2.0", "GraphicsEngine" }
		--linkoptions { "`pkg-config --libs gtk+-3.0`" }
	
	filter{"system:windows"}
		defines {"GLEW_STATIC"}
		includedirs
		{
		  "%{wks.location}/src/Example",
		  "%{wks.location}/src/",
		  "../../Libraries/glfw-3.3.2/include",
		  "../../Libraries/glew-2.1.0/include",
		  "../../Libraries/glm",
		  "../../Libraries/boost_1_83_0",
		  "../../Libraries/ImGui",
		  "../../Libraries/freetype-2.13.3/include",
		  "../../Libraries/"
		}
		libdirs{
		"../../Libraries/boost_1_83_0/stage/lib/",
		"../../Libraries/ImGui/bin/windows/Release-x64/",
		"../../Libraries/glfw-3.3.2/lib/x64",
		"../../Libraries/glew-2.1.0/lib/Release/x64",
		"../../Libraries/freetype-2.13.3/objs"}
		links { "opengl32", "glew32s", "glfw3", "ImGui", "freetype", "GraphicsEngine"}
		linkoptions { '/NODEFAULTLIB:"LIBCMTD" /NODEFAULTLIB:"LIBCMT"' }

    filter {"configurations:Debug", "system:windows"}
        buildoptions{"/MDd"}
        links {"libboost_filesystem-vc143-mt-gd-x64-1_83"}

    filter {"configurations:Release", "system:windows"}
        buildoptions{"/MD"}
        links{"libboost_filesystem-vc143-mt-x64-1_83"}

	filter { "configurations:Debug" }
		defines { "CONFIG_DEBUG" }
		symbols "On"
		optimize "Off"

	filter { "configurations:Release" }
		defines { "CONFIG_RELEASE" }
		optimize "On"
		symbols "Off"
