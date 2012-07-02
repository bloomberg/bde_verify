// csabase_visitor.h                                                  -*-C++-*-
// -----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#include <csabase_visitor.h>
#include <csabase_debug.h>
#include <llvm/Support/raw_ostream.h>
#include <algorithm>
#ident "$Id: visitor.cpp 161 2011-12-28 00:20:28Z kuehl $"

// -----------------------------------------------------------------------------

#define DECL(CLASS, BASE)                                        \
void                                                             \
cool::csabase::Visitor::do_visit(clang::CLASS##Decl const* decl) \
{                                                                \
    if (this->on##CLASS##Decl)                                   \
    {                                                            \
        cool::csabase::Debug d("event on" #CLASS "Decl");        \
        this->on##CLASS##Decl(decl);                             \
    }                                                            \
}
DECL(,)
#include "clang/AST/DeclNodes.inc"

#define STMT(CLASS, PARENT)                                 \
void                                                        \
 cool::csabase::Visitor::do_visit(clang::CLASS const* stmt) \
{                                                           \
    this->on##CLASS(stmt);                                  \
}
STMT(Stmt,)
#include "clang/AST/StmtNodes.inc"
