struct Base {
    virtual void foo() {}
};

struct Derived : Base {
    void foo() override {}
};

int main() {
    Derived d;
    d.foo();
    return 0;
}
