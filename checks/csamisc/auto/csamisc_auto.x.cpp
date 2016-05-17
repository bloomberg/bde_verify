// csamisc_auto.t.cpp                                                 -*-C++-*-

#if 0
template <typename T>
struct general_iterator
{
    void operator++();
    bool operator!= (general_iterator const&) const;
    T& operator*();
};

template <typename T>
struct vector
{
#if 0
    struct iterator
    {
        void operator++();
        bool operator!= (iterator const&) const;
        T& operator*();
    };
#else
    typedef general_iterator<T> iterator;
#endif
    iterator begin();
    iterator end();
};

int main()
{
    int  i = 17;
    auto var = 17;
    vector<int> v;
    for (auto it = v.begin(); it != v.end(); ++it) {
        auto& r = *it;
    }
}
#endif

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
