add_executable(wpa wpa.cpp)
target_link_libraries(wpa ${TOOL_LIBS})
set_target_properties(wpa PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
