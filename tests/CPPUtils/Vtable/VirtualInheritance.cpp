
class Top {
    public:
        int x1;
        int x2;

        Top() {
            x1 = 1;
            x2 = 2;
        }

        virtual void foo(int a) {}
        virtual void bar(bool b) {}
};

class Left: virtual Top {
    public:
        int y1;
        int y2;
        Left() {
            y1 = 3;
            y2 = 4;
        }

        void foo(int a) {}
        virtual void left_foo(char x) {}
        virtual void left_bar(short y) {}
};

class Middle: virtual Top {
    public:
        int y1;
        int y2;

        Middle() {
            y1 = 5;
            y2 = 6;
        }

        void foo(int x) {}
        void bar(bool b) {}
        virtual void middle_foo(unsigned char x) {}
        virtual void middle_bar(unsigned short y) {}
};

class Right: virtual Top {
    public:
        int z1;
        int z2;

        Right() {
            z1 = 7;
            z2 = 8;
        }

        void foo(int x) {}
        virtual void right_foo(long x) {}
        virtual void middle_bar(unsigned long y) {}
};

class Botton: public Left, public Right {
    public:
        int w1;
        int w2;

        Botton() {
            w1 = 9;
            w2 = 10;
        }

        void foo(int x) {}
        void left_foo(char x) {}
        void middle_foo(unsigned char x) {}
        void right_foo(long x) {}
        virtual void bar(char *cp) {}
};


void foo_dispatch(Top *t) {
    t->foo(0);
}

void right_foo_dispatch(Right *r) {
    r->foo(0);
}


void left_dispatch(Left *l) {
    l->left_bar(0);
    l->left_foo('a');
}



int main(int argc, char *argv[]) {
    Right r;
    Middle m;
    Left l;

    Top t;
    Botton b;

    return 0;
}
