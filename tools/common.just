MKDIR := 'mkdir -p'
RM := 'rm -rf ' + TMP_DIR
DOWN := 'curl -fsSL'
DOWN_OUT := '-o'

clean:
    {{RM}}

fetch-proto:
    {{MKDIR}} "{{TMP_DIR}}"
    {{DOWN}} "https://raw.githubusercontent.com/esclient/protos/{{PROTO_TAG}}/{{PROTO_NAME}}" {{DOWN_OUT}} "{{TMP_DIR}}/{{PROTO_NAME}}"

run:
    ENV=dev PATH=src ./tools/load_envs.sh pdm run run-server

gen-stubs: fetch-proto
    {{MKDIR}} "{{OUT_DIR}}"
    protoc \
      --proto_path="{{TMP_DIR}}" \
      --cpp_out="{{OUT_DIR}}" \
      --grpc_out="{{OUT_DIR}}" \
      --plugin=protoc-gen-grpc=$(which grpc_cpp_plugin) \
      "{{TMP_DIR}}/{{PROTO_NAME}}"

update: gen-stubs clean

cmake-build:
    cd build \
    cmake .. \
    cmake --build . \

cmake-launch: 
    ./moderation-service


