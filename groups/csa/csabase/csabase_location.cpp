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

bde_verify::csabase::Location::Location()
    : d_file("<unknown>")
    , d_line(0)
    , d_column(0)
    , d_location()
{
}

bde_verify::csabase::Location::Location(clang::SourceManager const& manager,
                                  clang::SourceLocation       location)
    : d_file()
    , d_line(0)
    , d_column(0)
    , d_location(location)
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
bde_verify::csabase::Location::file() const
{
    return d_file;
}

size_t
bde_verify::csabase::Location::line() const
{
    return d_line;
}

size_t
bde_verify::csabase::Location::column() const
{
    return d_column;
}

clang::SourceLocation
bde_verify::csabase::Location::location() const
{
    return d_location;
}

bool
bde_verify::csabase::Location::operator< (bde_verify::csabase::Location const& rhs) const
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

// -----------------------------------------------------------------------------

llvm::raw_ostream&
bde_verify::csabase::operator<< (llvm::raw_ostream& out, bde_verify::csabase::Location const& loc)
{
    return out << loc.file() << ":" << loc.line() << ":" << loc.column();
}

// -----------------------------------------------------------------------------

std::ostream&
bde_verify::csabase::operator<< (std::ostream& out, bde_verify::csabase::Location const& loc)
{
    return out << loc.file() << ":" << loc.line() << ":" << loc.column();
}

// -----------------------------------------------------------------------------

bool
bde_verify::csabase::operator==(bde_verify::csabase::Location const& a,
                          bde_verify::csabase::Location const& b)
{
    return a.file()   == b.file()
        && a.line()   == b.line()
        && a.column() == b.column();
}

// -----------------------------------------------------------------------------

bool
bde_verify::csabase::operator!=(bde_verify::csabase::Location const& a,
                          bde_verify::csabase::Location const& b)
{
    return a.file()   != b.file()
        || a.line()   != b.line()
        || a.column() != b.column();
}

// -----------------------------------------------------------------------------

bde_verify::csabase::Range::Range()
{
}

bde_verify::csabase::Range::Range(clang::SourceManager const& manager,
                            clang::SourceRange range)
    : d_from(manager, range.getBegin())
    , d_to(manager, range.getEnd())
{
}

// -----------------------------------------------------------------------------

const bde_verify::csabase::Location&
bde_verify::csabase::Range::from() const
{
    return d_from;
}

const bde_verify::csabase::Location&
bde_verify::csabase::Range::to() const
{
    return d_to;
}

bool
bde_verify::csabase::Range::operator< (bde_verify::csabase::Range const& rhs) const
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

// -----------------------------------------------------------------------------

llvm::raw_ostream&
bde_verify::csabase::operator<< (llvm::raw_ostream& out, bde_verify::csabase::Range const& loc)
{
    return out << "[" << loc.from() << ", " << loc.to() << "]";
}

// -----------------------------------------------------------------------------

std::ostream&
bde_verify::csabase::operator<< (std::ostream& out, bde_verify::csabase::Range const& loc)
{
    return out << "[" << loc.from() << ", " << loc.to() << "]";
}

// -----------------------------------------------------------------------------

bool
bde_verify::csabase::operator==(bde_verify::csabase::Range const& a,
                          bde_verify::csabase::Range const& b)
{
    return a.from() == b.from() && a.to() == b.to();
}

// -----------------------------------------------------------------------------

bool
bde_verify::csabase::operator!=(bde_verify::csabase::Range const& a,
                          bde_verify::csabase::Range const& b)
{
    return a.from() != b.from() || a.to() != b.to();
}
