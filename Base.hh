#ifndef BASE_HH
#define BASE_HH
#include "export.hh"
#include <iostream>

struct EXPORT_CLASS Base {
#if OUT_OF_LINE_DTOR
    virtual ~Base();
#else
    virtual ~Base() { }
#endif

};

#endif /* end of include guard: BASE_HH */
