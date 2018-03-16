// csabase_util.cpp                                                   -*-C++-*-

#include <csabase_util.h>
#include <csabase_debug.h>
#include <clang/Basic/SourceManager.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/Regex.h>
#include <cctype>

using namespace clang;
using namespace csabase;

std::pair<size_t, size_t>
csabase::mid_mismatch(const std::string &have, const std::string &want)
{
    std::pair<size_t, size_t> result(0, 0);
    while (   result.first < have.size()
           && result.first < want.size()
           && have[result.first] == want[result.first]) {
        ++result.first;
    }
    while (   result.second < have.size()
           && result.second < want.size()
           && have.size() > result.first + result.second
           && want.size() > result.first + result.second
           && have[have.size() - result.second - 1] ==
              want[want.size() - result.second - 1]) {
        ++result.second;
    }
    return result;
}

std::pair<size_t, size_t>
csabase::mid_match(const std::string &have, const std::string &want)
{
    std::pair<size_t, size_t> result(have.find(want), have.npos);
    if (result.first != have.npos) {
        result.second = have.size() - want.size() - result.first;
    }
    return result;
}

static llvm::Regex between_comments(
    "^[[:blank:]]*\r*\n?[[:blank:]]*$",
    llvm::Regex::NoFlags);

bool csabase::areConsecutive(clang::SourceManager &manager,
                             clang::SourceRange first,
                             clang::SourceRange second)
{
    clang::FileID fidf = manager.getFileID(first.getEnd());
    clang::FileID fids = manager.getFileID(second.getBegin());
    size_t colf = manager.getPresumedColumnNumber(first.getBegin());
    size_t cols = manager.getPresumedColumnNumber(second.getBegin());
    size_t offf = manager.getFileOffset(first.getEnd());
    size_t offs = manager.getFileOffset(second.getBegin());

    return fidf == fids && colf == cols && offf <= offs &&
           between_comments.match(
               manager.getBufferData(fidf).substr(offf, offs - offf));
}

std::string csabase::to_lower(llvm::StringRef s)
{
    return s.lower();
}

bool csabase::contains_word(llvm::StringRef have, llvm::StringRef want)
{
    size_t i = 0;
    size_t match;
    while ((match = have.find(want, i)) != want.npos) {
        bool found = true;
        if (match > 0) {
            char c = have[match - 1];
            if (std::isalnum(c) || c == '_') {
                found = false;
            }
        }
        if (found && match + want.size() < have.size()) {
            char c = have[match + want.size()];
            if (std::isalnum(c) || c == '_') {
                found = false;
            }
        }
        if (found) {
            return true;
        }
        i = match + want.size() + 1;
    }
    return false;
}

bool csabase::are_numeric_cognates(llvm::StringRef a, llvm::StringRef b)
{
    static std::string digits = "0123456789";
    size_t ai = 0;
    size_t bi = 0;

    while (ai < a.size() && bi < b.size()) {
        size_t adi = a.find_first_of(digits, ai);
        size_t bdi = b.find_first_of(digits, bi);
        if (a.slice(ai, adi) != b.slice(bi, bdi)) {
            break;
        }
        ai = a.find_first_not_of(digits, adi);
        bi = b.find_first_not_of(digits, bdi);
    }
    return ai == a.npos && bi == b.npos;
} 

std::string csabase::on_one_line(llvm::StringRef s, bool explicitNL)
{
    std::string ret;
    while (s.size()) {
        switch (s.front()) {
        case ' ': case '\r': case '\t':
            ret += ' ';
            s = s.ltrim();
            break;
        case '\n':
            ret += explicitNL ? " \\n " : " ";
            s = s.ltrim();
            break;
        default:
            ret += s.front();
            s = s.drop_front();
            break;
        }
    }
    return ret;
}

csabase::OnMatch<UseLambda, &UseLambda::NotFunction>::OnMatch(
    const std::function<void(const clang::ast_matchers::BoundNodes &)> &fun)
    : function_(fun)
{
}

void csabase::OnMatch<UseLambda, &UseLambda::NotFunction>::run(
    const clang::ast_matchers::MatchFinder::MatchResult &result)
{
    function_(result.Nodes);
}

SourceRange csabase::getOffsetRange(SourceLocation loc, int offset, int size)
{
    return SourceRange(loc.getLocWithOffset(offset),
                       loc.getLocWithOffset(offset + size));
}

SourceRange csabase::getOffsetRange(SourceRange range, int offset, int size)
{
    return SourceRange(range.getBegin().getLocWithOffset(offset),
                       range.getBegin().getLocWithOffset(offset + size));
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
