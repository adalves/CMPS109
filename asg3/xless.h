// $Id: xless.h,v 1.1 2016-02-10 13:28:40-08 - - $
// Ana Carolina Alves - adalves

#ifndef __XLESS_H__
#define __XLESS_H__

//
// We assume that the type type_t has an operator< function.
//

template <typename Type>
struct xless {
   bool operator() (const Type& left, const Type& right) const {
      return left < right;
   }
};

#endif

