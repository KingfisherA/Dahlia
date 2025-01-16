param ($config)
cmake -DCMAKE_BUILD_TYPE="$($config)" -DCMAKE_TOOLCHAIN_FILE="${env:VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" -DVCPKG_TARGET_TRIPLET=x64-windows-static-md -S ../ -B ../cmake-build-$($config.ToLower())-win
