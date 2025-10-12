ORIGINAL_DIR="$(pwd)" && \
cd ../src && \
mkdir -p ../include && \
rm -r ../include/* || true && \
find ./Dahlia -type f \( -name "*.h" -o -name "*.hpp" \) -exec /opt/homebrew/Cellar/coreutils/9.8/bin/gcp --parents '{}' '../include' \; && \
mkdir -p ../include/ImGui && \
cd ../Libraries/ImGui && \
find . -type f \( -name "*.h" -o -name "*.hpp" \) -exec /opt/homebrew/Cellar/coreutils/9.8/bin/gcp --parents '{}' '../../include/ImGui' \; && \
cd $ORIGINAL_DIR
