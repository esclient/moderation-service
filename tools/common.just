MKDIR := 'mkdir -p'
RM := 'rm -rf ' + TMP_DIR
DOWN := 'curl -fsSL'
DOWN_OUT := '-o'
EXE := if os() == "windows" { ".exe" } else { "" }
TEST_DIR := if os() == "windows" { "./build/tests/Release" } else { "./build/tests" }

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

kafka-container-up:
    docker-compose up -d
    MSYS_NO_PATHCONV=1 docker exec kafka /opt/kafka/bin/kafka-topics.sh --bootstrap-server localhost:9092 --create --topic moderation-result --replication-factor 1 --partitions 3
    MSYS_NO_PATHCONV=1 docker exec kafka /opt/kafka/bin/kafka-topics.sh --bootstrap-server localhost:9092 --create --topic moderation-request --replication-factor 1 --partitions 3

kafka-container-down:
    MSYS_NO_PATHCONV=1 docker exec kafka /opt/kafka/bin/kafka-topics.sh --bootstrap-server localhost:9092 --delete --topic moderation-request
    MSYS_NO_PATHCONV=1 docker exec kafka /opt/kafka/bin/kafka-topics.sh --bootstrap-server localhost:9092 --delete --topic moderation-result
    docker-compose down -vv

configure-lint: fetch-proto
    conan install . --output-folder=build-lint --build=missing -c tools.cmake.cmaketoolchain:generator=Ninja
    cmake -S . -B build-lint \
        -G "Ninja" \
        -DCMAKE_TOOLCHAIN_FILE=build-lint/conan_toolchain.cmake \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
        -DENABLE_CLANG_TIDY=ON

format:
    cmake --build build-lint --target clang-format
    cmake --build build-lint --target cmake-format

clean-lint:
    rm -rf build-lint

lint-all-linux:
    just configure-lint
    just format
    cmake --build build-lint
    cmake --build build-lint --target lint-all 

run-l:
    just kafka-container-up
    just build-linux
    ENV=dev tools/load_envs.sh ./build/moderation-service

run-w: 
    just kafka-container-up
    just build-windows
    ENV=dev tools/load_envs.sh ./build/Release/moderation-service.exe

check-dev-tools:
    @echo "Checking linting tools:"
    @which clang-format >/dev/null 2>&1 && echo "[OK] clang-format" || echo "[X] clang-format"
    @which clang-tidy >/dev/null 2>&1 && echo "[OK] clang-tidy" || echo "[X] clang-tidy"
    @which cppcheck >/dev/null 2>&1 && echo "[OK] cppcheck" || echo "[X] cppcheck"
    @which cpplint >/dev/null 2>&1 && echo "[OK] cpplint" || echo "[X] cpplint"
    @which cmake-format >/dev/null 2>&1 && echo "[OK] cmake-format" || echo "[X] cmake-format"
    @which pipx >/dev/null 2>&1 && echo "[OK] pipx" || echo "[X] pipx"

install-dev-tools:
    chmod +x ./tools/install_dev_tools.sh
    ./tools/install_dev_tools.sh

upgrade-python-tools:
    @which pipx >/dev/null 2>&1 || (echo "[X] pipx not found. Run 'just install-dev-tools' first." && exit 1)
    pipx upgrade cpplint
    pipx upgrade cmakelang

test:
    {{TEST_DIR}}/fake_repository_test{{EXE}}
    {{TEST_DIR}}/handler_test{{EXE}}
    {{TEST_DIR}}/moderation_service_test{{EXE}}
    {{TEST_DIR}}/text_normalization_test{{EXE}}
    {{TEST_DIR}}/text_processor_test{{EXE}}
    {{TEST_DIR}}/word_checker_test{{EXE}}

benchmark:
    just build-windows
    cmake -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_BENCHMARKS=ON
    cmake --build build --target text_processing_benchmark --config Release
    cmake --build build --target text_processing_benchmark --config Release && \
    (./build/benchmark/Release/text_processing_benchmark.exe || \
     ./build/benchmark/text_processing_benchmark)
