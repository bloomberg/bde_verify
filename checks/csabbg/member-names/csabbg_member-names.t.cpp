// csabbg_member-names.t.cpp                                          -*-C++-*-

class A
{
  public:
    int          d_i;

  protected:
    int          d_j;

  private:
    void         private_f();

    int          private_a;
    static int   private_b;
    char        *private_c;
    static char *private_d;

    int          private_a_p;
    static int   private_b_p;
    char        *private_c_p;
    static char *private_d_p;

    int          d_private_a;
    static int   d_private_b;
    char        *d_private_c;
    static char *d_private_d;

    int          s_private_a;
    static int   s_private_b;
    char        *s_private_c;
    static char *s_private_d;

    int          d_private_a_p;
    static int   d_private_b_p;
    char        *d_private_c_p;
    static char *d_private_d_p;

    int          s_private_a_p;
    static int   s_private_b_p;
    char        *s_private_c_p;
    static char *s_private_d_p;
};

struct B
{
    int d_i;
    static const int k_a = 1;
    static const int s_a = 2;
    typedef char *c_t;
    c_t d_x;
    typedef int i_t;
    i_t d_p;

    int :0;
};

// ----------------------------------------------------------------------------
// Copyright (C) 2015 Bloomberg Finance L.P.
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
