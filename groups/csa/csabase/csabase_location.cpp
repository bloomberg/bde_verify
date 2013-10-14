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

cool::csabase::Location::Location()
    : d_file("<unknown>")
    , d_line(0)
    , d_column(0)
{
}

cool::csabase::Location::Location(clang::SourceManager const& manager,
                                  clang::SourceLocation       location)
    : d_file()
    , d_line(0)
    , d_column(0)
{
    clang::PresumedLoc loc(manager.getPresumedLoc(location));
    char const* filename(loc.getFilename());
    if (filename) {
        this->d_file   = filename;
        this->d_line   = loc.getLine();
        this->d_column = loc.getColumn();
    }
    else {
        this->d_file = "<unknown>";
    }
}

#if 0
cool::csabase::Location::Location(std::string const& file, size_t line, size_t column)
    : d_file(file)
    , d_line(line)
    , d_column(column)
{
}
#endif

// -----------------------------------------------------------------------------

std::string
cool::csabase::Location::file() const
{
    return this->d_file;
}

size_t
cool::csabase::Location::line() const
{
    return this->d_line;
}

size_t
cool::csabase::Location::column() const
{
    return this->d_column;
}

bool
cool::csabase::Location::operator< (cool::csabase::Location const& rhs) const
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
cool::csabase::operator<< (llvm::raw_ostream& out, cool::csabase::Location const& loc)
{
    return out << loc.file() << ":" << loc.line() << ":" << loc.column();
}

// -----------------------------------------------------------------------------

std::ostream&
cool::csabase::operator<< (std::ostream& out, cool::csabase::Location const& loc)
{
    return out << loc.file() << ":" << loc.line() << ":" << loc.column();
}

// -----------------------------------------------------------------------------

bool
cool::csabase::operator==(cool::csabase::Location const& a,
                          cool::csabase::Location const& b)
{
    return a.file()   == b.file()
        && a.line()   == b.line()
        && a.column() == b.column();
}

// -----------------------------------------------------------------------------

bool
cool::csabase::operator!=(cool::csabase::Location const& a,
                          cool::csabase::Location const& b)
{
    return a.file()   != b.file()
        || a.line()   != b.line()
        || a.column() != b.column();
}
