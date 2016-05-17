// csabase_attachments.h                                              -*-C++-*-

#ifndef INCLUDED_CSABASE_ATTACHMENTS
#define INCLUDED_CSABASE_ATTACHMENTS

#include <vector>
#include <stddef.h>

// -----------------------------------------------------------------------------

namespace csabase
{
struct AttachmentBase
    // The base class for attachment data.  The intent is that each csabase
    // module creates a private class to hold the data it needs, and creates an
    // attchment to hold it.
{
    virtual ~AttachmentBase();
        // Destroy this object.
};

template <typename TYPE>
class Attachment : public AttachmentBase
    // The per-type class for attachment data.  It is intended that attachments
    // be held within a global data structure of the csabase analyser, and
    // that callback objects access this structure as needed.
{
  private:
    Attachment(Attachment const&);
        // Elided copy constructor.

    void operator=(Attachment const&);
        // Elided assignment operator.

  public:
    Attachment();
        // Create an object of this type.

    TYPE& data();
        // Return a modifiable reference to the data of this attachment.

  private:
    TYPE d_data;  // The data stored by this attachment.
};

template <typename TYPE>
inline
Attachment<TYPE>::Attachment()
: d_data()
{
}

template <typename TYPE>
inline
TYPE& Attachment<TYPE>::data()
{
    return d_data;
}

class Attachments
    // The class holding all attachments.  From the outside, the attachments
    // appear indexed by their data type.
{
  private:
    Attachments(Attachments const&);
        // Elided copy constructor.

    void operator=(Attachments const&);
        // Elided assignment operator.

  public:
    Attachments();
        // Create an object of this type;

    ~Attachments();
        // Destroy this object and its attachments.

    template <class TYPE>
    TYPE& attachment();
        // Return reference offering modifiable access to the attachment for 
        // the specified 'TYPE'.

  private:
    std::vector<AttachmentBase *> d_attachments;
};

template <class TYPE>
inline
TYPE& Attachments::attachment()
{
    // The first time this is called (for the specified 'TYPE') a new
    // attachment will be created in the attachments vector.
    static size_t index = d_attachments.size();
    if (index >= d_attachments.size()) {
        d_attachments.resize(index + 1);
    }
    if (!d_attachments[index]) {
        d_attachments[index] = new Attachment<TYPE>;
    }

    // We know that the 'AttachmentBase' at 'index' is really an
    // 'Attachment<TYPE>' given the code above, so the 'static_cast' is
    // correct.
    return static_cast<Attachment<TYPE>*>(d_attachments[index])->data();
}
}

#endif

// ----------------------------------------------------------------------------
// Copyright (C) 2014 Bloomberg Finance L.P.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// ----------------------------- END-OF-FILE ----------------------------------
