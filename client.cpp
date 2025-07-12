#include <iostream>
#include "satisfier.hpp"

int main() {
   // Propositions:
   //   Socrates_is:       true
   //   Socrates_is_man:    true
   //   All_men_are_mortal: true
   suppose_literal(Socrates_is, true);
   suppose_literal(Socrates_is_man,    true);
   suppose_literal(All_men_are_mortal, true);

   // Encode implication: Socrates_is_man → All_men_are_mortal
   // as ¬Socrates_is_man ∨ All_men_are_mortal
   auto implication = Socrates_is.Not(Socrates_is_man).Or(All_men_are_mortal);

   // Now derive: Socrates_is_mortal ≡ (Socrates_is_man → All_men_are_mortal)
   suppose_literal(Socrates_is_mortal, implication.value());

   // Output
   std::cout
       << "Socrates is a man:    "
       << (Socrates_is_man.value()    ? "True\n" : "False\n")
       << "All men are mortal:   "
       << (All_men_are_mortal.value() ? "True\n" : "False\n")
       << "Implication holds:    "
       << (implication.value()              ? "True\n" : "False\n")
       << "Socrates is mortal:   "
       << (Socrates_is_mortal.value() ? "True\n" : "False\n");
   return 0;
}
