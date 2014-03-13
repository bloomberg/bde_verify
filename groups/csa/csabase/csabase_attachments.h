// csabase_attachments.h                                              -*-C++-*-
// -----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#if !defined(INCLUDED_CSABASE_ATTACHMENTS)
#define INCLUDED_CSABASE_ATTACHMENTS 1
#ident "$Id$"

#include <utils/shared_ptr.hpp>
#include <vector>
#include <stddef.h>

// -----------------------------------------------------------------------------

namespace bde_verify
{
    namespace csabase
    {
        class AttachmentBase;
        template <typename> class Attachment;
        class Attachments;
    }
}

// -----------------------------------------------------------------------------

class bde_verify::csabase::AttachmentBase
{
public:
    virtual ~AttachmentBase();
};

// -----------------------------------------------------------------------------

template <typename T>
class bde_verify::csabase::Attachment:
    public bde_verify::csabase::AttachmentBase
{
public:
    static size_t index();
    Attachment();
    T& data();

private:
    Attachment(Attachment const&);
    void operator=(Attachment const&);

    T data_;
};

// -----------------------------------------------------------------------------

class bde_verify::csabase::Attachments
{
public:
    static size_t alloc();

    Attachments();
    ~Attachments();

    template <typename T> T& attachment();
    
private:
    Attachments(Attachments const&);
    void operator=(Attachments const&);
    utils::shared_ptr<bde_verify::csabase::AttachmentBase>& access(size_t);

    std::vector<utils::shared_ptr<bde_verify::csabase::AttachmentBase> > data_;
};

// -----------------------------------------------------------------------------

template <typename T>
size_t
bde_verify::csabase::Attachment<T>::index()
{
    static size_t rc(bde_verify::csabase::Attachments::alloc());
    return rc;
}

template <typename T>
bde_verify::csabase::Attachment<T>::Attachment():
    data_()
{
}

template <typename T>
T&
bde_verify::csabase::Attachment<T>::data()
{
    return data_;
}

// -----------------------------------------------------------------------------

template <typename T>
T&
bde_verify::csabase::Attachments::attachment()
{
    utils::shared_ptr<bde_verify::csabase::AttachmentBase>& ref(access(bde_verify::csabase::Attachment<T>::index()));
    if (!ref)
    {
        ref = utils::shared_ptr<bde_verify::csabase::AttachmentBase>(new bde_verify::csabase::Attachment<T>());
    }
    // In theory, this should be a dynamic_cast<...>() but
    // 1. it isn't really necessary because it known what type is at this location
    // 2. the code gets compiled with RTTI being turned off to link to clang
    return static_cast<bde_verify::csabase::Attachment<T>&>(*ref).data();
}

// -----------------------------------------------------------------------------

#endif
