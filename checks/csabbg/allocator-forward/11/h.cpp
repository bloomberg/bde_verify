#include <bslma_usesbslmaallocator.h>
#include <bslmf_nestedtraitdeclaration.h>

namespace BloombergLP {
namespace bslma { class Allocator; }
}
using namespace BloombergLP;
struct X {
    X(bslma::Allocator *);
};
struct Y : X {
    bslma::Allocator *d_allocator_p;
    Y(bslma::Allocator *);
    Y(int);
    Y(double d = 1.5, int a = 7);
};
struct Z {
    X                 x;
    bslma::Allocator *d_allocator_p;
    Z(bslma::Allocator *);
};
struct W {
    W(int, bslma::Allocator *);
    W();
    W(const char *) { }
};
W::W() { }

struct A {
    A(bslma::Allocator *);
    A(const A&, bslma::Allocator *);
    bslma::Allocator *d_allocator_p;
    bslma::Allocator *allocator() const;
    BSLMF_NESTED_TRAIT_DECLARATION(A, bslma::UsesBslmaAllocator);
};
struct B : A {
    B(bslma::Allocator *);
    B(const B&, bslma::Allocator *);
    B(int);
    B(int, bslma::Allocator *);
    BSLMF_NESTED_TRAIT_DECLARATION(B, bslma::UsesBslmaAllocator);
};
struct C {
    A x;
    C(bslma::Allocator *);
    C(const C&, bslma::Allocator *);
    bslma::Allocator *allocator() const;
    BSLMF_NESTED_TRAIT_DECLARATION(C, bslma::UsesBslmaAllocator);
};
struct D : private A {
    D(bslma::Allocator *);
    D(const D&, bslma::Allocator *);
    D(int);
    D(int, bslma::Allocator *);
    BSLMF_NESTED_TRAIT_DECLARATION(D, bslma::UsesBslmaAllocator);
};

int main()
{
    X x(0), x_(x);
    Y y(0), y_(y);
    Z z(0), z_(z);
    A a(0), a_(a);
    B b(0), b_(b);
    C c(0), c_(c);
    D d(0), d_(d);
}
