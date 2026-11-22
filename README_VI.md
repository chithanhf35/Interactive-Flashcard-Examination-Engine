# Hướng dẫn sử dụng — Flashcard & Thi thử C++ (Tiếng Việt)

## 1. Chương trình này làm gì?

Đây là project C++17 cho **Topic 5: Interactive Flashcard & Examination Engine**, với các chức năng:

1. Xem danh sách câu hỏi và bộ đề.
2. Thêm bộ đề/câu hỏi: trắc nghiệm (MC), đúng–sai (TF), điền từ (FB), kết quả chương trình (CO).
3. Luyện tập flashcard không giới hạn thời gian.
4. Thi thử có giới hạn thời gian.
5. Ôn lại câu thường trả lời sai.
6. Xem thống kê số câu đúng, tỷ lệ đúng và thời gian làm bài.
7. Lưu dữ liệu vào file CSV.

**Chương trình mặc định mở bằng tiếng Việt**, còn tiếng Anh chỉ là tùy chọn.

## 2. Chuẩn bị trên Windows

Cần cài đặt VS Code và trình biên dịch **G++ hỗ trợ C++17** (ví dụ MinGW-w64/MSYS2). Kiểm tra bằng:

```powershell
g++ --version
```

Mở thư mục gốc repository trong VS Code. Trong **Terminal → New Terminal**, chạy:

```powershell
g++ -std=c++17 -Wall -Wextra -pedantic src/main.cpp -o flashcard.exe
.\flashcard.exe
```

**Quan trọng:** Hãy chạy ở thư mục chứa các thư mục `src` và `data`, không chạy trong riêng thư mục `src`. Nếu dấu tiếng Việt lỗi trong terminal Windows cũ, chạy `chcp 65001`, sau đó mở lại chương trình. Nên dùng Windows Terminal hoặc terminal của VS Code với font hỗ trợ tiếng Việt. Lưu file mã nguồn ở định dạng UTF-8.

## 3. Chọn tiếng Việt

Khi xuất hiện:

```text
===== CHỌN NGÔN NGỮ / CHOOSE LANGUAGE =====
1) Tiếng Việt (mặc định)
2) English
Chọn ngôn ngữ / Choose language [1]:
```

**Nhấn Enter** (hoặc nhập `1`) để dùng **Tiếng Việt**. Nhập `2` nếu muốn chuyển sang **English**.

Chọn ngôn ngữ ở đầu mỗi lần chạy. Các file dữ liệu của hai ngôn ngữ độc lập nên không đè lên câu hỏi hoặc lịch sử của nhau.

## 4. Menu tiếng Việt

```text
1) Xem danh sách câu hỏi / bộ đề
2) Thêm câu hỏi / bộ đề
3) Luyện tập Flashcard
4) Thi thử có giới hạn thời gian
5) Ôn tập câu hỏi thường trả lời sai
6) Xem thống kê kết quả
7) Lưu dữ liệu
0) Lưu và thoát
Nhập lựa chọn:
```

Ví dụ muốn thi thử: chọn `4`, chọn bộ câu hỏi, nhập số câu và thời gian (giây). Trả lời câu hỏi theo hướng dẫn:

- **MC**: nhập `A`, `B`, `C` hoặc `D`.
- **TF**: nhập `T` nếu đúng, `F` nếu sai.
- **FB**: điền câu trả lời; không phân biệt chữ hoa/thường với câu trả lời dùng ký tự Latin.
- **CO**: nhập chính xác kết quả chương trình xuất ra, có phân biệt hoa/thường.

## 5. Dữ liệu lưu ở đâu?

| Ngôn ngữ | Ngân hàng câu hỏi | Lịch sử kết quả |
|---|---|---|
| Tiếng Việt | `data/questions_vi.csv` | `data/history_vi.csv` |
| English | `data/questions.csv` | `data/history.csv` |

File lịch sử sẽ được tạo khi làm bài và không đưa lên GitHub.

## 6. Kiểm tra source code

Chạy bộ kiểm tra nhanh:

```powershell
.\flashcard.exe --self-test
```

Nếu thấy `SELF-TEST PASS` thì kiểm tra cơ bản đã đạt.

Hoặc chạy bằng CMake:

```powershell
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

## 7. Lưu ý cho bài báo cáo

Bài sử dụng lập trình hướng đối tượng: lớp trừu tượng `Question`, kế thừa và đa hình với bốn lớp câu hỏi; `Deck` quản lý bộ đề; `Library` đọc/ghi CSV; `Exam` chấm bài; `History` lưu tiến độ. Khi trình bày, có thể giới thiệu chức năng trước, sau đó giải thích kế thừa, hàm ảo và cách đọc/ghi file.

**Giới hạn V1:** chế độ thi tính giờ chỉ kiểm tra hạn chót trước khi hiện câu hỏi và sau khi người dùng nhấn Enter. Nó chưa thể tự ngắt lệnh nhập `getline` đang chờ đúng thời điểm hết giờ.
