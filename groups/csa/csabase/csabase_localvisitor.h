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
#if defined(COOL_CXX2011)
#  include <cool/forward.hpp>
#endif
#include <clang/AST/Stmt.h>
#include <clang/AST/StmtVisitor.h>
#include <clang/AST/Decl.h>
#include <clang/AST/DeclVisitor.h>
#include <memory>

// -----------------------------------------------------------------------------

namespace cool
{
    namespace detail
    {
#if defined(COOL_CXX2011)
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

#if defined(COOL_CXX2011)
template <>
class cool::detail::visitor<>:
    public cool::csabase::AbstractVisitor
{
};
#endif

// -----------------------------------------------------------------------------

#if defined(COOL_2011)
template <typename F0, typename... F>
class cool::detail::visitor<F0, F...>:
    public cool::detail::visitor<F...>
{
public:
    typedef typename F0::argument_type argument_type;
    visitor(F0&& functor, F&&... functors):
        cool::detail::visitor<F...>(cool::forward<F>(functors)...),
        functor_(cool::forward<F0>(functor))
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

#if !defined(COOL_2011)
template <typename F0>
class cool::detail::visitor:
    public cool::csabase::AbstractVisitor
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

class cool::local_visitor
{
public:
#if defined(COOL_2011)
    template <typename... F>
    local_visitor(F&&... functors):
        visitor_(new cool::detail::visitor<F...>(cool::forward<F>(functors)...))
#else
    template <typename F0>
    local_visitor(F0 const& functor):
        visitor_(new cool::detail::visitor<F0>(functor))
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
    std::auto_ptr<cool::csabase::AbstractVisitor> visitor_;
};

// -----------------------------------------------------------------------------

#endif /* FRAMEWORK_LOCAL_VISITOR_HPP */
