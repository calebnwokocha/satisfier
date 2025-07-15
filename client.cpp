#include <iostream>
#include "satisfier.hpp"

int main() {
   // Propositions
   suppose_literal(Socrates_is, true);
   suppose_literal(Socrates_is_man,    false);
   suppose_literal(All_men_are_mortal, false);

   // Assertion
   auto assertion = Not(Socrates_is.Or(Socrates_is_man)).
                      Or(Not(All_men_are_mortal)).
                      And(Socrates_is);

   // Output
   std::cout
       << "Socrates is a man:    "
       << (Socrates_is_man.value()    ? "true\n" : "false\n")
       << "All men are mortal:   "
       << (All_men_are_mortal.value() ? "true\n" : "false\n")
       << "Assertion holds:    "
       << (assertion.value()        ? "true\n" : "false\n");

   return 0;
}
