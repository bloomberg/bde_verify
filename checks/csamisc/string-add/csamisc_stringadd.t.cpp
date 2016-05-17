// csamisc_stringadd.t.cpp                                            -*-C++-*-

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

// ----------------------------------------------------------------------------
// Copyright (C) 2014 Bloomberg Finance L.P.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// ----------------------------- END-OF-FILE ----------------------------------
