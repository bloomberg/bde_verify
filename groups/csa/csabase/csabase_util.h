// csabase_util.h                                                     -*-C++-*-
// -----------------------------------------------------------------------------
// Copyright 2013 Hyman Rosen (hrosen4@bloomberg.net)
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#if !defined(INCLUDED_CSABASE_UTIL)
#define INCLUDED_CSABASE_UTIL 1
#ident "$Id$"

#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/Basic/SourceManager.h>

#include <stddef.h>
#include <string>
#include <utility>

namespace csabase {

std::pair<size_t, size_t>
mid_mismatch(const std::string &have, const std::string &want);
    // Return a pair of values '(a,b)' such that 'a' is the maximum length of a
    // common prefix of the specified 'have' and 'want' and 'b' is the maximum
    // length of a common suffix of 'have.substr(a)' and 'want.substr(a)'.

std::pair<size_t, size_t>
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

std::string to_lower(std::string s);
    // Return a copy of the specified 's' with all letters in lower case.

template <class Class,
          void (Class::*Method)(const clang::ast_matchers::BoundNodes &)>
class OnMatch : public clang::ast_matchers::MatchFinder::MatchCallback
    // This class template acts as an intermediary to forward AST match
    // callbacks to the specified 'Method' of the specified 'Class'.
{
  public:
    OnMatch(Class *object);
        // Create an 'OnMatch' object, storing the specified 'object' pointer
        // for use in the callback.

    void run(const clang::ast_matchers::MatchFinder::MatchResult &result);
        // Invoke the 'Method' of the 'object_', passing the 'BoundNodes' from
        // the specified 'result' as an argument.

  private:
    Class *object_;
};

template <class Class,
          void (Class::*Method)(const clang::ast_matchers::BoundNodes &)>
OnMatch<Class, Method>::OnMatch(Class *object)
    : object_(object)
{
}

template <class Class,
          void (Class::*Method)(const clang::ast_matchers::BoundNodes &)>
void OnMatch<Class, Method>::run(
    const clang::ast_matchers::MatchFinder::MatchResult &result)
{
    (object_->*Method)(result.Nodes);
}

} // close package namespace

#endif
