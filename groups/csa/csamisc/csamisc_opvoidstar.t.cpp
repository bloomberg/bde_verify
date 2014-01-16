// csamisc_longinline.t.cpp                                           -*-C++-*-

namespace cool
{
    namespace csamisc
    {
        struct ConvX {
            operator const void *() const { return this; }
            operator       void *()       { return this; }
            operator bool        () const { return this; }
        };

        struct ConvY {
            explicit operator const void *() const { return this; }
            explicit operator       void *()       { return this; }
            explicit operator bool        () const { return this; }
        };

        template <class Type>
        struct ConvZ {
            operator const void *() const { return this; }
            operator       void *()       { return this; }
            operator bool        () const { return this; }
        };

        template <class Type>
        struct ConvW {
            explicit operator const void *() const { return this; }
            explicit operator       void *()       { return this; }
            explicit operator bool        () const { return this; }
        };
    }
}
