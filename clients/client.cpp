#include <iostream>
#include "satisfier.hpp"

int main () 
{
   // Propositions
   Suppose_literal (Socrates_is, true);
   Suppose_literal (Socrates_is_man,    false);
   Suppose_literal (All_men_are_mortal, false);

   // Assertion
   auto assertion = Not (Socrates_is. Or (Socrates_is_man)).
                    Or (Not (All_men_are_mortal)).
                    And (Socrates_is);

   // Output
   std::cout
       << "Socrates is a man:    "
       << (Socrates_is_man. Value (/*truth value*/)    ? "true\n" : "false\n")
       << "All men are mortal:   "
       << (All_men_are_mortal. Value (/*truth value*/) ? "true\n" : "false\n")
       << "Assertion holds:    "
       << (assertion. Value ()        ? "true\n" : "false\n");

   return 0;
}
