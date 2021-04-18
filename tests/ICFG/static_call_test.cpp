
void foo(int &a);

void bar(int x) {

    if (x < 1001)
        foo(x);
}

void foo(int &a) { a = a + 1; }

int main() {
    int a = 100;
    foo(a);

    return 0;
}
