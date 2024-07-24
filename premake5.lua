workspace "GraphicsEngine"
	architecture "x64"

	configurations
	{
	"Debug",
	"Release"
	}

filter{"system:linux"}
	publicLibDir = "/media/charlie/Data/dev/Libraries/C++/"
	defines{"OS_LINUX"}

filter{"system:windows"}
	publicLibDir = "D:/dev/Libraries/"
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
		  publicLibDir .. "glm",
		  publicLibDir .. "ImGui"
		}
	
	filter{"system:windows"}
		defines {"GLEW_STATIC"}
		includedirs
		{
		  "%{wks.location}/src/",
		  publicLibDir .. "glfw-3.3.2/include",
		  publicLibDir .. "glew-2.1.0/include",
		  publicLibDir .. "glm",
		  publicLibDir .. "boost_1_83_0",
		  publicLibDir .. "ImGui",
		  publicLibDir
		}
	
	
	filter { "configurations:Debug"}
		symbols "On"
		optimize "Off"
		defines { "CONFIG_DEBUG" }
		buildoptions{"/MDd"}
		
	filter { "configurations:Release"}
		optimize "On"
		symbols "Off"
		defines { "CONFIG_RELEASE"}
		buildoptions{"/MD"}

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
		  publicLibDir .. "glm",
		  publicLibDir .. "ImGui"
		}
	
	
	filter{"system:windows"}
		defines{"GLEW_BUILD", "_GLFW_BUILD_DLL", "IMGUI_BUILD_DLL"}
		includedirs
		{
		  "%{wks.location}/src/Example",
		  "%{wks.location}/src/",
		  publicLibDir .. "glfw-3.3.2/include",
		  publicLibDir .. "glew-2.1.0/include",
		  publicLibDir .. "glm",
		  publicLibDir .. "boost_1_83_0",
		  publicLibDir .. "ImGui",
		  publicLibDir
		}
		libdirs{
		publicLibDir .. "boost_1_83_0/stage/lib/",
		publicLibDir .. "ImGui/bin/windows/Release-x64/",
		publicLibDir .. "glfw-3.3.2/lib/x64",
		publicLibDir .. "glew-2.1.0/lib/Release/x64"}
		links { "opengl32", "glew32s", "glfw3", "ImGui"}
		linkoptions { '/NODEFAULTLIB:"LIBCMTD" /NODEFAULTLIB:"LIBCMT"' }
	
	
	filter { "configurations:Debug"}
		symbols "On"
		optimize "Off"
		defines { "CONFIG_DEBUG" }
		buildoptions{"/MDd"}
		links {"libboost_filesystem-vc143-mt-gd-x64-1_83"}
		
	filter { "configurations:Release"}
		optimize "On"
		symbols "Off"
		defines { "CONFIG_RELEASE"}
		buildoptions{"/MD"}
		links{"libboost_filesystem-vc143-mt-x64-1_83"}


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
		  publicLibDir .. "glm",
		  publicLibDir .. "ImGui"
		}
		links { "glfw3", "ImGui", "boost_filesystem", "GLEW.a", "GL", "m", "Xrandr", "Xi", "X11", "Xxf86vm", "pthread", "nfd", "gtk-3", "z", "gdk-3", "glib-2.0", "GraphicsEngine" }
		--linkoptions { "`pkg-config --libs gtk+-3.0`" }
	
	filter{"system:windows"}
		defines {"GLEW_STATIC"}
		includedirs
		{
		  "%{wks.location}/src/Example",
		  "%{wks.location}/src/",
		  publicLibDir .. "glfw-3.3.2/include",
		  publicLibDir .. "glew-2.1.0/include",
		  publicLibDir .. "glm",
		  publicLibDir .. "boost_1_83_0",
		  publicLibDir .. "ImGui",
		  publicLibDir
		}
		libdirs{
		publicLibDir .. "boost_1_83_0/stage/lib/",
		publicLibDir .. "ImGui/bin/windows/Release-x64/",
		publicLibDir .. "glfw-3.3.2/lib/x64",
		publicLibDir .. "glew-2.1.0/lib/Release/x64"}
		links { "opengl32", "glew32s", "glfw3", "ImGui", "GraphicsEngine"}
		linkoptions { '/NODEFAULTLIB:"LIBCMTD" /NODEFAULTLIB:"LIBCMT"' }


	filter { "configurations:Debug" }
		defines { "CONFIG_DEBUG" }
		links {"libboost_filesystem-vc143-mt-gd-x64-1_83"}
		symbols "On"
		optimize "Off"

	filter { "configurations:Release" }
		defines { "CONFIG_RELEASE" }
		links {"libboost_filesystem-vc143-mt-x64-1_83"}
		optimize "On"
		symbols "Off"
