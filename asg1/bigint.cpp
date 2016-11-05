// $Id: bigint.cpp,v 1.6 2016-01-18 00:37:37-08 - - $
// Ana Carolina Alves - adalves

#include <cstdlib>
#include <exception>
#include <stack>
#include <stdexcept>
using namespace std;

#include "bigint.h"
#include "debug.h"
#include "relops.h"

bigint::bigint (long that): uvalue (that), is_negative (that < 0) {
   DEBUGF ('~', this << " -> " << uvalue)
}

bigint::bigint (const ubigint& uvalue, bool is_negative):
                uvalue(uvalue), is_negative(is_negative) {
}

bigint::bigint (const string& that) {
   is_negative = that.size() > 0 and that[0] == '_';
   uvalue = ubigint (that.substr (is_negative ? 1 : 0));
}

bigint bigint::operator+() const {
   return *this;
}

bigint bigint::operator-() const {
   return {uvalue, not is_negative};
}

//
// Overloads the + operator
// Checks signs and calls ubigint to perform the calculations
//
bigint bigint::operator+ (const bigint& that) const {
   bigint result;
   if (is_negative == that.is_negative){
      result = uvalue + that.uvalue;
      result.is_negative = is_negative;
   } else {
      if (uvalue > that.uvalue) {
         result = uvalue - that.uvalue;
         result.is_negative = is_negative;
      } else {
         result = that.uvalue - uvalue;
         result.is_negative = that.is_negative;
      }
   }

   deal_with_zero(result);

   return result;
}

//
// Overloads the - operator
// Checks signs and calls ubigint to perform the calculations
//
bigint bigint::operator- (const bigint& that) const {
   bigint result;
   if (is_negative == that.is_negative) {
      if (uvalue > that.uvalue) {
         result = uvalue - that.uvalue;
         result.is_negative = is_negative;
      } else {
         result = that.uvalue - uvalue;
         result.is_negative = not that.is_negative;
      }
   } else {
      result = uvalue + that.uvalue;
      result.is_negative = is_negative;
   }

   deal_with_zero(result);

   return result;
}

//
// Overloads the * operator
// Calls ubigint to perform the calculations and uses
// the rule of signs to determine the result
//
bigint bigint::operator* (const bigint& that) const {
   bigint result = uvalue * that.uvalue;
   result.is_negative = is_negative != that.is_negative;

   deal_with_zero(result);

   return result;
}

//
// Overloads the / operator
// Calls ubigint to perform the calculations and uses
// the rule of signs to determine the result
//
bigint bigint::operator/ (const bigint& that) const {
   static const bigint zero = 0;
   if (that == zero) throw domain_error ("bigint::divide: by 0");
   bigint result = uvalue / that.uvalue;
   result.is_negative = is_negative != that.is_negative;

   deal_with_zero(result);

   return result;
}

//
// Overloads the % operator
// Calls ubigint to perform the calculations 
// and determines the sign based on the dividend
//
bigint bigint::operator% (const bigint& that) const {
   bigint result = uvalue % that.uvalue;
   result.is_negative = is_negative;

   deal_with_zero(result);
   
   return result;
}

//
// Overloads the == sign
// First checks the signs of the numbers, 
// and then calls ubigint
// 
bool bigint::operator== (const bigint& that) const {
   return is_negative == that.is_negative and uvalue == that.uvalue;
}

//
// Overloads the < sign
// First checks the signs of the numbers, 
// and if necessary calls ubigint
// 
bool bigint::operator< (const bigint& that) const {
   if (is_negative != that.is_negative) return is_negative;
   return is_negative ? uvalue > that.uvalue
                      : uvalue < that.uvalue;
}

//
// Makes sure the sign of 0 is positive
//
void bigint::deal_with_zero (bigint& that) const {
   static const ubigint empty;
   if (that.uvalue == empty) that.is_negative = false;
}

// 
// Overloads the << operator
// If the number is negative, calls uvalue.print_string() to 
// be able to format the output correctly
// If not, uses the overloaded << operator in ubigint
//
ostream& operator<< (ostream& out, const bigint& that) {
   if (that.is_negative) {
      string uvalue_string = that.uvalue.print_string();
      out << "-";
      int i = 1;
      const int line_length = 69;
      for (char digit: uvalue_string) {
         if (i == line_length) {
            out << "\\" << endl;
            i = 0;
         }
         out << to_string(digit);
      }
   } else {
      out << that.uvalue;
   }
   return out;
}

