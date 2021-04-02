add_executable(wpa wpa.cpp)
target_link_libraries(wpa Svf Cudd ${llvm_libs})
set_target_properties(wpa PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
