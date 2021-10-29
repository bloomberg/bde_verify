// csaxform_test.cpp                                                  -*-C++-*-
#include <csaxform_test.h>

Test::Test(int x)
{
    BSLS_ASSERT(0 > x);
}

void
Test::foo(int a, float b)
{
    BSLS_ASSERT(0 < a);
    BSLS_ASSERT(a < b);
}

void testFunc1(int p1)
{
    BSLS_ASSERT(p1 > 1);
    BSLS_ASSERT_OPT(p1 > 2);
    BSLS_ASSERT_SAFE(p1 > 3);
}

void testFunc1a(int p1)
{
    BSLS_ASSERT(p1 > 10);
}

void testFunc2(int p1, int p2)
{
    BSLS_ASSERT(p1 > 20);
    BSLS_ASSERT(p2 > 21);
}

void testFunc3(int p1, int p2, int p3)
{
    BSLS_ASSERT(p1 > 30);
    BSLS_ASSERT(p2 > 31);
    BSLS_ASSERT(p3 > 32);
}
