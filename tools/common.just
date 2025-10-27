MKDIR := 'mkdir -p'
RM := 'rm -rf ' + TMP_DIR
DOWN := 'curl -fsSL'
DOWN_OUT := '-o'

clean:
    {{RM}}

fetch-proto:
    {{MKDIR}} "{{TMP_DIR}}"
    {{DOWN}} "https://raw.githubusercontent.com/esclient/protos/{{PROTO_TAG}}/{{PROTO_NAME}}" {{DOWN_OUT}} "{{TMP_DIR}}/{{PROTO_NAME}}"

gen-stubs: fetch-proto
    {{MKDIR}} "{{OUT_DIR}}"
    protoc \
      --proto_path="{{TMP_DIR}}" \
      --cpp_out="{{OUT_DIR}}" \
      --grpc_out="{{OUT_DIR}}" \
      --plugin=protoc-gen-grpc=$(which grpc_cpp_plugin) \
      "{{TMP_DIR}}/{{PROTO_NAME}}"

update: gen-stubs clean

build:
    @cmake -S . -B build -DCMAKE_BUILD_TYPE=Release 2>/dev/null || true
    @cmake --build build -j$(nproc)
    
rebuild:
    #!/usr/bin/env bash
    if [ -d "build" ]; then
        chmod -R u+w build 2>/dev/null || true
        rm -rf build
    fi
    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
    cmake --build build -j$(nproc)

reconfigure:
    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
    cmake --build build -j$(nproc) --clean-first

run: build
    ENV=dev tools/load_envs.sh ./build/moderation-service

dev: 
    @cmake --build build -j$(nproc)
    ENV=dev tools/load_envs.sh ./build/moderation-service

