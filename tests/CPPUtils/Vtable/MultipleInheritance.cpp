class Parent1 {
  public:
    virtual void foo() {}
};

class Parent2 {
  public:
    virtual void bar() {}
};

class Child : public Parent1, public Parent2 {
  public:
    virtual void baz() {}
    virtual void foo() {}
};

void dispatch_parent1(Parent1 *p1) { p1->foo(); }

void dispatch_parent2(Parent2 *p2) { p2->bar(); }

int main(int argc, char *argv[]) {
    Child c;
    dispatch_parent1(&c);
    dispatch_parent2(&c);
    return 0;
}
