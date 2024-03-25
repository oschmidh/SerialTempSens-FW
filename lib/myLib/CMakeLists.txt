add_library(myLib INTERFACE)

set_target_properties(myLib PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(myLib INTERFACE include)
