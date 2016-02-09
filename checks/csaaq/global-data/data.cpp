int a;
static int b;
extern int c;

int d = 1;
static int e = 1;

void g();
static void h();
extern void i();

void j() { }
static void k() { }
extern void l() { }

namespace {
int a;
static int b;
extern int c;

int d = 1;
static int e = 1;

void g();
static void h();
extern void i();

void j() { }
static void k() { }
extern void l() { }
}

namespace N {
int a;
static int b;
extern int c;

int d = 1;
static int e = 1;

void g();
static void h();
extern void i();

void j() { }
static void k() { }
extern void l() { }
}

void m()
{
    int a;
    static int b;
    extern int c;

    int d = 1;
    static int e = 1;
}

struct n {
    int a;
    static int b;

    static int e;

    void g();
    static void h();

    void j() { }
    static void k() { }
};
int n::b;
int n::e = 1;

template <int N>
void o()
{
    int a;
    static int b;
    extern int c;

    int d = 1;
    static int e = 1;
}

template <int N>
struct p {
    int a;
    static int b;

    static int e;

    void g();
    static void h();

    void j() { }
    static void k() { }
};
template <int N> int p<N>::b;
template <int N> int p<N>::e = 1;
