class Base {
    public:
        virtual void foo() {}
};

class Child: public Base {
    public:
        virtual void foo() {}
};


int main(int argc, char *argv[]) {
    Child c;
    return 0;
}
