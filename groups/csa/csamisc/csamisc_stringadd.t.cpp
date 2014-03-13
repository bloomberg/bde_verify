// csamisc_stringadd.t.cpp                                            -*-C++-*-
// -----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

namespace bde_verify
{
    namespace csamisc
    {
        template <class T>
        static void use(T const&)
        {
        }

        namespace
        {
            struct string
            {
            };
            
            string operator+(string const& s, char const*)
            {
                return s;
            }
            string operator+(char const*, string const& s)
            {
                return s;
            }

            string operator-(string const& s, char const*)
            {
                return s;
            }
            string operator-(char const*, string const& s)
            {
                return s;
            }
        }
    }
}

int main(int ac, char*[])
{
    bde_verify::csamisc::string str;
    char const* lit("0123");

    bde_verify::csamisc::use("0123" + ac);
    bde_verify::csamisc::use("0123" + str);
    bde_verify::csamisc::use(lit + ac);

    bde_verify::csamisc::use(ac + "0123");
    bde_verify::csamisc::use(str + "0123");
    bde_verify::csamisc::use(ac + lit);

    bde_verify::csamisc::use("0123" - ac);
    bde_verify::csamisc::use("0123" - str);
    bde_verify::csamisc::use(lit - ac);

    bde_verify::csamisc::use(str - "0123");

    bde_verify::csamisc::use("0123" + -1);
    bde_verify::csamisc::use("0123" + 0);
    bde_verify::csamisc::use("0123" + 1);
    bde_verify::csamisc::use("0123" + 2);
    bde_verify::csamisc::use("0123" + 3);
    bde_verify::csamisc::use("0123" + 4);
    bde_verify::csamisc::use("0123" + 5);

    bde_verify::csamisc::use("0123" - 1);
    bde_verify::csamisc::use("0123" - 0);
    bde_verify::csamisc::use("0123" - -1);
    bde_verify::csamisc::use("0123" - -2);
    bde_verify::csamisc::use("0123" - -3);
    bde_verify::csamisc::use("0123" - -4);
    bde_verify::csamisc::use("0123" - -5);
}
