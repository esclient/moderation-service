MKDIR := 'mkdir -p'
RM := 'rm -rf ' + TMP_DIR
DOWN := 'curl -fsSL'
DOWN_OUT := '-o'
EXE := if os() == "windows" { ".exe" } else { "" }
TEST_DIR := if os() == "windows" { "./build/tests/Release" } else { "./build/tests" }

CACHE_TOOL := `which sccache 2>/dev/null || which ccache 2>/dev/null || echo ""`

# Parallel jobs for Linux (nproc with getconf fallback)
JOBS := if os() == "windows" { "" } else { `nproc 2>/dev/null || getconf _NPROCESSORS_ONLN 2>/dev/null || echo 4` }
PARALLEL := if os() == "windows" { "" } else { "-j" + JOBS }

fetch-proto:
    {{MKDIR}} "{{TMP_DIR}}"
    {{DOWN}} "https://raw.githubusercontent.com/esclient/protos/{{PROTO_TAG}}/{{PROTO_NAME}}" {{DOWN_OUT}} "{{TMP_DIR}}/{{PROTO_NAME}}"

build-linux: fetch-proto
    conan install . --output-folder=build --build=missing --settings compiler.cppstd=20
    cmake -S . -B build \
        -DCMAKE_TOOLCHAIN_FILE=build/conan_toolchain.cmake \
        -DCMAKE_BUILD_TYPE=Release \
        {{ if CACHE_TOOL != "" { "-DCMAKE_C_COMPILER_LAUNCHER=" + CACHE_TOOL + " -DCMAKE_CXX_COMPILER_LAUNCHER=" + CACHE_TOOL } else { "" } }}
    cmake --build build --config Release -- {{PARALLEL}}

build-windows: fetch-proto
    conan install . --output-folder=build --build=missing --settings compiler.cppstd=20
    cmake -S . -B build \
        -G "Visual Studio 17 2022" \
        -A x64 \
        -DCMAKE_TOOLCHAIN_FILE=build/conan_toolchain.cmake \
        -DCMAKE_BUILD_TYPE=Release
    cmake --build build --config Release

configure-lint: fetch-proto
    conan install . --output-folder=build-lint --build=missing -c tools.cmake.cmaketoolchain:generator=Ninja
    cmake -S . -B build-lint \
        -G "Ninja" \
        -DCMAKE_TOOLCHAIN_FILE=build-lint/conan_toolchain.cmake \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
        -DENABLE_CLANG_TIDY=ON \
        {{ if CACHE_TOOL != "" { "-DCMAKE_C_COMPILER_LAUNCHER=" + CACHE_TOOL + " -DCMAKE_CXX_COMPILER_LAUNCHER=" + CACHE_TOOL } else { "" } }}

format:
    cmake --build build-lint --target clang-format -- {{PARALLEL}}
    cmake --build build-lint --target cmake-format

lint-all-linux:
    just configure-lint
    just format
    cmake --build build-lint -- {{PARALLEL}}
    cmake --build build-lint --target lint-all -- {{PARALLEL}}

kafka-container-up:
    docker-compose up -d
    sleep 10
    MSYS_NO_PATHCONV=1 docker exec kafka /opt/kafka/bin/kafka-topics.sh --bootstrap-server localhost:9092 --create --topic moderation-result --replication-factor 1 --partitions 3
    MSYS_NO_PATHCONV=1 docker exec kafka /opt/kafka/bin/kafka-topics.sh --bootstrap-server localhost:9092 --create --topic moderation-request --replication-factor 1 --partitions 3

kafka-container-down:
    MSYS_NO_PATHCONV=1 docker exec kafka /opt/kafka/bin/kafka-topics.sh --bootstrap-server localhost:9092 --delete --topic moderation-request
    MSYS_NO_PATHCONV=1 docker exec kafka /opt/kafka/bin/kafka-topics.sh --bootstrap-server localhost:9092 --delete --topic moderation-result
    docker-compose down -vv

clean-lint:
    rm -rf build-lint

run-l:
    just build-linux
    ENV=dev tools/load_envs.sh ./build/moderation-service

run-w:
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
    @echo ""
    @echo "Checking compiler cache:"
    @which sccache >/dev/null 2>&1 && echo "[OK] sccache (preferred)" || echo "[X] sccache"
    @which ccache >/dev/null 2>&1 && echo "[OK] ccache (fallback)" || echo "[X] ccache"
    @echo "Active cache tool: {{CACHE_TOOL}}"

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