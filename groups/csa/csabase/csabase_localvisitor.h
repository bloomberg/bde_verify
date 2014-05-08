// -*-c++-*- framework/local_visitor.hpp 
// -----------------------------------------------------------------------------
// Copyright 2011 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#if !defined(FRAMEWORK_LOCAL_VISITOR_HPP)
#define FRAMEWORK_LOCAL_VISITOR_HPP 1
#ident "$Id: local_visitor.hpp 141 2011-09-29 18:59:08Z kuehl $"

#include <csabase_abstractvisitor.h>
#if defined(UTILS_CXX2011)
#  include <utils/forward.hpp>
#endif
#include <clang/AST/Stmt.h>
#include <clang/AST/StmtVisitor.h>
#include <clang/AST/Decl.h>
#include <clang/AST/DeclVisitor.h>
#include <memory>

// -----------------------------------------------------------------------------

namespace bde_verify
{
    namespace detail
    {
#if defined(UTILS_CXX2011)
        template <typename...>                class visitor;
        template <>                           class visitor<>;
        template <typename F0, typename... F> class visitor<F0, F...>;
#else
        template <typename F0> class visitor;
#endif
    }
    class local_visitor;
}

// -----------------------------------------------------------------------------

#if defined(UTILS_CXX2011)
template <>
class bde_verify::detail::visitor<>:
    public csabase::AbstractVisitor
{
};
#endif

// -----------------------------------------------------------------------------

#if defined(UTILS_2011)
template <typename F0, typename... F>
class bde_verify::detail::visitor<F0, F...>:
    public bde_verify::detail::visitor<F...>
{
public:
    typedef typename F0::argument_type argument_type;
    visitor(F0&& functor, F&&... functors):
        bde_verify::detail::visitor<F...>(bde_verify::forward<F>(functors)...),
        functor_(bde_verify::forward<F0>(functor))
    {
    }
    void do_visit(argument_type arg)
    {
        functor_(arg);
    }
private:
    F0 functor_;
};
#endif

// -----------------------------------------------------------------------------

#if !defined(UTILS_2011)
template <typename F0>
class bde_verify::detail::visitor:
    public csabase::AbstractVisitor
{
public:
    typedef typename F0::argument_type argument_type;
    visitor(F0 const& functor):
        functor_(functor)
    {
    }
    void do_visit(argument_type arg)
    {
        functor_(arg);
    }
private:
    F0 functor_;
};
#endif

// -----------------------------------------------------------------------------

class bde_verify::local_visitor
{
public:
#if defined(UTILS_2011)
    template <typename... F>
    local_visitor(F&&... functors):
        visitor_(new bde_verify::detail::visitor<F...>(bde_verify::forward<F>(functors)...))
#else
    template <typename F0>
    local_visitor(F0 const& functor):
        visitor_(new bde_verify::detail::visitor<F0>(functor))
#endif
    {
    }
    void visit(clang::Decl const* ptr)
    {
        visitor_->visit(ptr);
    }
    void visit(clang::Stmt const* ptr)
    {
        visitor_->visit(ptr);
    }
    
private:
    std::auto_ptr<csabase::AbstractVisitor> visitor_;
};

// -----------------------------------------------------------------------------

#endif /* FRAMEWORK_LOCAL_VISITOR_HPP */
