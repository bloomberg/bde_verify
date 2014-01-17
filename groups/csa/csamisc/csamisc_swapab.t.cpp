// csamisc_swapab.t.cpp                                               -*-C++-*-

namespace cool
{
    namespace {
        void swap(int  x, int  y);
        void swap(int *x, int &y);
        void swap(int *x, int *y);
        void swap(int *x, int *b);
        void swap(int *a, int *y);
        void swap(int *a, int *b);
        void swap(int &x, int &y);
        void swap(int &x, int &b);
        void swap(int &a, int &y);
        void swap(int &a, int &b);
        void swap(int &a, int &b);
        void swap(int &a, int & );
        void swap(int & , int &b);
        void swap(int & , int & );
        void swap(int * , int *b);
        void swap(int *a, int * );
        void swap(int * , int * );
            // Exchange the specified items.

        template <class SS> void swap(SS  x, SS  y);
        template <class SS> void swap(SS *x, SS &y);
        template <class SS> void swap(SS *x, SS *y);
        template <class SS> void swap(SS *x, SS *b);
        template <class SS> void swap(SS *a, SS *y);
        template <class SS> void swap(SS *a, SS *b);
        template <class SS> void swap(SS &x, SS &y);
        template <class SS> void swap(SS &x, SS &b);
        template <class SS> void swap(SS &a, SS &y);
        template <class SS> void swap(SS &a, SS &b);
        template <class SS> void swap(SS &a, SS &b);
        template <class SS> void swap(SS &a, SS & );
        template <class SS> void swap(SS & , SS &b);
        template <class SS> void swap(SS & , SS & );
        template <class SS> void swap(SS * , SS *b);
        template <class SS> void swap(SS *a, SS * );
        template <class SS> void swap(SS * , SS * );
            // Exchange the specified items.
    }
}
