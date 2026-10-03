add_library(librules STATIC rules.h rules.cpp)

# SYSTEM: the server's own warning flags (ENABLE_WARNINGS) must not report the vendored header.
target_include_directories(librules SYSTEM PRIVATE "${PROJECT_SOURCE_DIR}/vendor/tomlplusplus-3.4.0")
