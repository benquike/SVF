struct Base {
    virtual void foo() {}
};

struct Derived : Base {};

int main() {
    Derived d;
    d.foo();
    return 0;
}
