// csabase_util.h                                                     -*-C++-*-
// -----------------------------------------------------------------------------
// Copyright 2013 Hyman Rosen (hrosen4@bloomberg.net)
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#if !defined(INCLUDED_CSABASE_UTIL)
#define INCLUDED_CSABASE_UTIL 1
#ident "$Id$"

#include <string>
#include <utility>

namespace cool {
namespace csabase {

std::pair<unsigned, unsigned>
mid_mismatch(const std::string &have, const std::string &want);
    // Return a pair of values '(a,b)' such that 'a' is the maximum length of a
    // common prefix of the specified 'have' and 'want' and 'b' is the maximum
    // length of a common suffix of 'have.substr(a)' and 'want.substr(a)'.

}
}

#endif
