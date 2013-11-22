// csabase_attachments.h                                              -*-C++-*-
// -----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#include <csabase_attachments.h>
#ident "$Id$"

// -----------------------------------------------------------------------------

cool::csabase::AttachmentBase::~AttachmentBase()
{
}

// -----------------------------------------------------------------------------

static size_t next_index(0u);

size_t
cool::csabase::Attachments::alloc()
{
    return next_index++;
}

// -----------------------------------------------------------------------------

cool::csabase::Attachments::Attachments()
{
    this->data_.resize(next_index);
}

cool::csabase::Attachments::~Attachments()
{
}

// -----------------------------------------------------------------------------

cool::shared_ptr<cool::csabase::AttachmentBase>&
cool::csabase::Attachments::access(size_t index)
{
    if (this->data_.size() <= index)
    {
        this->data_.resize(index + 1u);
    }
    return this->data_[index];
}
