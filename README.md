<p align="center">
  <img src="Icons/Dahlia%20Icon.svg" alt="Dahlia" width="200">
</p>

<h1 align="center">Dahlia</h1>

<p align="center">A cross-platform C++20 graphics engine with OpenGL, Vulkan and Metal backends.</p>

---

Dahlia puts one graphics API in front of three backends:

| Backend    | Version | Platforms              |
|------------|---------|------------------------|
| OpenGL     | 4.1     | All (always built)     |
| Vulkan     | 1.3     | Wherever a Vulkan SDK/loader is found |
| Metal      | -       | macOS                  |

Each backend follows OpenGL's conventions, so application code is the same whichever one is
used. Shaders are written once as Vulkan-style GLSL (`#version 450`) and compiled offline to SPIR-V
with `glslc`. At load time, [SPIRV-Cross](https://github.com/KhronosGroup/SPIRV-Cross) translates
them to GLSL 410 for OpenGL or MSL for Metal. The engine also includes
[Dear ImGui](https://github.com/ocornut/imgui) (docking branch) for UI, FreeType text rendering,
2D/3D cameras and a layer stack.

**DahliaInfo** is the demo application. It renders a scene and shows the active renderer, vendor,
shading language version, frame rate and camera controls.

## Requirements

- CMake 3.24 or newer, and Ninja
- A C++20 compiler (Clang/AppleClang, GCC or MSVC)
- `glslc`, from the [Vulkan SDK](https://vulkan.lunarg.com) or shaderc
- Boost (filesystem), GLFW 3, GLM, GLEW and FreeType
- Optional: the Vulkan SDK (Vulkan backend) and
  [metal-cpp](https://developer.apple.com/metal/cpp/) (Metal backend)

On macOS with Homebrew:

```sh
brew install cmake ninja shaderc boost glfw glm glew freetype vulkan-loader
```

## Getting the source

ImGui and SPIRV-Cross are git submodules:

```sh
git clone --recursive https://github.com/KingfisherA/GraphicsEngine.git Dahlia
# or, in an existing clone:
git submodule update --init --recursive
```

## Building

The build is always configured in `./build`:

```sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug   # or Release
cmake --build build
```

In VS Code, the CMake Tools extension uses `./build` automatically (see `.vscode/settings.json`).
The `Scripts/Configure_*` scripts configure the same directory for each platform.

Outputs:

| Path                       | Contents                                |
|----------------------------|-----------------------------------------|
| `build/Dahlia/`            | `libDahlia` (shared), `libDahliaTools`  |
| `build/DahliaInfo/`        | `DahliaInfo` executable                 |
| `build/DahliaInfo/DahliaInfo.app` | macOS app bundle (see below)     |

### Options

| Option              | Default                         | Description                         |
|---------------------|---------------------------------|-------------------------------------|
| `DH_ENABLE_VULKAN`  | `ON` if Vulkan is found         | Build the Vulkan backend            |
| `DH_ENABLE_METAL`   | `ON` on Apple platforms         | Build the Metal backend             |
| `DH_METAL_CPP_DIR`  | `/opt/metal-cpp_macOS26_iOS26-beta` | Location of the metal-cpp headers |
| `DH_BUILD_INFO_APP` | `ON` when Dahlia is the top-level project | Build DahliaInfo          |

For example: `cmake -S . -B build -DDH_ENABLE_VULKAN=OFF`.

### macOS app bundle

```sh
cmake --build build --target DahliaInfoApp
open build/DahliaInfo/DahliaInfo.app
```

This builds `DahliaInfo.app` with the Dahlia icon, a copy of `libDahlia.dylib` in
`Contents/Frameworks`, and an ad-hoc signature. Third-party libraries (GLFW, Boost, etc.) are still
loaded from their installed locations, so the bundle runs on machines that have those libraries
installed.

The icon, `Icons/Dahlia.icns`, is generated from `Icons/Dahlia Icon.svg`. To regenerate it after
editing the SVG, run `Scripts/MakeIcns.sh` (needs Inkscape or `librsvg`).

## Running

```sh
./build/DahliaInfo/DahliaInfo            # OpenGL 4.1 (default)
./build/DahliaInfo/DahliaInfo --vulkan   # Vulkan 1.3
./build/DahliaInfo/DahliaInfo --metal    # Metal
```

## Using Dahlia in another project

```cmake
add_subdirectory(Dahlia)            # builds only the library when Dahlia is not the top-level project
target_link_libraries(MyApp PRIVATE Dahlia)

# Optional: compile your own shaders to SPIR-V
dahlia_compile_shaders(MyApp SOURCES shaders/Mesh.vert shaders/Mesh.frag)
```

`dahlia_compile_shaders` is documented at the top of `cmake/DahliaShaders.cmake`.

## Repository layout

```
CMakeLists.txt          Top level: options, dependencies, subprojects
cmake/                  CMake helpers (shader compilation, target defaults, app bundle Info.plist)
src/Dahlia/             Engine sources and CMakeLists.txt
src/DahliaInfo/         Demo application and CMakeLists.txt
Shaders/                Engine shaders (Vulkan GLSL, compiled to SPIR-V at build time)
Libraries/              Git submodules (ImGui, SPIRV-Cross) and their build script
Icons/                  Dahlia icon (SVG source and generated .icns)
Scripts/                Configure and helper scripts
```
