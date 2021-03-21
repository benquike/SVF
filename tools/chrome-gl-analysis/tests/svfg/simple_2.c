void emit_log(const char *msg) {

}

void test_api(int arg1, int arg2) {
  if (arg1 < 0 || arg1 > 10) {
    emit_log("msg 1");
  }


  if (arg2 >= 1 && arg2 <= 100) {
    emit_log("msg 2");
  }
}
