workspace "GraphicsEngine"
	architecture "x64"

	configurations
	{
	"Debug",
	"Release"
	}

	publicLibDir = "/media/charlie/Data/dev/Libraries/C++/"
	publicLibDirC = "/media/charlie/Data/dev/Libraries/C/"
	outputdir = "%{os.host()}/%{cfg.buildcfg}-x64/"
	libDir = "%{wks.location}/libs/"

project "GraphicsEngine"
	kind "StaticLib"
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
	
	includedirs
	{
	  "%{wks.location}/src/",
	  "/usr/include",
	  "/usr/local/include",
	  publicLibDir .. "glm",
	  publicLibDir .. "ImGui"
	}
	
	filter { "configurations:Debug"}
		symbols "On"
		defines { "CONFIG_DEBUG", "_DEBUG" }
		
	filter { "configurations:Debug"}
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
	
	files
	{
	  "%{wks.location}/src/Example/**.h",
	  "%{wks.location}/src/Example/**.hpp",
	  "%{wks.location}/src/Example/**.cpp"
	}

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

	filter { "configurations:Debug" }
		defines { "CONFIG_DEBUG", "_DEBUG" }
		symbols "On"

	filter { "configurations:Release" }
		defines { "CONFIG_RELEASE" }
		optimize "On"
		symbols "Off"
