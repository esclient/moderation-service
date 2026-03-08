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
    conan install . --output-folder=build --build=missing -s compiler.cppstd=20
    cmake -S . -B build \
        -G "Visual Studio 17 2022" \
        -A x64 \
        -DCMAKE_TOOLCHAIN_FILE=build/conan_toolchain.cmake \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_POLICY_VERSION_MINIMUM=3.5
    cmake --build build --config Release

kafka-container-up:
    docker-compose up -d
    @echo "Waiting for Kafka to be ready..."
    @for i in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30; do \
        if docker exec kafka nc -z localhost 9092 2>/dev/null; then \
            echo "Kafka is ready!"; \
            break; \
        fi; \
        echo "Waiting for Kafka... attempt $$i/30"; \
        sleep 2; \
    done
    MSYS_NO_PATHCONV=1 docker exec kafka /opt/kafka/bin/kafka-topics.sh --bootstrap-server localhost:9092 --create --if-not-exists --topic moderation-result --replication-factor 1 --partitions 3
    MSYS_NO_PATHCONV=1 docker exec kafka /opt/kafka/bin/kafka-topics.sh --bootstrap-server localhost:9092 --create --if-not-exists --topic moderation-request --replication-factor 1 --partitions 3

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
    @which clang-format >/dev/null 2>&1 && echo "✓ clang-format" || echo "✗ clang-format"
    @which clang-tidy >/dev/null 2>&1 && echo "✓ clang-tidy" || echo "✗ clang-tidy"
    @which cppcheck >/dev/null 2>&1 && echo "✓ cppcheck" || echo "✗ cppcheck"
    @which cpplint >/dev/null 2>&1 && echo "✓ cpplint" || echo "✗ cpplint"
    @which cmake-format >/dev/null 2>&1 && echo "✓ cmake-format" || echo "✗ cmake-format"
    @which pipx >/dev/null 2>&1 && echo "✓ pipx" || echo "✗ pipx"

install-dev-tools:
    chmod +x ./tools/install_dev_tools.sh
    ./tools/install_dev_tools.sh

upgrade-python-tools:
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
