add_library(embeddedProto STATIC)

set_target_properties(embeddedProto PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(embeddedProto INTERFACE ${CMAKE_SOURCE_DIR}/../../modules/EmbeddedProto/src)

target_sources(embeddedProto
PRIVATE
    ${CMAKE_SOURCE_DIR}/../../modules/EmbeddedProto/src/Fields.cpp
    ${CMAKE_SOURCE_DIR}/../../modules/EmbeddedProto/src/MessageInterface.cpp
    ${CMAKE_SOURCE_DIR}/../../modules/EmbeddedProto/src/ReadBufferSection.cpp
)

target_link_libraries(embeddedProto zephyr_interface)
