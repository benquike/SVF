#include <cstdio>
class Base {
    public:
        virtual void test(int x) {

            if (x == 1) {
                printf("Base test");
            }
        }
};


class Child: public Base {
    public:
        virtual void test(int x) {
            if (x == 3) {
                printf("Child test");
            }

        }
};


void dispatch(Base *b, int x) {
    b->test(x);
}

int main(int argc, char *argv[]) {

    Base b;
    dispatch(&b, 1);

    return 0;
}
