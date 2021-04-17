add_executable(dvf dda.cpp)
target_link_libraries(dvf ${TOOL_LIBS})
set_target_properties(dvf PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
