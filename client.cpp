#include <iostream>
#include "satisfier.hpp"

int main() {
   // Propositions
   suppose_literal(Socrates_is, true);
   suppose_literal(Socrates_is_man,    false);
   suppose_literal(All_men_are_mortal, false);
   //suppose_literal(n1, true);

   // Encode implication
   auto implication = Not(Socrates_is.Or(Socrates_is_man)).Or(Not(All_men_are_mortal));

   // Output
   std::cout
       << "Socrates is a man:    "
       << (Socrates_is_man.value()    ? "true\n" : "false\n")
       << "All men are mortal:   "
       << (All_men_are_mortal.value() ? "true\n" : "false\n")
       << "Implication holds:    "
       << (implication.value()              ? "true\n" : "false\n");

   return 0;
}
