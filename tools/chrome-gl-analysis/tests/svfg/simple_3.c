void emit_log(const char *msg) {

}

void check_arg2(int arg2) {
  if (arg2 >= 1 && arg2 <= 100) {
    emit_log("msg 2");
  }
}

void test_api(int arg1, int arg2) {
  if (arg1 < 0 || arg1 > 10) {
    emit_log("msg 1");
  }

  check_arg2(arg2);
}
