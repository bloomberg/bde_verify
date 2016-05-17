// csabbg_midreturn.t.cpp                                             -*-C++-*-
// ----------------------------------------------------------------------------

#include "csabbg_midreturn.t.hpp"
#include <bdes_ident.h>

namespace bde_verify
{
    namespace csabbg
    {
        int f()
        {
            struct x {
                static bool test() {
                    volatile bool b = true;
                    if (!b) {
                        return false;
                    }
                    return b;
                }
            };
            if (x::test()) {
                return 1;
            }
            if (x::test()) {
                return 2;                                            // RETURN
            }
            if (x::test()) {
                return 3;
            }
            if (f()) {
                return 4;                                             // RETURN
            }
            return 0;
        }

        template <class X>
        int g()
        {
            struct x {
                static bool test() {
                    volatile bool b = true;
                    if (!b) {
                        return false;
                    }
                    return b;
                }
            };
            if (x::test()) {
                return 1;
            }
            if (x::test()) {
                return 2;                                            // RETURN
            }
            if (x::test()) {
                return 3;
            }
            if (f()) {
                return 4;                                             // RETURN
            }
            return 0;
        }

        int h()
        {
            return g<char>() + g<double>();
        }

        int i()
        {
            if (i()) {
                return 5;          // Comment                        // RETURN
            }
            if (i()) {
                return 6;          // Comment                         // RETURN
            }
            if (i()) {
                return 7;          // Comment                          // RETURN
            }
            return 0;
        }

        struct y {
            y()
            {
                if (f()) {
                    return;
                }
                if (f()) {
                    return;                                          // RETURN
                }
                if (f()) {
                    return;
                }
                if (f()) {
                    return;                                           // RETURN
                }
                return;
            }
        };

        template <class X>
        struct z {
            z()
            {
                if (f()) {
                    return;
                }
                if (f()) {
                    return;                                          // RETURN
                }
                if (f()) {
                    return;
                }
                if (f()) {
                    return;                                           // RETURN
                }
                return;
            }
        };

        void w()
        {
            z<char> z1;
            z<int> z2;
        }

        int l()
        {
            if (f()) return                                                    1;
            if (f()) return                                                   1;
            if (f()) return                                                  1;
            if (f()) return                                                 1;
            if (f()) return                                                1;
            if (f()) return                                               1;
            if (f()) return                                              1;
            if (f()) return                                             1;
            if (f()) return                                            1;
            if (f()) return                                           1;
            if (f()) return                                          1;
            if (f()) return                                         1;
            if (f()) return                                        1;
            return 0;
        }

        int m()
        {
            if (f()) return                                                    1;
                                                                      // RETURN
            if (f()) return                                                   1;
                                                                      // RETURN
            if (f()) return                                                  1;
                                                                      // RETURN
            if (f()) return                                                 1;
                                                                      // RETURN
            if (f()) return                                                1;
                                                                      // RETURN
            if (f()) return                                               1;
                                                                      // RETURN
            if (f()) return                                              1;
                                                                      // RETURN
            if (f()) return                                             1;
                                                                      // RETURN
            if (f()) return                                            1;
                                                                      // RETURN
            if (f()) return                                           1;
                                                                      // RETURN
            if (f()) return                                          1;
                                                                      // RETURN
            if (f()) return                                         1;
                                                                      // RETURN
            if (f()) return                                        1; // RETURN
            return 0;
        }
    }
}

#include <stdio.h>

namespace bde_verify
{
    namespace csabbg
    {
        void *n()
        {
            if (f()) {
                return NULL;
            }
            if (f()) {
                return NULL;  // RETURN
            }
            if (f()) {
                return NULL;                                          // RETURN
            }
            return NULL;
        }

        int f(int n)
        {
            switch (n) {
              case 1:
                return 7;
              default:
                ++n;
                return n;
              case 2:
                return 8;
            }
        }

        int g(int n)
        {
            switch (n) {
              case 1:
                return 7;
              default:
                return 0;
              case 2:
                return 8;
            }
        }

        int h(int n)
        {
            switch (n) {
              default:
                return 7;
              case 1:
                ++n;
                return n;
              case 2:
                return 8;                                             // RETURN
            }
        }

        int i(int n)
        {
            switch (n) {
              case 1:
                switch (n) {
                  case 1:
                    return 7;
                }
              default:
                return n;
            }
        }
    }
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
