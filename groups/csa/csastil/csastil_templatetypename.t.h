// csastil_templatetypename.t.h                                       -*-C++-*-

#if !defined(INCLUDED_CSASTIL_TEMPLATETYPENAME)
#define INCLUDED_CSASTIL_TEMPLATETYPENAME

namespace cool
{
    namespace csastil
    {
        template <typename T>
        struct TemplateTypenameBad
        {
        public:
            template <typename S> TemplateTypenameBad();
            template <typename S> void foo();
        };

        struct TemplateTypenameMember
        {
        public:
            template <typename S> TemplateTypenameMember();
            template <typename S> void foo();
        };

        template <typename T>
        void swap(TemplateTypenameBad<T>&, TemplateTypenameBad<T>&);

        template <class T>
        class TemplateTypenameGood
        {
        public:
            template <class S> TemplateTypenameGood();
            template <class S> void foo();
        };

        template <class T>
        void swap(TemplateTypenameGood<T>&, TemplateTypenameGood<T>&);
    }
}

template <typename T>
inline void
cool::csastil::swap(TemplateTypenameBad<T>&, TemplateTypenameBad<T>&)
{
}

#endif
