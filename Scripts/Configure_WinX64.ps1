# Configures the build in .\build. Dependencies come from vcpkg (classic mode, VCPKG_ROOT set).
param ($config = "Debug")
$root = Resolve-Path "$PSScriptRoot/.."
cmake -DCMAKE_BUILD_TYPE="$config" -DCMAKE_TOOLCHAIN_FILE="${env:VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" -DVCPKG_TARGET_TRIPLET=x64-windows-static-md -S "$root" -B "$root/build"
