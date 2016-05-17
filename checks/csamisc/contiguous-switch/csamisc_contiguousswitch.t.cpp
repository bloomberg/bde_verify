// csamisc_contiguousswitch.t.cpp                                     -*-C++-*-

#include "csamisc_contiguousswitch.t.hpp"
#include <bdes_ident.h>

// -----------------------------------------------------------------------------

namespace bde_verify
{
    namespace csamisc
    {
        namespace
        {
            void f(int ac) {
                switch (ac) {
                case 5:
                    break;
                case 3:
                    break;
                }
            }
        }
    }
}

int main(int, char*[]);

int main(int ac, char* av[])
{
    switch (ac) {
    }

    switch (ac) { // starts with `default:` instead of `case 0:`
    default:
        ++ac;
        break;
    case 1:
        {
            ++ac;
            ++ac;
        }
        break;
    }

    switch (ac) { // doesn't start with `case 0:` and `case 0` in the middle
    case 1: {
        break; 
    }
    case 0: {
        ++ac;
        break;
    }
    default: ;
    }

    switch (ac) { // `default:` not at end
    case 0:
    default: break;
    case 1: break;
    }

    switch (ac) { // `default:` not at end
    case 0:
    case 2: break;
    default: break;
    case 1: break;
    }

    switch (ac) // missing default
    {
    case 0:
        switch (ac + 1) {
        case 10: break;
        case 12: break;
        }
    case 5: break;
    case 4:
    case 3: break;
    case 2: break;
    case 1: break;
    }

    switch (ac)
    {
    case 0:
    case 100: break;
    case 5: break;
    case 4: break;
    case 3: break;
    case 2: break;
    case 1: break;
    case -1: break;
    case -5: break;
    default: break;
    }

    switch (ac)
    {
    case 0:
    case 12:
        ++ac;
        ++ac;
        ++ac;
        { break; }
    case 11:
        { ++ac; }
        ++ac;
        ++ac;
    case 10: {
        ++ac;
        ++ac;
    } break;
    default:
        { break; }
    }
    return 0;
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
