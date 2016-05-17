// csabase_location.h                                                 -*-C++-*-

#ifndef INCLUDED_CSABASE_LOCATION
#define INCLUDED_CSABASE_LOCATION

#include <clang/Basic/SourceLocation.h>
#include <stddef.h>
#include <iosfwd>
#include <string>

namespace clang { class SourceManager; }
namespace llvm { class raw_ostream; }

// -----------------------------------------------------------------------------

namespace csabase
{
class Location
{
private:
    std::string           d_file;
    size_t                d_line;
    size_t                d_column;
    clang::SourceLocation d_location;

public:
    Location();
    Location(clang::SourceManager const& manager,
             clang::SourceLocation location);
    Location(const Location&) = default;
    Location& operator=(const Location&) = default;

    std::string           file() const;
    size_t                line() const;
    size_t                column() const;
    clang::SourceLocation location() const;

    bool operator< (Location const& location) const;
    operator bool() const;
};

llvm::raw_ostream& operator<< (llvm::raw_ostream&, Location const&);
std::ostream& operator<< (std::ostream&, Location const&);
bool operator== (Location const&, Location const&);
bool operator!= (Location const&, Location const&);

class Range
{
private:
    Location d_from;
    Location d_to;

public:
    Range();
    Range(clang::SourceManager const& manager,
          clang::SourceRange range);
    Range(const Range&) = default;
    Range& operator=(const Range&) = default;

    const Location& from() const;
    const Location& to() const;

    bool operator< (Range const& range) const;
    operator bool() const;
};

llvm::raw_ostream& operator<< (llvm::raw_ostream&, Range const&);
std::ostream& operator<< (std::ostream&, Range const&);
bool operator== (Range const&, Range const&);
bool operator!= (Range const&, Range const&);
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
