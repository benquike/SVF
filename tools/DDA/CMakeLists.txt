add_executable(dvf dda.cpp)
target_link_libraries(dvf Svf Cudd ${llvm_libs})
set_target_properties(dvf PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
