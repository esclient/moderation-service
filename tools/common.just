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
    mkdir -p build
    cd build && cmake .. && cmake --build .

rebuild:
    rm -rf build
    mkdir -p build
    cd build && cmake .. && cmake --build .


run: build
    ENV=dev tools/load_envs.sh ./build/moderation-service

run-new: rebuild
    ENV=dev tools/load_envs.sh ./build/moderation-service

