// $Id: ubigint.h,v 1.7 2016-01-18 00:07:47-08 - - $
// Ana Carolina Alves - adalves

#ifndef __UBIGINT_H__
#define __UBIGINT_H__

#include <exception>
#include <iostream>
#include <limits>
#include <utility>
using namespace std;

#include "debug.h"
#include "relops.h"

class ubigint {
   friend ostream& operator<< (ostream&, const ubigint&);

   private:
      using quot_rem = pair<ubigint,ubigint>;
      using udigit_t = unsigned char;
      using ubigvalue_t = vector<udigit_t>;
      ubigvalue_t ubig_value;
      void assign_vector (const string&);
      quot_rem divide (const ubigint&) const;
      void remove_high_order_zeros (ubigint&) const;
      void multiply_by_2();
      void divide_by_2();

   public:
      ubigint() = default; // Need default ctor as well.
      ubigint (unsigned long);
      ubigint (const string&);

      ubigint operator+ (const ubigint&) const;
      ubigint operator- (const ubigint&) const;
      ubigint operator* (const ubigint&) const;
      ubigint operator/ (const ubigint&) const;
      ubigint operator% (const ubigint&) const;

      bool operator== (const ubigint&) const;
      bool operator<  (const ubigint&) const;

      string print_string () const;
};

#endif

