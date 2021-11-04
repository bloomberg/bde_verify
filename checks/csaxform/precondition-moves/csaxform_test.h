// csaxform_test.h                                                    -*-C++-*-

#include <bsls_assert.h>
#include <bsls_review.h>

class Test {
  private:
    int d_data;

  public:
    Test(int x);

    void foo(int a, float b);

    template <class TYPE>
    void tFoo(const TYPE& a);
};


template <class TYPE>
class TemplateTest {
  private:
    TemplateTest(const TYPE& x);

    void foo(int x);
};

void testFunc1(int p1);

void testFunc1a(int);

void testFunc2(int p1, int p2);

void testFunc3(int p1, int p2, int p3);

void testFunc4(int p1, int p2, int p3);

template <class TYPE>
void testTFunc1(const TYPE& p1);

template <class TYPE>
void testTFunc2(int p1, const TYPE& p2);

// ----------------------------------------------------------------------------
//                            INLINE DEFINITIONS
// ----------------------------------------------------------------------------

template <class TYPE>
void
Test::tFoo(const TYPE& a)
{
    BSLS_ASSERT(d_data > 0);
    BSLS_ASSERT(a.isInitialized());
}

template <class TYPE>
void testTFunc1(const TYPE& p1)
{
    BSLS_ASSERT(p1 > 100);
}

template <class TYPE>
void testTFunc2(int p1, const TYPE& p2)
{
    BSLS_ASSERT(p1 > 110);
    BSLS_ASSERT(p2.isNotInitialized());
}
