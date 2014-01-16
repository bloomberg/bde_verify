// csabase_util.h                                                     -*-C++-*-
// -----------------------------------------------------------------------------
// Copyright 2013 Hyman Rosen (hrosen4@bloomberg.net)
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#if !defined(INCLUDED_CSABASE_UTIL)
#define INCLUDED_CSABASE_UTIL 1
#ident "$Id$"

#include <clang/Basic/SourceManager.h>
#include <string>
#include <utility>

namespace cool {
namespace csabase {

std::pair<unsigned, unsigned>
mid_mismatch(const std::string &have, const std::string &want);
    // Return a pair of values '(a,b)' such that 'a' is the maximum length of a
    // common prefix of the specified 'have' and 'want' and 'b' is the maximum
    // length of a common suffix of 'have.substr(a)' and 'want.substr(a)'.

std::pair<unsigned, unsigned>
mid_match(const std::string &have, const std::string &want);
    // Return a pair of values '(a,b)' such that 'a' is the count of characters
    // in the specified 'have' before the first appearance of the specified
    // 'want' and 'b' is the count of characters in 'have' after the first
    // appearance of 'want'.  If 'want' is not in 'have', return a pair of
    // 'npos' instead.

bool areConsecutive(clang::SourceManager& manager,
                    clang::SourceRange    first,
                    clang::SourceRange    second);
    // Return 'true' iff the specified 'first' range is immediately followed by
    // the specified 'second' range, with only whitespace in between, and the
    // two begin at the same column.  (This is used to paste consecutive '//'
    // comments into single blocks.)

}
}

#endif
