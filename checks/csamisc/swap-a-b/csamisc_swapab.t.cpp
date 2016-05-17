// csamisc_swapab.t.cpp                                               -*-C++-*-

namespace bde_verify
{
    namespace {
        void swap(int  x, int  y);
        void swap(int *x, int &y);
        void swap(int *x, int *y);
        void swap(int *x, int *b);
        void swap(int *a, int *y);
        void swap(int *a, int *b);
        void swap(int &x, int &y);
        void swap(int &x, int &b);
        void swap(int &a, int &y);
        void swap(int &a, int &b);
        void swap(int &a, int &b);
        void swap(int &a, int & );
        void swap(int & , int &b);
        void swap(int & , int & );
        void swap(int * , int *b);
        void swap(int *a, int * );
        void swap(int * , int * );
            // Exchange the specified items.

        template <class SS> void swap(SS  x, SS  y);
        template <class SS> void swap(SS *x, SS &y);
        template <class SS> void swap(SS *x, SS *y);
        template <class SS> void swap(SS *x, SS *b);
        template <class SS> void swap(SS *a, SS *y);
        template <class SS> void swap(SS *a, SS *b);
        template <class SS> void swap(SS &x, SS &y);
        template <class SS> void swap(SS &x, SS &b);
        template <class SS> void swap(SS &a, SS &y);
        template <class SS> void swap(SS &a, SS &b);
        template <class SS> void swap(SS &a, SS &b);
        template <class SS> void swap(SS &a, SS & );
        template <class SS> void swap(SS & , SS &b);
        template <class SS> void swap(SS & , SS & );
        template <class SS> void swap(SS * , SS *b);
        template <class SS> void swap(SS *a, SS * );
        template <class SS> void swap(SS * , SS * );
            // Exchange the specified items.
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
