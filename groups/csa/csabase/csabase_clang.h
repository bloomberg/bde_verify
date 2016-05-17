// csabase_clang.h                                                    -*-C++-*-
#ifndef INCLUDED_CSABASE_CLANG
#define INCLUDED_CSABASE_CLANG

#include <clang/Basic/SourceLocation.h>
#include <llvm/ADT/Hashing.h>
#include <llvm/ADT/StringRef.h>
#include <tuple>
#include <utility>

namespace std
{

template <typename A, typename B>
struct hash<pair<A, B>>
{
    size_t operator()(const pair<A, B>& p) const
    {
        return llvm::hash_combine(hash<A>()(p.first), hash<B>()(p.second));
    }
};

template <typename A, typename B, typename C>
struct hash<tuple<A, B, C>>
{
    size_t operator()(const tuple<A, B, C>& t) const
    {
        return llvm::hash_combine(
            hash<A>()(get<0>(t)), hash<B>()(get<1>(t)), hash<C>()(get<2>(t)));
    }
};

template <>
struct hash<clang::SourceLocation>
{
    size_t operator()(const clang::SourceLocation& sl) const
    {
        return sl.getRawEncoding();
    }
};

template <>
struct hash<clang::SourceRange>
{
    size_t operator()(const clang::SourceRange& sr) const
    {
        return llvm::hash_combine(
            sr.getBegin().getRawEncoding(), sr.getEnd().getRawEncoding());
    }
};

template <>
struct hash<llvm::StringRef> {
    size_t operator()(const llvm::StringRef& sr) const
    {
        return llvm::hash_value(sr);
    }
};

template <>
struct hash<clang::FileID> {
    size_t operator()(const clang::FileID& fid) const
    {
        return fid.getHashValue();
    }
};

}

namespace clang
{

inline
bool operator<(const clang::SourceRange& a, const clang::SourceRange& b)
{
    if (a.getBegin() < b.getBegin()) return true;
    if (b.getBegin() < a.getBegin()) return false;

    if (a.getEnd()   < b.getEnd()  ) return true;
//  if (b.getEnd()   < a.getEnd()  ) return false;
    return false;
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
