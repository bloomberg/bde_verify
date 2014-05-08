// -*-c++-*- csabase_location.cpp 
// -----------------------------------------------------------------------------
// Copyright 2011 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#include <csabase_location.h>
#include <clang/Basic/SourceLocation.h>
#include <clang/Basic/SourceManager.h>
#include <ostream>
#ident "$Id$"

// -----------------------------------------------------------------------------

csabase::Location::Location()
    : d_file("<unknown>")
    , d_line(0)
    , d_column(0)
    , d_location()
{
}

csabase::Location::Location(clang::SourceManager const& manager,
                            clang::SourceLocation location)
: d_file()
, d_line(0)
, d_column(0)
, d_location(manager.getExpansionLoc(location))
{
    clang::PresumedLoc loc(manager.getPresumedLoc(location));
    char const* filename(loc.getFilename());
    if (filename) {
        d_file   = filename;
        d_line   = loc.getLine();
        d_column = loc.getColumn();
    }
    else {
        d_file = "<unknown>";
    }
}

// -----------------------------------------------------------------------------

std::string
csabase::Location::file() const
{
    return d_file;
}

size_t
csabase::Location::line() const
{
    return d_line;
}

size_t
csabase::Location::column() const
{
    return d_column;
}

clang::SourceLocation
csabase::Location::location() const
{
    return d_location;
}

bool
csabase::Location::operator<(csabase::Location const& rhs) const
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

llvm::raw_ostream& csabase::operator<<(
    llvm::raw_ostream& out,
    csabase::Location const& loc)
{
    return out << loc.file() << ":" << loc.line() << ":" << loc.column();
}

// -----------------------------------------------------------------------------

std::ostream& csabase::operator<<(
    std::ostream& out,
    csabase::Location const& loc)
{
    return out << loc.file() << ":" << loc.line() << ":" << loc.column();
}

// -----------------------------------------------------------------------------

bool csabase::operator==(csabase::Location const& a,
                         csabase::Location const& b)
{
    return a.file()   == b.file()
        && a.line()   == b.line()
        && a.column() == b.column();
}

// -----------------------------------------------------------------------------

bool csabase::operator!=(csabase::Location const& a,
                         csabase::Location const& b)
{
    return a.file()   != b.file()
        || a.line()   != b.line()
        || a.column() != b.column();
}

// -----------------------------------------------------------------------------

csabase::Range::Range()
{
}

csabase::Range::Range(clang::SourceManager const& manager,
                      clang::SourceRange range)
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

bool csabase::Range::operator<(csabase::Range const& rhs) const
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

llvm::raw_ostream& csabase::operator<<(
    llvm::raw_ostream& out,
    csabase::Range const& loc)
{
    return out << "[" << loc.from() << ", " << loc.to() << "]";
}

// -----------------------------------------------------------------------------

std::ostream& csabase::operator<<(std::ostream& out, csabase::Range const& loc)
{
    return out << "[" << loc.from() << ", " << loc.to() << "]";
}

// -----------------------------------------------------------------------------

bool csabase::operator==(csabase::Range const& a, csabase::Range const& b)
{
    return a.from() == b.from() && a.to() == b.to();
}

// -----------------------------------------------------------------------------

bool csabase::operator!=(csabase::Range const& a, csabase::Range const& b)
{
    return a.from() != b.from() || a.to() != b.to();
}
