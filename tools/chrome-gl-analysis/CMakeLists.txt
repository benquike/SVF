
if(DEFINED IN_SOURCE_BUILD)
    set(LLVM_LINK_COMPONENTS BitWriter Core IPO IrReader InstCombine Instrumentation Target Linker Analysis ScalarOpts Support Svf Cudd)
    add_llvm_tool(chrome-gl-analysis chrome-gl-analysis.cpp )
else()
    if (LLVM_LINK_LLVM_DYLIB)
        set(llvm_libs LLVM)
    else()
        llvm_map_components_to_libnames(llvm_libs BitWriter Core IPO IrReader InstCombine Instrumentation Target Linker Analysis ScalarOpts Support TransformUtils)
    endif()
    add_executable(chrome-gl-analysis chrome-gl-analysis.cpp )

    target_link_libraries(chrome-gl-analysis Svf Cudd ${llvm_libs} )

    set_target_properties(chrome-gl-analysis PROPERTIES
      RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin )
    # set_target_properties(chrome-gl-analysis PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS} Wno-error=c++17-extensions")
    set_property(TARGET chrome-gl-analysis PROPERTY CXX_STANDARD 17)
endif()
