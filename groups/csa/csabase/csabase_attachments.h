// csabase_attachments.h                                              -*-C++-*-
// -----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#if !defined(INCLUDED_CSABASE_ATTACHMENTS)
#define INCLUDED_CSABASE_ATTACHMENTS 1
#ident "$Id$"

#include <cool/shared_ptr.hpp>
#include <vector>
#include <stddef.h>

// -----------------------------------------------------------------------------

namespace cool
{
    namespace csabase
    {
        class AttachmentBase;
        template <typename> class Attachment;
        class Attachments;
    }
}

// -----------------------------------------------------------------------------

class cool::csabase::AttachmentBase
{
public:
    virtual ~AttachmentBase();
};

// -----------------------------------------------------------------------------

template <typename T>
class cool::csabase::Attachment:
    public cool::csabase::AttachmentBase
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

class cool::csabase::Attachments
{
public:
    static size_t alloc();

    Attachments();
    ~Attachments();

    template <typename T> T& attachment();
    
private:
    Attachments(Attachments const&);
    void operator=(Attachments const&);
    cool::shared_ptr<cool::csabase::AttachmentBase>& access(size_t);

    std::vector<cool::shared_ptr<cool::csabase::AttachmentBase> > data_;
};

// -----------------------------------------------------------------------------

template <typename T>
size_t
cool::csabase::Attachment<T>::index()
{
    static size_t rc(cool::csabase::Attachments::alloc());
    return rc;
}

template <typename T>
cool::csabase::Attachment<T>::Attachment():
    data_()
{
}

template <typename T>
T&
cool::csabase::Attachment<T>::data()
{
    return data_;
}

// -----------------------------------------------------------------------------

template <typename T>
T&
cool::csabase::Attachments::attachment()
{
    cool::shared_ptr<cool::csabase::AttachmentBase>& ref(access(cool::csabase::Attachment<T>::index()));
    if (!ref)
    {
        ref = cool::shared_ptr<cool::csabase::AttachmentBase>(new cool::csabase::Attachment<T>());
    }
    // In theory, this should be a dynamic_cast<...>() but
    // 1. it isn't really necessary because it known what type is at this location
    // 2. the code gets compiled with RTTI being turned off to link to clang
    return static_cast<cool::csabase::Attachment<T>&>(*ref).data();
}

// -----------------------------------------------------------------------------

#endif
