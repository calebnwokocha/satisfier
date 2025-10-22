#include <iostream>
#include "satisfier.hpp"

int main (/* implementation-defined */)
{
   // Propositions
   Suppose_literal (Socrates_is, true);
   Suppose_literal (Socrates_is_man,    false);
   Suppose_literal (All_men_are_mortal, false);

   // Assertion
   auto assertion = Not (Socrates_is. Or (Socrates_is_man)).
                    Or (Not (All_men_are_mortal)).
                    And (Socrates_is);

   Suppose_literal (Adverb, true);
   Suppose_literal (Conjunction, true);
   Suppose_literal (Pronoun, true);

   // Definitions of "when" in data from Oxford Languages
   Suppose_literal (at_what_time, true); // Adverb
   Suppose_literal (how_soon, true); // Adverb
   Suppose_literal (in_what_circumstances, true); // Adverb
   Suppose_literal (at_or_on_which_and_referring_to_a_time_or_circumstance, true); // Adverb
   Suppose_literal (at_or_during_the_time_that, true); // Conjunction
   Suppose_literal (after, true); // Conjunction
   Suppose_literal (at_any_time_that_and_whenever, true); // Conjunction
   Suppose_literal (after_which_and_just_then_and_implying_suddenness, true); // Conjunction
   Suppose_literal (in_view_of_the_fact_that_and_considering_that, true); // Conjunction
   Suppose_literal (although_and_whereas, true); // Conjunction

   // Definitions of "when" in Merriam-Webster dictionary
   Suppose_literal (at_or_during_which_time, true); // Adverb
   Suppose_literal (at_a_former_and_usually_less_prosperous_time, true); // Adverb
   Suppose_literal (just_at_the_moment_that, true); // Conjunction
   Suppose_literal (at_any_or_every_time_that, true); // Conjunction
   Suppose_literal (in_the_event_that, true); // Conjunction
   Suppose_literal (considering_that, true); // Conjunction
   Suppose_literal (in_spite_of_the_fact_that, true); // Conjunction
   Suppose_literal (the_time_or_occasion_at_or_in_which, true); // Conjunction
   Suppose_literal (what_or_which_time, true); // Pronoun
   Suppose_literal (the_time_in_which_something_is_done_or_comes_about, true); // Pronoun

   Satisfy::Formula when = Adverb. And (at_what_time. And (how_soon). And (in_what_circumstances).
               And (at_or_on_which_and_referring_to_a_time_or_circumstance).
               And (at_or_during_which_time). And (at_a_former_and_usually_less_prosperous_time)).
               Or (Conjunction. And (at_or_during_the_time_that). And (after).
               And (at_any_time_that_and_whenever). And (after_which_and_just_then_and_implying_suddenness).
               And (in_view_of_the_fact_that_and_considering_that). And (although_and_whereas).
               And (just_at_the_moment_that). And (at_any_or_every_time_that). And (in_the_event_that).
               And (considering_that). And (in_spite_of_the_fact_that).
               And (the_time_or_occasion_at_or_in_which)). Or (Pronoun. And (what_or_which_time).
               And (the_time_in_which_something_is_done_or_comes_about));

   Suppose_literal (The_assertion, assertion. Value (/* truth value */));
   Suppose_literal (is_true, true);
   Suppose_literal (the_assertion_holds, assertion. Value (/* truth value */));

   auto The_assertion_is_true_when_the_assertion_holds = The_assertion. And (is_true).
                                                         And (when). And (the_assertion_holds);

   // Output
   std::cout
       << "Socrates is a man:    "
       << (Socrates_is_man. Value (/* truth value */)    ? "true\n" : "false\n")
       << "All men are mortal:   "
       << (All_men_are_mortal. Value (/* truth value */) ? "true\n" : "false\n")
       << "Assertion holds:    "
       << (assertion. Value (/* truth value */)        ? "true\n" : "false\n")
       << "The assertion is true when the assertion holds:    "
       << (The_assertion_is_true_when_the_assertion_holds. Value (/* truth value */) ? "true\n" : "false\n");

   return 0;
}
