#include <algorithm>
#include <chrono>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <random>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#ifdef _WIN32
#include <windows.h>
#endif

using namespace std;
namespace fs = std::filesystem;
using Clock = std::chrono::steady_clock;

// Default Vietnamese; English remains available for coursework demonstrations.
enum class Language { Vietnamese, English };
Language currentLanguage = Language::Vietnamese;
const char* L(const char* english, const char* vietnamese) {
    return currentLanguage == Language::Vietnamese ? vietnamese : english;
}

void setupConsoleUtf8() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
}

string trim(string text) {
    const auto first = text.find_first_not_of(" \t\r\n");
    if (first == string::npos) return "";
    const auto last = text.find_last_not_of(" \t\r\n");
    return text.substr(first, last - first + 1);
}

string upper(string text) {
    transform(text.begin(), text.end(), text.begin(), [](unsigned char c) {
        return static_cast<char>(toupper(c));
    });
    return text;
}

string lower(string text) {
    transform(text.begin(), text.end(), text.begin(), [](unsigned char c) {
        return static_cast<char>(tolower(c));
    });
    return text;
}

// CSV supports commas and doubled quotes inside quoted fields.
vector<string> parseCsv(const string& line) {
    vector<string> fields;
    string field;
    bool quoted = false;
    for (size_t i = 0; i < line.size(); ++i) {
        char c = line[i];
        if (c == '"') {
            if (quoted && i + 1 < line.size() && line[i + 1] == '"') {
                field += '"';
                ++i;
            } else {
                quoted = !quoted;
            }
        } else if (c == ',' && !quoted) {
            fields.push_back(field);
            field.clear();
        } else {
            field += c;
        }
    }
    fields.push_back(field);
    return fields;
}

string csvEscape(const string& value) {
    if (value.find_first_of(",\"\r\n") == string::npos) return value;
    string result = "\"";
    for (char c : value) {
        if (c == '"') result += "\"\"";
        else result += c;
    }
    return result + "\"";
}

class Question {
protected:
    string id_, subject_, prompt_, answer_;
public:
    Question(string id, string subject, string prompt, string answer)
        : id_(std::move(id)), subject_(std::move(subject)),
          prompt_(std::move(prompt)), answer_(std::move(answer)) {}
    virtual ~Question() = default;
    const string& id() const { return id_; }
    const string& subject() const { return subject_; }
    const string& prompt() const { return prompt_; }
    const string& answer() const { return answer_; }
    virtual string type() const = 0;
    virtual void display() const = 0;
    virtual bool check(const string& response) const = 0;
    virtual vector<string> options() const { return {"", "", "", ""}; }
};

class MultipleChoice : public Question {
    vector<string> choices_;
public:
    MultipleChoice(string id, string subject, string prompt, vector<string> choices, string answer)
        : Question(std::move(id), std::move(subject), std::move(prompt), upper(trim(answer))),
          choices_(std::move(choices)) {}
    string type() const override { return "MC"; }
    void display() const override {
        cout << prompt_ << '\n';
        for (size_t i = 0; i < choices_.size(); ++i)
            cout << "  " << char('A' + i) << ") " << choices_[i] << '\n';
    }
    bool check(const string& response) const override { return upper(trim(response)) == answer_; }
    vector<string> options() const override { return choices_; }
};

class TrueFalse : public Question {
public:
    using Question::Question;
    string type() const override { return "TF"; }
    void display() const override { cout << prompt_ << L("\n  (T) True / (F) False\n", "\n  Nhập T (Đúng) hoặc F (Sai).\n"); }
    bool check(const string& response) const override {
        string value = upper(trim(response));
        if (value == "TRUE") value = "T";
        if (value == "FALSE") value = "F";
        return value == upper(trim(answer_));
    }
};

class FillBlank : public Question {
public:
    using Question::Question;
    string type() const override { return "FB"; }
    void display() const override { cout << prompt_ << L("\n  Fill in the blank.\n", "\n  Điền từ hoặc cụm từ còn thiếu.\n"); }
    bool check(const string& response) const override {
        return lower(trim(response)) == lower(trim(answer_));
    }
};

class CodeOutput : public Question {
public:
    using Question::Question;
    string type() const override { return "CO"; }
    void display() const override { cout << prompt_ << L("\n  Enter the exact output (case-sensitive).\n", "\n  Nhập chính xác kết quả in ra (phân biệt hoa/thường).\n"); }
    bool check(const string& response) const override { return trim(response) == trim(answer_); }
};

unique_ptr<Question> makeQuestion(const string& id, const string& subject,
                                   const string& type, const string& prompt,
                                   const vector<string>& choices, const string& answer) {
    if (id.empty() || subject.empty() || prompt.empty() || answer.empty()) return nullptr;
    if (type == "MC") {
        if (choices.size() != 4 || answer.size() != 1 || answer[0] < 'A' || answer[0] > 'D')
            return nullptr;
        return make_unique<MultipleChoice>(id, subject, prompt, choices, answer);
    }
    if (type == "TF") {
        if (answer != "T" && answer != "F") return nullptr;
        return make_unique<TrueFalse>(id, subject, prompt, answer);
    }
    if (type == "FB") return make_unique<FillBlank>(id, subject, prompt, answer);
    if (type == "CO") return make_unique<CodeOutput>(id, subject, prompt, answer);
    return nullptr;
}

class Deck {
    string name_;
    vector<unique_ptr<Question>> questions_;
public:
    explicit Deck(string name) : name_(std::move(name)) {}
    const string& name() const { return name_; }
    size_t size() const { return questions_.size(); }
    void add(unique_ptr<Question> question) { questions_.push_back(std::move(question)); }
    const vector<unique_ptr<Question>>& questions() const { return questions_; }
};

class Library {
    vector<Deck> decks_;
    int nextId_ = 1;
public:
    Deck& getOrCreate(const string& name) {
        for (auto& deck : decks_)
            if (deck.name() == name) return deck;
        decks_.emplace_back(name);
        return decks_.back();
    }
    const vector<Deck>& decks() const { return decks_; }
    size_t questionCount() const {
        size_t total = 0;
        for (const auto& deck : decks_) total += deck.size();
        return total;
    }
    string newId() { return "Q" + to_string(nextId_++); }
    vector<const Question*> select(int deckIndex) const {
        vector<const Question*> result;
        for (size_t i = 0; i < decks_.size(); ++i) {
            if (deckIndex != 0 && static_cast<int>(i) + 1 != deckIndex) continue;
            for (const auto& q : decks_[i].questions()) result.push_back(q.get());
        }
        return result;
    }
    bool load(const string& path) {
        ifstream input(path);
        if (!input) return false;
        decks_.clear();
        nextId_ = 1;
        string line;
        getline(input, line); // CSV header
        int skipped = 0;
        while (getline(input, line)) {
            if (trim(line).empty()) continue;
            auto fields = parseCsv(line);
            if (fields.size() != 10) { ++skipped; continue; }
            vector<string> choices(fields.begin() + 5, fields.begin() + 9);
            auto q = makeQuestion(fields[0], fields[2], fields[3], fields[4], choices, fields[9]);
            if (!q || trim(fields[1]).empty()) { ++skipped; continue; }
            if (fields[0].size() > 1 && fields[0][0] == 'Q') {
                try { nextId_ = max(nextId_, stoi(fields[0].substr(1)) + 1); }
                catch (const exception&) { /* Keep sequential generator. */ }
            }
            getOrCreate(fields[1]).add(std::move(q));
        }
        if (skipped) cerr << L("Warning: skipped ", "Cảnh báo: đã bỏ qua ") << skipped << L(" invalid CSV row(s).\n", " dòng CSV không hợp lệ.\n");
        return true;
    }
    bool save(const string& path) const {
        fs::path file(path);
        if (!file.parent_path().empty()) fs::create_directories(file.parent_path());
        ofstream output(path);
        if (!output) return false;
        output << "id,deck,subject,type,prompt,option_a,option_b,option_c,option_d,answer\n";
        for (const auto& deck : decks_) {
            for (const auto& q : deck.questions()) {
                vector<string> fields = {q->id(), deck.name(), q->subject(), q->type(), q->prompt()};
                auto choices = q->options();
                fields.insert(fields.end(), choices.begin(), choices.end());
                fields.push_back(q->answer());
                for (size_t i = 0; i < fields.size(); ++i) {
                    if (i) output << ',';
                    output << csvEscape(fields[i]);
                }
                output << '\n';
            }
        }
        return static_cast<bool>(output);
    }
};

struct HistoryEntry {
    int attempts = 0;
    int correct = 0;
    double totalSeconds = 0;
};

class History {
    unordered_map<string, HistoryEntry> entries_;
public:
    const HistoryEntry& get(const string& id) const {
        static const HistoryEntry empty;
        auto it = entries_.find(id);
        return it == entries_.end() ? empty : it->second;
    }
    void record(const string& id, bool correct, double seconds) {
        auto& entry = entries_[id];
        ++entry.attempts;
        entry.correct += correct ? 1 : 0;
        entry.totalSeconds += seconds;
    }
    bool load(const string& path) {
        ifstream input(path);
        if (!input) return false;
        entries_.clear();
        string line;
        getline(input, line);
        while (getline(input, line)) {
            auto row = parseCsv(line);
            if (row.size() != 4) continue;
            try {
                HistoryEntry entry{stoi(row[1]), stoi(row[2]), stod(row[3])};
                if (entry.attempts >= 0 && entry.correct >= 0 &&
                    entry.correct <= entry.attempts && entry.totalSeconds >= 0)
                    entries_[row[0]] = entry;
            } catch (const exception&) { /* Skip invalid history row. */ }
        }
        return true;
    }
    bool save(const string& path) const {
        fs::path file(path);
        if (!file.parent_path().empty()) fs::create_directories(file.parent_path());
        ofstream output(path);
        if (!output) return false;
        output << "id,attempts,correct,total_seconds\n";
        output << fixed << setprecision(3);
        for (const auto& pair : entries_)
            output << csvEscape(pair.first) << ',' << pair.second.attempts << ','
                   << pair.second.correct << ',' << pair.second.totalSeconds << '\n';
        return static_cast<bool>(output);
    }
};

struct Attempt {
    string id, subject, expectedAnswer;
    bool correct;
    double seconds;
};

class Exam {
public:
    static void run(vector<const Question*> pool, History& history,
                    int count, int timeLimitSeconds, bool adaptive) {
        if (pool.empty()) { cout << L("No questions available.\n", "Chưa có câu hỏi nào.\n"); return; }
        mt19937 rng(random_device{}());
        shuffle(pool.begin(), pool.end(), rng);
        if (adaptive) {
            stable_sort(pool.begin(), pool.end(), [&history](const Question* a, const Question* b) {
                auto priority = [&history](const Question* q) {
                    const auto& h = history.get(q->id());
                    return h.attempts == 0 ? 0.5 : double(h.attempts - h.correct) / h.attempts;
                };
                return priority(a) > priority(b);
            });
        }
        count = min(count, static_cast<int>(pool.size()));
        if (count <= 0) { cout << L("No questions selected.\n", "Chưa chọn câu hỏi nào.\n"); return; }
        vector<Attempt> attempts;
        auto examStart = Clock::now();
        for (int i = 0; i < count; ++i) {
            auto now = Clock::now();
            double elapsed = chrono::duration<double>(now - examStart).count();
            if (timeLimitSeconds > 0 && elapsed >= timeLimitSeconds) {
                cout << L("\nTime is up!\n", "\nĐã hết thời gian làm bài!\n");
                break;
            }
            const Question* q = pool[i];
            cout << L("\nQuestion ", "\nCâu hỏi ") << i + 1 << "/" << count
                 << L(" | ID ", " | Mã câu ") << q->id() << L(" | Subject: ", " | Chủ đề: ") << q->subject() << '\n';
            if (timeLimitSeconds > 0)
                cout << L("Time remaining: ", "Thời gian còn lại: ") << max(0, int(timeLimitSeconds - elapsed)) << L(" seconds\n", " giây\n");
            q->display();
            cout << L("Your answer: ", "Câu trả lời của bạn: ");
            string response;
            if (!getline(cin, response)) { cout << L("\nInput closed.\n", "\nĐã đóng đầu vào.\n"); break; }
            double seconds = chrono::duration<double>(Clock::now() - now).count();
            bool withinTime = timeLimitSeconds == 0 ||
                chrono::duration<double>(Clock::now() - examStart).count() <= timeLimitSeconds;
            bool correct = withinTime && q->check(response);
            history.record(q->id(), correct, seconds);
            attempts.push_back({q->id(), q->subject(), q->answer(), correct, seconds});
            if (timeLimitSeconds == 0)
                cout << (correct ? L("Correct!", "Chính xác!") : L("Incorrect.", "Chưa chính xác.")) << L(" Answer: ", " Đáp án: ") << q->answer() << '\n';
            else cout << L("Answer recorded.\n", "Đã ghi nhận câu trả lời.\n");
            if (!withinTime) { cout << L("Time is up!\n", "Đã hết thời gian làm bài!\n"); break; }
        }
        int correct = 0;
        double total = 0;
        map<string, pair<int, int>> bySubject;
        cout << L("\n========== RESULTS ==========\n", "\n========== KẾT QUẢ BÀI LÀM ==========\n");
        for (const auto& a : attempts) {
            correct += a.correct ? 1 : 0;
            total += a.seconds;
            auto& subject = bySubject[a.subject];
            ++subject.first;
            subject.second += a.correct ? 1 : 0;
            cout << a.id << ": " << (a.correct ? L("RIGHT", "ĐÚNG") : L("WRONG", "SAI"))
                 << " | " << fixed << setprecision(1) << a.seconds << "s\n";
        }
        int answered = static_cast<int>(attempts.size());
        cout << L("Score: ", "Điểm: ") << correct << "/" << count
             << L(" | Accuracy (answered): ", " | Tỷ lệ đúng (câu đã làm): ") << fixed << setprecision(1)
             << (answered ? 100.0 * correct / answered : 0.0) << "%"
             << L(" | Overall score: ", " | Điểm toàn bài: ") << 100.0 * correct / count << "%\n";
        cout << L("Average time per answered question: ", "Thời gian trung bình mỗi câu đã làm: ")
             << (answered ? total / answered : 0.0) << "s\n";
        if (timeLimitSeconds > 0) {
            cout << L("Answer review (incorrect questions):\n", "Đáp án các câu trả lời sai:\n");
            for (const auto& a : attempts)
                if (!a.correct) cout << a.id << " -> " << a.expectedAnswer << '\n';
        }
        for (const auto& s : bySubject)
            cout << L("Subject ", "Chủ đề ") << s.first << ": " << s.second.second
                 << "/" << s.second.first << L(" correct\n", " câu đúng\n");
        cout << L("=============================\n", "=================================\n");
    }
};

string readLine(const string& prompt) {
    cout << prompt;
    string value;
    getline(cin, value);
    return trim(value);
}

int readInt(const string& prompt, int fallback, int minValue, int maxValue) {
    string value = readLine(prompt);
    if (value.empty()) return fallback;
    try {
        size_t pos = 0;
        int number = stoi(value, &pos);
        if (pos == value.size() && number >= minValue && number <= maxValue) return number;
    } catch (const exception&) {}
    cout << L("Invalid input; using ", "Giá trị không hợp lệ, dùng mặc định: ") << fallback << ".\n";
    return fallback;
}

int chooseDeck(const Library& library) {
    if (library.decks().empty()) return 0;
    cout << L("\n0) All decks\n", "\n0) Tất cả bộ câu hỏi\n");
    for (size_t i = 0; i < library.decks().size(); ++i)
        cout << i + 1 << ") " << library.decks()[i].name()
             << " (" << library.decks()[i].size() << L(" questions)\n", " câu hỏi)\n");
    return readInt(L("Choose deck [0]: ", "Chọn bộ câu hỏi [0]: "), 0, 0, static_cast<int>(library.decks().size()));
}

void addQuestion(Library& library) {
    string deck = readLine(L("Deck name (new or existing): ", "Tên bộ câu hỏi (mới hoặc có sẵn): "));
    string subject = readLine(L("Subject: ", "Chủ đề: "));
    string prompt = readLine(L("Question: ", "Nội dung câu hỏi: "));
    string type = upper(readLine(L("Type (MC/TF/FB/CO): ", "Loại (MC: Trắc nghiệm / TF: Đúng-Sai / FB: Điền từ / CO: Kết quả code): ")));
    if (deck.empty() || subject.empty() || prompt.empty()) {
        cout << L("Deck, subject and question are required.\n", "Vui lòng nhập đủ tên bộ câu hỏi, chủ đề và nội dung câu hỏi.\n");
        return;
    }
    vector<string> choices(4);
    if (type == "MC") {
        for (size_t i = 0; i < choices.size(); ++i)
            choices[i] = readLine(string(L("Option ", "Đáp án lựa chọn ")) + char('A' + i) + ": ");
    }
    string answer = readLine(L("Correct answer: ", "Đáp án đúng: "));
    if (type == "MC" || type == "TF") answer = upper(answer);
    auto q = makeQuestion(library.newId(), subject, type, prompt, choices, answer);
    if (!q) { cout << L("Invalid question type or answer.\n", "Loại câu hỏi hoặc đáp án không hợp lệ.\n"); return; }
    library.getOrCreate(deck).add(std::move(q));
    cout << L("Question added. Remember to save.\n", "Đã thêm câu hỏi. Nhớ lưu dữ liệu nhé!\n");
}

void showQuestions(const Library& library) {
    const auto questions = library.select(chooseDeck(library));
    if (questions.empty()) { cout << L("No questions yet.\n", "Chưa có câu hỏi nào.\n"); return; }
    for (const auto* q : questions)
        cout << q->id() << " [" << q->type() << "] " << q->subject()
             << " - " << q->prompt() << '\n';
}

void showAnalytics(const Library& library, const History& history) {
    int attempts = 0, correct = 0;
    double seconds = 0;
    map<string, pair<int, int>> subjects;
    for (const auto* q : library.select(0)) {
        const auto& h = history.get(q->id());
        attempts += h.attempts;
        correct += h.correct;
        seconds += h.totalSeconds;
        auto& stat = subjects[q->subject()];
        stat.first += h.attempts;
        stat.second += h.correct;
    }
    cout << L("\n===== LEARNING ANALYTICS =====\n", "\n===== THỐNG KÊ HỌC TẬP =====\n");
    cout << L("Attempts: ", "Số lượt trả lời: ") << attempts << L(" | Correct: ", " | Đúng: ") << correct
         << L(" | Accuracy: ", " | Tỷ lệ đúng: ") << fixed << setprecision(1)
         << (attempts ? 100.0 * correct / attempts : 0.0) << "%\n";
    cout << L("Average time per attempt: ", "Thời gian trung bình mỗi lượt: ") << (attempts ? seconds / attempts : 0.0) << "s\n";
    string weakest;
    double weakestRate = 2.0;
    for (const auto& subject : subjects) {
        int count = subject.second.first;
        if (!count) continue;
        double rate = double(subject.second.second) / count;
        cout << subject.first << ": " << subject.second.second << "/" << count
             << " (" << 100.0 * rate << "%)\n";
        if (rate < weakestRate) { weakestRate = rate; weakest = subject.first; }
    }
    if (!weakest.empty()) cout << L("Weakest subject: ", "Chủ đề cần ôn tập nhất: ") << weakest << '\n';
    else cout << L("Take a quiz to collect learning statistics.\n", "Hãy làm thử một bài để có số liệu thống kê.\n");
}

bool selfTest() {
    const string raw = "one,\"two, words\",\"say \"\"hi\"\"\"";
    auto parsed = parseCsv(raw);
    if (parsed.size() != 3 || parsed[1] != "two, words" || parsed[2] != "say \"hi\"")
        return false;
    vector<string> choices = {"1", "2", "3", "4"};
    auto mc = makeQuestion("Q1", "Test", "MC", "Pick B", choices, "B");
    auto tf = makeQuestion("Q2", "Test", "TF", "True?", choices, "T");
    auto fb = makeQuestion("Q3", "Test", "FB", "Word?", choices, "Hello");
    auto co = makeQuestion("Q4", "Test", "CO", "Output?", choices, "Hi");
    if (!mc || !tf || !fb || !co || !mc->check("b") || !tf->check("true") ||
        !fb->check(" hello ") || !co->check("Hi") || co->check("hi")) return false;
    const string filename = "flashcard_self_test.csv";
    Library lib;
    lib.getOrCreate("Test Deck").add(std::move(mc));
    if (!lib.save(filename)) return false;
    Library reloaded;
    bool ok = reloaded.load(filename) && reloaded.questionCount() == 1 &&
              reloaded.select(0)[0]->check("B");
    fs::remove(filename);
    return ok;
}

int main(int argc, char* argv[]) {
    if (argc > 1 && string(argv[1]) == "--self-test") {
        bool ok = selfTest();
        cout << (ok ? "SELF-TEST PASS\n" : "SELF-TEST FAIL\n");
        return ok ? 0 : 1;
    }
    setupConsoleUtf8();
    cout << "\n===== CHỌN NGÔN NGỮ / CHOOSE LANGUAGE =====\n"
         << "1) Tiếng Việt (mặc định)\n2) English\n";
    string languageSelection;
    cout << "Chọn ngôn ngữ / Choose language [1]: ";
    if (!getline(cin, languageSelection)) return 0;
    currentLanguage = trim(languageSelection) == "2" ? Language::English : Language::Vietnamese;
    const string bankFile = currentLanguage == Language::Vietnamese
        ? "data/questions_vi.csv" : "data/questions.csv";
    const string historyFile = currentLanguage == Language::Vietnamese
        ? "data/history_vi.csv" : "data/history.csv";
    Library library;
    History history;
    if (!library.load(bankFile))
        cout << L("No question bank found. Start by adding a question.\n", "Không tìm thấy ngân hàng câu hỏi. Hãy thêm câu hỏi mới.\n");
    history.load(historyFile);
    cout << L("Interactive Flashcard & Examination Engine (C++17)\n", "HỆ THỐNG FLASHCARD VÀ THI THỬ - C++17\n");
    while (true) {
        cout << L("\n1) View questions / decks\n2) Add question / deck\n", "\n1) Xem danh sách câu hỏi / bộ đề\n2) Thêm câu hỏi / bộ đề\n")
             << L("3) Flashcard practice\n4) Timed examination\n", "3) Luyện tập Flashcard\n4) Thi thử có giới hạn thời gian\n")
             << L("5) Adaptive practice (weak questions first)\n", "5) Ôn tập câu hỏi thường trả lời sai\n")
             << L("6) Learning analytics\n7) Save data\n0) Save and exit\n", "6) Xem thống kê kết quả\n7) Lưu dữ liệu\n0) Lưu và thoát\n");
        int option = readInt(L("Choose: ", "Nhập lựa chọn: "), -1, 0, 7);
        if (option == 0 || !cin) {
            bool bankOk = library.save(bankFile);
            bool historyOk = history.save(historyFile);
            cout << (bankOk && historyOk ? L("Data saved. Goodbye!\n", "Đã lưu dữ liệu. Hẹn gặp lại!\n") : L("Error saving files!\n", "Lỗi: không thể lưu dữ liệu!\n"));
            return bankOk && historyOk ? 0 : 1;
        }
        if (option == 1) showQuestions(library);
        else if (option == 2) addQuestion(library);
        else if (option == 6) showAnalytics(library, history);
        else if (option == 7) {
            cout << (library.save(bankFile) && history.save(historyFile) ? L("Saved!\n", "Đã lưu thành công!\n") : L("Save error!\n", "Lưu dữ liệu thất bại!\n"));
        } else if (option == 3 || option == 4 || option == 5) {
            auto questions = library.select(chooseDeck(library));
            int count = readInt(L("Number of questions [5]: ", "Số câu hỏi muốn làm [5]: "), 5, 1, 10000);
            int seconds = option == 4 ? readInt(L("Exam time limit in seconds [120]: ", "Thời gian thi (giây) [120]: "), 120, 1, 86400) : 0;
            Exam::run(questions, history, count, seconds, option == 5);
            if (!history.save(historyFile)) cout << L("Warning: could not save history.\n", "Cảnh báo: không thể lưu lịch sử ôn tập.\n");
        } else cout << L("Choose a number from 0 to 7.\n", "Hãy chọn số từ 0 đến 7.\n");
    }
}
