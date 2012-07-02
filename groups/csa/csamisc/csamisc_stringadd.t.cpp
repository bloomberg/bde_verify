// csamisc_stringadd.t.cpp                                            -*-C++-*-
// -----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

namespace cool
{
    namespace csamisc
    {
        template <typename T>
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
    cool::csamisc::string str;
    char const* lit("0123");

    cool::csamisc::use("0123" + ac);
    cool::csamisc::use("0123" + str);
    cool::csamisc::use(lit + ac);

    cool::csamisc::use(ac + "0123");
    cool::csamisc::use(str + "0123");
    cool::csamisc::use(ac + lit);

    cool::csamisc::use("0123" - ac);
    cool::csamisc::use("0123" - str);
    cool::csamisc::use(lit - ac);

    cool::csamisc::use(str - "0123");

    cool::csamisc::use("0123" + -1);
    cool::csamisc::use("0123" + 0);
    cool::csamisc::use("0123" + 1);
    cool::csamisc::use("0123" + 2);
    cool::csamisc::use("0123" + 3);
    cool::csamisc::use("0123" + 4);
    cool::csamisc::use("0123" + 5);

    cool::csamisc::use("0123" - 1);
    cool::csamisc::use("0123" - 0);
    cool::csamisc::use("0123" - -1);
    cool::csamisc::use("0123" - -2);
    cool::csamisc::use("0123" - -3);
    cool::csamisc::use("0123" - -4);
    cool::csamisc::use("0123" - -5);
}
