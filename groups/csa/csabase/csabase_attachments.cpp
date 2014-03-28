// csabase_attachments.h                                              -*-C++-*-
// -----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#include <csabase_attachments.h>
#ident "$Id$"

namespace bde_verify {
namespace csabase {

AttachmentBase::~AttachmentBase()
{
}

Attachments::Attachments()
{
}

Attachments::~Attachments()
{
    for (size_t i = 0; i < d_attachments.size(); ++i) {
        delete d_attachments[i];
    }
}

}  // close package namespace
}  // close enterprise namespace
