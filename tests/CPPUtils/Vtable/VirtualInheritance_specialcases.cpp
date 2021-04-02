
class Top {
  public:
    int x1;
    int x2;

    Top() {
        x1 = 1;
        x2 = 2;
    }
    virtual void foo() {}
};

class Left : public virtual Top {
  public:
    int y1;
    int y2;
    Left() {
        y1 = 3;
        y2 = 4;
    }
    virtual void left_foo() {}
};

class Right : public virtual Top {
  public:
    int z1;
    int z2;

    Right() {
        z1 = 7;
        z2 = 8;
    }

    virtual void right_foo() {}
};

class Botton : public virtual Left, public virtual Right {
  public:
    int w1;
    int w2;

    Botton() {
        w1 = 9;
        w2 = 10;
    }

    virtual void foo() {}
    // void left_foo() {}
    // virtual void bar() {}
};

int main(int argc, char *argv[]) {

    Botton b;

    return 0;
}
