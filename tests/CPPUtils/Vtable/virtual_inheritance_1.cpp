class Base {
  private:
    char base_char_data[4];

  public:
    Base() {}
    ~Base() {}
};

class Child : virtual Base {
  public:
    Child() {}
    ~Child() {}
};

int main(int argc, char *argv[]) {
    Child c;
    return 0;
}
