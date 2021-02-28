file(GLOB SRCS CONFIGURE_DEPENDS "*.cpp")


foreach(TEST_SRC ${SRCS})
  add_unittest(${TEST_SRC})
endforeach(TEST_SRC)
