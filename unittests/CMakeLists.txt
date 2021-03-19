subdirlist(subdirs ${CMAKE_CURRENT_SOURCE_DIR})

add_custom_target(SvfUnitTests)

foreach(DIR ${subdirs})
    add_subdirectory(${DIR})
endforeach(TEST_SRC)

