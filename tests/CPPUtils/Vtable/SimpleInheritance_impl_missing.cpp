class Base {
  public:
    virtual void foo() = 0;
};

class Child : public Base {
  public:
    void foo(){};
};

int main(int argc, char *argv[]) {
    Child c;
    return 0;
}
