class Parent1 {
    public:
        virtual void foo() {}
};

class Parent2 {
    public:
        virtual void bar() {}
};

class Child: public Parent1, public Parent2{
    public:
        virtual void baz() {}
        virtual void foo() {}
};


int main(int argc, char *argv[]) {
    Child c;
    return 0;
}
