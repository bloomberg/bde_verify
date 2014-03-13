// csabase_attachments.h                                              -*-C++-*-
// -----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#include <csabase_attachments.h>
#ident "$Id$"

// -----------------------------------------------------------------------------

bde_verify::csabase::AttachmentBase::~AttachmentBase()
{
}

// -----------------------------------------------------------------------------

static size_t next_index(0u);

size_t
bde_verify::csabase::Attachments::alloc()
{
    return next_index++;
}

// -----------------------------------------------------------------------------

bde_verify::csabase::Attachments::Attachments()
{
    data_.resize(next_index);
}

bde_verify::csabase::Attachments::~Attachments()
{
}

// -----------------------------------------------------------------------------

utils::shared_ptr<bde_verify::csabase::AttachmentBase>&
bde_verify::csabase::Attachments::access(size_t index)
{
    if (data_.size() <= index)
    {
        data_.resize(index + 1u);
    }
    return data_[index];
}
