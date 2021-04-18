void foo(int &a) { a = a + 1; }

int main() {
    void (*ptr)(int &) = &foo;
    int a = 100;

    ptr(a);

    return 0;
}
