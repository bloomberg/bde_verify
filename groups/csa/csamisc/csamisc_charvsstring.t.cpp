// csamisc_charvsstring.t.cpp                                         -*-C++-*-
// -----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

namespace cool
{
    namespace csamisc
    {
        void f(const char*);
        void f(int, const char*);
        void f(int, const char*, int);

        namespace {
            struct g
            {
                g(const char*) {}                                   // IMPLICIT
                g(int, const char*) {}
                void operator()(const char*) {}
                void operator()(int, const char*) {}
            };
        }
    }
}

using namespace cool::csamisc;

int main()
{
    char array[] = "hello";
    char value('/');
    f("hello"); // OK
    f(array); // OK
    f(&array[0]); // OK
    f(&value); // not OK
    f(0, "hello"); // OK
    f(0, array); // OK
    f(0, &array[0]); // OK
    f(0, &value); // not OK
    f(0, "hello", 1); // OK
    f(0, array, 1); // OK
    f(0, &array[0], 1); // OK
    f(0, &value, 1); // not OK

    g g0("hello"); // OK
    g g1(array); // OK
    g g2(&array[0]); // OK
    g g3(&value); // not OK
    g g4(0, "hello"); // OK
    g g5(0, array); // OK
    g g6(0, &array[0]); // OK
    g g7(0, &value); // not OK

    g0("hello"); // OK
    g1(array); // OK
    g2(&array[0]); // OK
    g3(&value); // not OK
    g4(0, "hello"); // OK
    g5(0, array); // OK
    g6(0, &array[0]); // OK
    g7(0, &value); // not OK
}
