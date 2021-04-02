add_executable(svf-ex svf-ex.cpp)
target_link_libraries(svf-ex Svf Cudd ${llvm_libs})
set_target_properties( svf-ex PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
