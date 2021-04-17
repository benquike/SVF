add_executable(chrome-gl-analysis chrome-gl-analysis.cpp)
target_link_libraries(chrome-gl-analysis ${TOOL_LIBS})
set_target_properties(chrome-gl-analysis PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
