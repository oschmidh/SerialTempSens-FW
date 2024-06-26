cmake_minimum_required(VERSION 3.13.1)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(app LANGUAGES CXX)

target_include_directories(app PRIVATE include)

if (CMAKE_HOST_WIN32)
    set(PROTC_PLUGIN protoc-gen-eams=protoc-gen-eams.bat)
else()
    set(PROTC_PLUGIN protoc-gen-eams)
endif()

execute_process(
COMMAND
    protoc
    --plugin=${PROTC_PLUGIN}
    -I${CMAKE_CURRENT_SOURCE_DIR}/../proto
    --eams_out=${CMAKE_CURRENT_BINARY_DIR}/zephyr/include/generated
    SerialTempSens_messages.proto
WORKING_DIRECTORY
    ${ZEPHYR_BASE}/../modules/EmbeddedProto
ERROR_VARIABLE
    PROTOC_ERROR
RESULT_VARIABLE
    PROTOC_STATUS
)

if(NOT PROTOC_STATUS EQUAL 0)
    message(SEND_ERROR "Failed to generate protobuf headers: ${PROTOC_ERROR}")
endif()

target_link_libraries(app
PUBLIC
    embeddedProto
    myLib
)

target_sources(app
PRIVATE
    src/main.cpp
    src/SensorService.cpp
)
