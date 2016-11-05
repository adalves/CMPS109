// $Id: libfns.cpp,v 1.3 2016-01-18 00:59:06-08 - - $
// Ana Carolina Alves - adalves

#include <stdexcept>

#include "libfns.h"

//
// This algorithm would be more efficient with operators
// *=, /=2, and is_odd.  But we leave it here.
//

bigint pow (const bigint& base_arg, const bigint& exponent_arg) {
   bigint base (base_arg);
   bigint exponent (exponent_arg);
   static const bigint ZERO (0);
   static const bigint ONE (1);
   static const bigint TWO (2);
   DEBUGF ('^', "base = " << base << ", exponent = " << exponent);
   if (exponent < bigint (numeric_limits<long>::min()) || 
       exponent > bigint (numeric_limits<long>::max())) {
      throw range_error 
            ("libfns::pow(exponent does not fit in a long)");
   }
   if (base == ZERO) return ZERO;
   bigint result = ONE;
   if (exponent < ZERO) {
      base = ONE / base;
      exponent = - exponent;
   }
   while (exponent > ZERO) {
      if (exponent % TWO == ONE) {
         result = result * base;
         exponent = exponent - 1;
      }else {
         base = base * base;
         exponent = exponent / 2;
      }
   }
   DEBUGF ('^', "result = " << result);
   return result;
}

