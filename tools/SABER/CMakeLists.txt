add_executable(saber saber.cpp)
target_link_libraries(saber Svf Cudd ${llvm_libs})
set_target_properties(saber PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
