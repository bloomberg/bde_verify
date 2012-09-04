// csatr_entityrestrictions.t.h                                       -*-C++-*-

#if !defined(INCLUDED_CSATR_ENTITYRESTRICTIONS)
#define INCLUDED_CSATR_ENTITYRESTRICTIONS

namespace cool
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
}

inline void cool::csatr::EntityRestrictions::legalFunction()
{
}

#endif
