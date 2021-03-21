void emit_log(const char *msg) {

}

int check_arg2(int arg2) {
  if (arg2 >= 1 && arg2 <= 100) {
    return 1;
  }

  return 0;
}

void test_api(int arg1, int arg2) {
  if (arg1 < 0 || arg1 > 10) {
    emit_log("msg 1");
  }

  if (check_arg2(arg2)) {
    emit_log("msg 2");
  }
}
