MKDIR := 'mkdir -p'
RM := 'rm -rf ' + TMP_DIR
DOWN := 'curl -fsSL'
DOWN_OUT := '-o'

fetch-proto:
    {{MKDIR}} "{{TMP_DIR}}"
    {{DOWN}} "https://raw.githubusercontent.com/esclient/protos/{{PROTO_TAG}}/{{PROTO_NAME}}" {{DOWN_OUT}} "{{TMP_DIR}}/{{PROTO_NAME}}"

build-linux: fetch-proto
    conan install . --output-folder=build --build=missing
    cmake -S . -B build \
        -DCMAKE_TOOLCHAIN_FILE=build/conan_toolchain.cmake \
        -DCMAKE_BUILD_TYPE=Release
    cmake --build build --config Release

build-windows: fetch-proto
    conan install . --output-folder=build --build=missing
    cmake -S . -B build \
        -G "Visual Studio 17 2022" \
        -A x64 \
        -DCMAKE_TOOLCHAIN_FILE=build/conan_toolchain.cmake \
        -DCMAKE_BUILD_TYPE=Release
    cmake --build build --config Release

run-l: build-linux
    ENV=dev tools/load_envs.sh ./build/moderation-service

run-w: build-windows
    wsl ENV=dev tools/load_envs.sh ./build/moderation-service
