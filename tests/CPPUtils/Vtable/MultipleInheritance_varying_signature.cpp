class Parent1 {
  public:
    virtual void foo(char a) {}
};

class Parent2 {
  public:
    virtual void bar(int b) {}
};

class Child : public Parent1, public Parent2 {
  public:
    virtual void baz(float c) {}
    virtual void foo(char a) {}
};

void dispatch_parent1(Parent1 *p1) { p1->foo('a'); }

void dispatch_parent2(Parent2 *p2) { p2->bar(2); }

int main(int argc, char *argv[]) {
    Child c;
    dispatch_parent1(&c);
    dispatch_parent2(&c);
    return 0;
}
