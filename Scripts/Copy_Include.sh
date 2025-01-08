ORIGINAL_DIR="$(pwd)" && \
cd ../src && \
mkdir -p ../include && \
rm -r ../include/* || true && \
find ./GraphicsEngine -type f \( -name "*.h" -o -name "*.hpp" \) -exec cp --parents '{}' '../include' ';' && \
mkdir -p ../include/ImGui && \
cd ../Libraries/ImGui && \
find . -type f \( -name "*.h" -o -name "*.hpp" \) -exec cp --parents '{}' '../../include/ImGui' ';' && \
cd $ORIGINAL_DIR
