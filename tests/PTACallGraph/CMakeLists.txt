file(GLOB SRCS CONFIGURE_DEPENDS "*.cpp" "*.c")

foreach(TEST_SRC ${SRCS})
    get_filename_component(FNAME ${TEST_SRC} NAME)
    generate_ll_file(FILE ${FNAME})
    generate_ll_file(DEBUG FILE ${FNAME})
endforeach(TEST_SRC)
