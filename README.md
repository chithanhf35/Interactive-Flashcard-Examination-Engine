> **Tiếng Việt:** Xem [Hướng dẫn sử dụng bằng tiếng Việt](README_VI.md). Khi chạy ứng dụng, ngôn ngữ mặc định là **Tiếng Việt** (nhấn Enter để chọn); có thể nhập **2** để sử dụng tiếng Anh.

# Interactive Flashcard & Examination Engine

**Course project - C++17 (Topic 5).** A console-based flashcard and examination system with question banks, timed quizzes, adaptive practice, learning analytics and CSV persistence.

## Features and assignment mapping

| Requirement | Implementation |
|---|---|
| Polymorphic question types | Abstract `Question`, derived `MultipleChoice` (`MC`), `TrueFalse` (`TF`), `FillBlank` (`FB`), `CodeOutput` (`CO`) |
| Deck & subject management | `Deck` and `Library` group questions by deck, and store a separate subject for each question |
| Timed exam | Select number of questions and exam duration; show remaining time; score responses received within the limit |
| Adaptive practice | Prioritize questions with higher historical error rate; unseen questions receive a neutral priority |
| Result breakdown and analytics | Scores, percentages, time per answer, average time, subject accuracy and weakest practiced subject |
| File persistence | Read/write `data/questions.csv` and `data/history.csv` |

## Run on Windows (VS Code + G++)

Open the **repository root folder** in VS Code and use the terminal:

```powershell
g++ -std=c++17 -Wall -Wextra -pedantic src/main.cpp -o flashcard.exe
.\flashcard.exe
```

If using Linux/macOS:

```bash
g++ -std=c++17 -Wall -Wextra -pedantic src/main.cpp -o flashcard
./flashcard
```

CMake alternative:

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

When using CMake, launch the compiled executable **from the repository root** so `data/questions.csv` is found. For example, on Linux: `./build/flashcard`; on Windows with MinGW: `.\build\flashcard.exe`.

Run built-in regression checks with `./flashcard --self-test` (Windows: `.\flashcard.exe --self-test`).

## Menu

1. View decks and questions.
2. Add a deck (by name) and a question.
3. Practice flashcards without a timer.
4. Take a timed examination (answers reviewed at the end).
5. Adaptive practice prioritizing previously incorrect questions.
6. Review learning analytics.
7. Save questions and history.
0. Save and exit.

## Data format

`data/questions.csv` uses:

`id,deck,subject,type,prompt,option_a,option_b,option_c,option_d,answer`

- `MC`: four choices; `answer` is `A`, `B`, `C`, or `D`.
- `TF`: `answer` is `T` or `F`.
- `FB`: case-insensitive comparison, ignoring leading/trailing whitespace.
- `CO`: case-sensitive answer, ignoring leading/trailing whitespace.

The CSV parser handles quoted commas and doubled quotes. **Each question must occupy one physical line; multi-line CSV fields are not supported in V1.**

`data/history.csv` is created automatically when you take a quiz. It stores `id,attempts,correct,total_seconds` and is ignored by Git to avoid committing personal study history.

## Structure

```text
.
├── CMakeLists.txt
├── README.md
├── .gitignore
├── src/
│   └── main.cpp
├── data/
│   └── questions.csv
└── docs/
    └── REPORT_OUTLINE.md
```

The first version deliberately keeps its classes together in `main.cpp` for easier course-level study and debugging. They can be separated into `.hpp`/`.cpp` files later without changing the algorithms.

## Known limitations

- The timed console mode checks the deadline **before each question and after Enter is pressed**. Portable blocking `std::getline` cannot interrupt input automatically at the exact deadline; the system simulates a timed exam rather than implementing a live countdown thread.
- Analytics are per local history file, not multi-user, and results are not exported separately.
- CSV data files are intended to be edited by the program; avoid simultaneous writers.

## Testing

Verified with **G++ 14 / CMake 3.31 on Linux**: compilation succeeds and the built-in CSV, polymorphism, answer-checking and data round-trip test passes. Manual CLI smoke tests covered deck listing, timed quiz and analytics.
