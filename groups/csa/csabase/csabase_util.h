// csabase_util.h                                                     -*-C++-*-

#ifndef INCLUDED_CSABASE_UTIL
#define INCLUDED_CSABASE_UTIL

#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/Basic/SourceLocation.h>
#include <llvm/ADT/StringRef.h>
#include <stddef.h>
#include <functional>
#include <string>
#include <utility>

namespace clang { class SourceManager; }

namespace csabase
{
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

std::string to_lower(llvm::StringRef s);
    // Return a copy of the specified 's' with all letters in lower case.

size_t contains_word(llvm::StringRef have, const llvm::StringRef want);
    // Return the first position where the specified string 'have' contains the
    // specified string 'want' as a word, that is, neither preceeded nor
    // followed by an underscore or alphanumeric character, or 'npos'
    // otherwise.

bool are_numeric_cognates(llvm::StringRef a, llvm::StringRef b);
    // Return true iff the specified 'a' and 'b' have the same value when
    // digits are disregarded.

clang::SourceRange
getOffsetRange(clang::SourceLocation loc, int offset, int size);
clang::SourceRange
getOffsetRange(clang::SourceRange range, int offset, int size);
    // Return the range of the specified 'size' beginning at the specified
    // 'offset' from the start of the specified 'loc' or 'range'.

std::string on_one_line(llvm::StringRef s, bool explicitNL = false);
    // Return a one-line representation of the specified input 's'.  Optionally
    // specify 'explicitNL' to show newlines as '\n'.

struct UseLambda {
    void NotFunction(const clang::ast_matchers::BoundNodes &);
};

template <class Class = UseLambda,
          void (Class::*Method)(const clang::ast_matchers::BoundNodes &) =
              &UseLambda::NotFunction>
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

template <>
class OnMatch<UseLambda, &UseLambda::NotFunction> :
    public clang::ast_matchers::MatchFinder::MatchCallback
    // This class template acts as an intermediary to forward AST match
    // callbacks to the specified lambda.
{
  public:
    OnMatch(const std::function<
        void(const clang::ast_matchers::BoundNodes &)> &fun);
        // Create an 'OnMatch' object, storing the specified 'function'

    void run(const clang::ast_matchers::MatchFinder::MatchResult &result);
        // Invoke the 'function_', passing the 'BoundNodes' from the specified
        // 'result' as an argument.

  private:
    std::function<void(const clang::ast_matchers::BoundNodes &)> function_;
};

class SortByLocation
{
    clang::SourceManager &m;

public:
    SortByLocation(clang::SourceManager &m) : m(m) { }

    template <class T>
    bool operator()(const T *a, const T *b) const
    {
        return m.isBeforeInTranslationUnit(a->getBeginLoc(), b->getBeginLoc());
    }
};

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
