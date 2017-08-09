// csabase_location.cpp                                               -*-C++-*-

#include <csabase_location.h>
#include <clang/Basic/SourceLocation.h>
#include <clang/Basic/SourceManager.h>
#include <llvm/Support/raw_ostream.h>
#include <ostream>

using namespace csabase;
using namespace clang;

// -----------------------------------------------------------------------------

csabase::Location::Location()
    : d_file("<unknown>")
    , d_line(0)
    , d_column(0)
    , d_location()
{
}

csabase::Location::Location(SourceManager const& manager,
                            SourceLocation location)
: d_file()
, d_line(0)
, d_column(0)
, d_location(manager.getExpansionLoc(location))
{
    PresumedLoc loc(manager.getPresumedLoc(location));
    if (loc.isValid()) {
        d_file   = loc.getFilename();
        d_line   = loc.getLine();
        d_column = loc.getColumn();
    }
    else {
        d_file = "<unknown>";
    }
}

// -----------------------------------------------------------------------------

std::string csabase::Location::file() const
{
    return d_file;
}

size_t csabase::Location::line() const
{
    return d_line;
}

size_t csabase::Location::column() const
{
    return d_column;
}

SourceLocation csabase::Location::location() const
{
    return d_location;
}

bool csabase::Location::operator<(Location const& rhs) const
{
    if (d_file < rhs.d_file) {
        return true;
    }
    if (rhs.d_file < d_file) {
        return false;
    }
    if (d_line < rhs.d_line) {
        return true;
    }
    if (rhs.d_line < d_line) {
        return false;
    }
    if (d_column < rhs.d_column) {
        return true;
    }
    if (rhs.d_column < d_column) {
        return false;
    }
    return false;
}

csabase::Location::operator bool() const
{
    return d_location.isValid();
}

// -----------------------------------------------------------------------------

llvm::raw_ostream& csabase::operator<<(llvm::raw_ostream& out,
                                       Location const& loc)
{
    return out << loc.file() << ":" << loc.line() << ":" << loc.column();
}

// -----------------------------------------------------------------------------

std::ostream& csabase::operator<<(std::ostream& out, Location const& loc)
{
    return out << loc.file() << ":" << loc.line() << ":" << loc.column();
}

// -----------------------------------------------------------------------------

bool csabase::operator==(Location const& a, Location const& b)
{
    return a.file()   == b.file()
        && a.line()   == b.line()
        && a.column() == b.column();
}

// -----------------------------------------------------------------------------

bool csabase::operator!=(Location const& a, Location const& b)
{
    return a.file()   != b.file()
        || a.line()   != b.line()
        || a.column() != b.column();
}

// -----------------------------------------------------------------------------

csabase::Range::Range()
{
}

csabase::Range::Range(SourceManager const& manager, SourceRange range)
: d_from(manager, range.getBegin())
, d_to(manager, range.getEnd())
{
}

// -----------------------------------------------------------------------------

const csabase::Location& csabase::Range::from() const
{
    return d_from;
}

const csabase::Location& csabase::Range::to() const
{
    return d_to;
}

bool csabase::Range::operator<(Range const& rhs) const
{
    if (d_from < rhs.d_from) {
        return true;
    }
    if (rhs.d_from < d_from) {
        return false;
    }
    if (d_to < rhs.d_to) {
        return true;
    }
    if (rhs.d_to < d_to) {
        return false;
    }
    return false;
}

csabase::Range::operator bool() const
{
    return d_from && d_to && d_from.file() == d_to.file();
}

// -----------------------------------------------------------------------------

llvm::raw_ostream& csabase::operator<<(llvm::raw_ostream& out,
                                       Range const& loc)
{
    return out << "[" << loc.from() << ", " << loc.to() << "]";
}

// -----------------------------------------------------------------------------

std::ostream& csabase::operator<<(std::ostream& out, Range const& loc)
{
    return out << "[" << loc.from() << ", " << loc.to() << "]";
}

// -----------------------------------------------------------------------------

bool csabase::operator==(Range const& a, Range const& b)
{
    return a.from() == b.from() && a.to() == b.to();
}

// -----------------------------------------------------------------------------

bool csabase::operator!=(Range const& a, Range const& b)
{
    return a.from() != b.from() || a.to() != b.to();
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
