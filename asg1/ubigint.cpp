// $Id: ubigint.cpp,v 1.16 2016-01-18 00:37:37-08 - - $
// Ana Carolina Alves - adalves

#include <cstdlib>
#include <exception>
#include <stack>
#include <stdexcept>
using namespace std;

#include "ubigint.h"
#include "debug.h"

ubigint::ubigint (unsigned long that) {
   assign_vector (to_string(that));
}

ubigint::ubigint (const string& that) {
   assign_vector (that);
}

//
// Stores the digits from least significant order to most significant
//
void ubigint::assign_vector (const string& that) {
   for (auto itor = that.crbegin(); itor != that.crend(); ++itor) {
      ubig_value.push_back (*itor - '0');
   }
}

//
// Overloads the + operator
//
ubigint ubigint::operator+ (const ubigint& that) const {
   ubigint result;
   auto min_itor = ubig_value.cbegin();
   auto min_itor_end = ubig_value.cend();
   auto max_itor = that.ubig_value.cbegin();
   auto max_itor_end = that.ubig_value.cend();
   udigit_t digit = 0;
   udigit_t carry = 0;

   // Changes the values of the iterators 
   // if "this" is bigger than "that"
   if (ubig_value.size() > that.ubig_value.size()) {
      min_itor = that.ubig_value.cbegin();
      min_itor_end = that.ubig_value.cend();
      max_itor = ubig_value.cbegin();
      max_itor_end = ubig_value.cend();
   }

   // Loops until the smaller vector ends, 
   // adding every pair of digits and "carry"
   // e.g. 200 + 25 -> 25
   for (; min_itor != min_itor_end; ++min_itor, ++max_itor) {
      digit = *min_itor + *max_itor + carry;
      if (digit >= 10) {
         carry = 1;
         digit -= 10;
      }
      else carry = 0;
      result.ubig_value.push_back (digit);
   }

   // Loops from where it stopped until the bigger vector ends,
   // adding its digit and "carry"
   // e.g. 200 + 25 = 25 -> 225
   for (; max_itor != max_itor_end; ++max_itor) {
      digit = *max_itor + carry;
      if (digit >= 10) {
         carry = 1;
         digit -= 10;
      }
      else carry = 0;
      result.ubig_value.push_back (digit);
   }
   
   // If carry still has 1, put it on the end of the result
   // e.g. 60 + 70 = 30 -> 130
   if (carry != 0)
   result.ubig_value.push_back (1);

   return result;
}

//
// Overloads the - operator
// Assumes that this > that
//
ubigint ubigint::operator- (const ubigint& that) const {
   if (*this < that) throw domain_error ("ubigint::operator-(a<b)");
   
   ubigint result;
   auto this_itor = ubig_value.cbegin();
   auto this_itor_end = ubig_value.cend();
   auto that_itor = that.ubig_value.cbegin();
   auto that_itor_end = that.ubig_value.cend();
   udigit_t digit = 0;
   udigit_t borrow = 0;

   // Loops until the smaller vector ends,
   // subtracting every pair of digits and "borrow"
   // e.g. 150 - 25 -> 25
   for (; that_itor != that_itor_end; ++that_itor, ++this_itor) {
      // Checks if this digit < that digit or
      // if they're equal and have a borrow
      // which would result in -1.
      if (*this_itor < *that_itor || 
          (*this_itor == *that_itor && borrow != 0)) {
         digit = (*this_itor + 10) - *that_itor - borrow;
         borrow = 1;
      } else {
         digit = *this_itor - *that_itor - borrow;
         borrow = 0;
      }
      result.ubig_value.push_back (digit);
   }

   // Loops until the bigger vector ends,
   // subtracting its digit and "borrow"
   // e.g. 150 - 25 = 25 -> 125
   for (; this_itor != this_itor_end; ++this_itor) {
      if (*this_itor == 0 && borrow != 0) {
         digit = (*this_itor + 10) - borrow;
         borrow = 1;
      } else {
         digit = *this_itor - borrow;
         borrow = 0;
      }
      result.ubig_value.push_back (digit);
   }
   
   remove_high_order_zeros (result);

   return result;
}

//
// Multiplies two values
//
ubigint ubigint::operator* (const ubigint& that) const {
   ubigint result;
   result.ubig_value.resize(ubig_value.size() + 
                            that.ubig_value.size());
   udigit_t c = 0;
   udigit_t d = 0;

   // Follows the same logic as multiplication by hand.
   // Loops through "that" inside another "for"
   // that loops through "this", multiplying a digit in "this"
   // at a time for every digit in "that"
   for (size_t i = 0; i < ubig_value.size(); ++i) {
      c = 0;
      for (size_t j = 0; j < that.ubig_value.size(); ++j) {
         d = result.ubig_value[i + j] + 
             (ubig_value[i] * that.ubig_value[j]) + c;
         result.ubig_value[i + j] = d % 10;
         c = d / 10;
      }
      result.ubig_value[i + that.ubig_value.size()] = c;
   }
  
   remove_high_order_zeros(result);

   return result;
}

//
// Removes high order zeros
//
void ubigint::remove_high_order_zeros (ubigint& that) const {
   while (that.ubig_value.size() > 0 && that.ubig_value.back() == 0)
      that.ubig_value.pop_back();
}

//
// Multiplies a value by 2
//
void ubigint::multiply_by_2() {
   static const ubigint two = 2;
   *this = *this * two;
}

//
// Divides a value by 2
// Loops through the vector, dividing every digit by 2
// If [i + 1] is odd, adds 5 to [i] and subtracts 1 from [i + 1]
//
void ubigint::divide_by_2() {
   auto itor = ubig_value.cbegin();
   size_t i = 0;

   for (; itor != ubig_value.cend(); ++itor, ++i) {
      ubig_value[i] /= 2;
      if (i + 1 < ubig_value.size() && ubig_value[i + 1] % 2 != 0) {
         ubig_value[i] += 5;
         ubig_value[i + 1] -= 1;
      }
   }
}

//
// Returns the quotient and remainder of a division,
// using the ancient Egyptian algorithm
//
ubigint::quot_rem ubigint::divide (const ubigint& that) const {
   static const ubigint zero = 0;
   if (that == zero) throw domain_error ("ubigint::divide: by 0");
   ubigint power_of_2 = 1;
   ubigint divisor = that; // right operand, divisor
   ubigint quotient = 0;
   ubigint remainder = *this; // left operand, dividend
   while (divisor < remainder) {
      divisor.multiply_by_2();
      power_of_2.multiply_by_2();
   }
   while (power_of_2 > zero) {
      if (divisor <= remainder) {
         remainder = remainder - divisor;
         quotient = quotient + power_of_2;
      }
      divisor.divide_by_2();
      power_of_2.divide_by_2();
      remove_high_order_zeros (power_of_2);
      remove_high_order_zeros (divisor);
   }
   
   return {quotient, remainder};
}

//
// Overloads the / operator
// Calls "divide" and returns the first result
//
ubigint ubigint::operator/ (const ubigint& that) const {
   return divide (that).first;
}

//
// Overloads the % operator
// Calls "divide" and returns the second result
//
ubigint ubigint::operator% (const ubigint& that) const {
   return divide (that).second;
}

//
// Overloads the == operator
// Checks the size of the vectors first, then iterates
// them to determine the equality
//
bool ubigint::operator== (const ubigint& that) const {
   if (ubig_value.size() != that.ubig_value.size()) {
      return false;
   } else {
      auto this_itor = ubig_value.crbegin();
      auto that_itor = that.ubig_value.crbegin();
      for (; this_itor != ubig_value.crend(); 
           ++this_itor, ++that_itor) {
         if (*this_itor != *that_itor) return false;
      }
   }
   return true;
}

//
// Overloards the < operator
// Checks the size of the vectors first, then iterates
// them to determine the result
//
bool ubigint::operator< (const ubigint& that) const {
   if (ubig_value.size() < that.ubig_value.size()) return true;
   else if (ubig_value.size() > that.ubig_value.size()) return false;
   else {
      auto this_itor = ubig_value.crbegin();
      auto that_itor = that.ubig_value.crbegin();
      for (; this_itor != ubig_value.crend(); 
           ++this_itor, ++that_itor) {
         if (*this_itor < *that_itor) return true;
         else if (*this_itor > *that_itor) return false;
      }
   }
   return false;
}

//
// Returns a string representation of ubigint.
// It is called in bigint to print negative numbers
//
string ubigint::print_string () const{
   string return_string;
   for (auto itor = ubig_value.crbegin(); 
           itor != ubig_value.crend(); ++itor) {
         return_string += *itor;
      }
   return return_string;
}

//
// Overloads the << operator
//
ostream& operator<< (ostream& out, const ubigint& that) {
   if (that.ubig_value.size() == 0) {
      out << "0";
   } else {
      int i = 0;
      const int line_length = 69;
      for (auto itor = that.ubig_value.crbegin(); 
           itor != that.ubig_value.crend(); ++itor, ++i) {
         if (i == line_length) {
            out << "\\" << endl;
            i = 0;
         }
         out << to_string(*itor);
      }
   }
   return out;
}

