file(GLOB SRCS CONFIGURE_DEPENDS "*.cpp" "*.c")

foreach(TEST_SRC ${SRCS})
    get_filename_component(FNAME ${TEST_SRC} NAME)
    generate_ll_file(FILE ${FNAME})
endforeach(TEST_SRC)
