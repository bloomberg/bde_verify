// csamisc_longinline.t.cpp                                           -*-C++-*-

namespace bde_verify
{
    namespace csamisc
    {
        inline
        void f()
        { ; //  1
        }   //  2

        inline
        void g()
        {   ; //  1
            ; //  2
            ; //  3
            ; //  4
            ; //  5
            ; //  6
            ; //  7
            ; //  8
        }     //  9

        inline
        void h()
        {   ; //  1
            ; //  2
            ; //  3
            ; //  4
            ; //  5
            ; //  6
            ; //  7
            ; //  8
            ; //  9
        }     // 10

        inline
        void i()
        {   ; //  1
            ; //  2
            ; //  3
            ; //  4
            ; //  5
            ; //  6
            ; //  7
            ; //  8
            ; //  9
            ; // 10
        }     // 11

        inline
        void j()
        {   ; //  1
            ; //  2
            ; //  3
            ; //  4
            ; //  5
            ; //  6
            ; //  7
            ; //  8
            ; //  9
            ; // 10
            ; // 11
        }     // 12

        inline
        void k()
        {   ; //  1
            ; //  2
            ; //  3
            ; //  4
            ; //  5
            ; //  6
            ; //  7
            ; //  8
            ; //  9
            ; // 10
            ; // 11
            ; // 12
        }     // 13
    }
}

// -----------------------------------------------------------------------------

int main()
{
        // this space is intentionally left empty
}

