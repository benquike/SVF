file(GLOB SRCS CONFIGURE_DEPENDS "*.cpp")

foreach(TEST_SRC ${SRCS})
    get_filename_component(FNAME ${TEST_SRC} NAME)
    generate_ll_file(FILE ${FNAME} DEBUG)
endforeach(TEST_SRC)
