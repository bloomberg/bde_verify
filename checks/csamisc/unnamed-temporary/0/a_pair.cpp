// a_pair.cpp                                                         -*-C++-*-
#include <bsl_utility.h>
#include <bsl_map.h>

int main()
{
    bsl::map<int, int> m;
    m.insert(bsl::make_pair(1, 2));
}
