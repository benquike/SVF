
class Base {
    public:
        virtual int test(int a, int b) { return a + b; }
};

class Child: public Base {
    public:
        int test(int a, int b) override { return a + b + 1; }
};


void dispatch(Base *b) {
    b->test(1, 2);
}

int main() {
    Base b;;
    dispatch(&b);
    Child c;
    dispatch(&c);
    return 0;
}
