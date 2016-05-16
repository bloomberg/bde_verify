namespace BloombergLP { namespace bslma { class Allocator; } }

struct A { explicit A(BloombergLP::bslma::Allocator * = 0); };
struct B { };

void f(A *a) { *a = A(); }
void f(B *b) { *b = B(); }
void f(void *) { }
void f(char *c) { *c = char(); }
void f(int *i) { *i = int(); }
void f(char *c, int) { *c = char(); }
void f(int *i, int) { *i = int(); }
void f(char *c, unsigned) { *c = char(); }
void f(int *i, unsigned) { *i = int(); }

void g(int *i) { *i = int(); }
int g();

struct C {
    explicit C(A *a) { *a = A(); }
    explicit C(B *b) { *b = B(); }
    explicit C(void *) { }
    explicit C(char *c) { *c = char(); }
    explicit C(int *i) { *i = int(); }
    explicit C(char *c, int) { *c = char(); }
    explicit C(int *i, int) { *i = int(); }
    explicit C(char *c, unsigned) { *c = char(); }
    explicit C(int *i, unsigned) { *i = int(); }

    void f(A *a) { *a = A(); }
    void f(B *b) { *b = B(); }
    void f(void *) { }
    void f(char *c) { *c = char(); }
    void f(int *i) { *i = int(); }
    void f(char *c, int) { *c = char(); }
    void f(int *i, int) { *i = int(); }
    void f(char *c, unsigned) { *c = char(); }
    void f(int *i, unsigned) { *i = int(); }

    void g(int *i) { *i = int(); }
    int g();
};

void f1(int *i, int *) { *i = int(); }
void f2(int *i, const int *) { *i = int(); }
void f3(int *i, short *) { *i = int(); }
void f4(int *i, double *) { *i = int(); }

struct D {
    void f1(int *i, int *) { *i = int(); }
    void f2(int *i, const int *) { *i = int(); }
    void f3(int *i, short *) { *i = int(); }
    void f4(int *i, double *) { *i = int(); }
};
