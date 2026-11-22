# Report outline - Topic 5 (C++ course project)

## 1. Introduction
- Problem: support interactive flashcard learning and simulated examinations.
- Objectives and scope based on the supplied Topic 5 assignment.

## 2. Requirements analysis
- Four question types: multiple choice, true/false, fill in the blank, code output.
- Deck and subject organization.
- Timed exam simulation, adaptive practice, analytics, CSV persistence.

## 3. Design
- `Question` is an abstract base class with pure virtual `type`, `display`, `check`.
- `MultipleChoice`, `TrueFalse`, `FillBlank`, `CodeOutput` override behavior.
- `Deck` owns `unique_ptr<Question>` objects.
- `Library` loads and saves decks; `Exam` conducts tests; `History` tracks attempts.

Simple relationships:

```text
                      Question (abstract)
                        /   |   |    \
                      MC   TF   FB    CO
                        ^   ^   ^    ^
                        | polymorphism
Deck 1..* --- Question 1..*   Library --- Deck 1..*
                Exam -------- Question
                History ----- Question ID
```

## 4. Algorithm descriptions
- Shuffle candidate questions for practice/exam.
- Adaptive rank: `(attempts - correct) / attempts`; use `0.5` for unseen questions.
- Record response correctness and time for every completed attempt.
- Save CSV by escaping commas and double quotes; reload on startup.

## 5. Test plan
1. Build with G++ and C++17.
2. Run `--self-test` and CTest.
3. Try all four types (correct and incorrect answers).
4. Create a new deck, save and reopen.
5. Run a timed quiz and inspect end-of-exam feedback.
6. Repeat wrong questions and observe adaptive ordering.
7. Inspect learning analytics after multiple attempts.

## 6. Limitations and possible improvements
- Time limit is checked around blocking console input, not asynchronously.
- No login, network, database or GUI in this course version.
- Future: split sources by class, richer test suite, multi-line question import, result export, graphical UI.

## 7. Conclusion
Explain how C++ inheritance, polymorphism, STL, file I/O and time measurement meet the assignment.
