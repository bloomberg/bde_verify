// csatr_entityrestrictions.t.h                                       -*-C++-*-

#ifndef INCLUDED_CSATR_ENTITYRESTRICTIONS
#define INCLUDED_CSATR_ENTITYRESTRICTIONS

#if !defined(INCLUDED_BDES_IDENT)
#  include <bdes_ident.h>
#endif
#ifndef INCLUDED_CSASCM_VERSION
#  include <csascm_version.h>
#endif

namespace bde_verify
{
    namespace csatr
    {
        struct EntityRestrictions
        {
            enum LegalEnum {};
            typedef LegalEnum LegalTypedef;
            static int legalVar;
            int legalMember;
            void legalFunction();
        };
        struct EntityRestrictionsAux
        {
        };

        void swap(EntityRestrictions&);
        void swap(EntityRestrictions&, EntityRestrictions&, EntityRestrictions&);
        void swap(EntityRestrictions&, EntityRestrictionsAux&);
        void swap(EntityRestrictions, EntityRestrictions);
        void swap(EntityRestrictions const&, EntityRestrictions const&);
        void swap(EntityRestrictions&, EntityRestrictions&);

        void operator+(EntityRestrictions);

        extern int entityRestrictionsVar;
        void entityRestrictionsFunction();

        typedef EntityRestrictions EntityRestrictionsTypedef;
        enum EntityRestrictionsEnum {};
        struct EntityRestrictionsStruct {};
        class EntityRestrictionsClass {};
        union EntityRestrictionsUnion {};
    }

    struct csatr_EntityRestrictions
    {
    };

    void swap(csatr_EntityRestrictions&, csatr_EntityRestrictions&);
    bool operator== (const csatr_EntityRestrictions&,
                    const  csatr_EntityRestrictions&);
    bool operator!= (const csatr_EntityRestrictions&,
                    const  csatr_EntityRestrictions&);
}

inline void bde_verify::csatr::EntityRestrictions::legalFunction()
{
}

// BDE_VERIFY pragma: set global_names x y z w
int x;
void y() { }
typedef int z;
enum w { v };

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
