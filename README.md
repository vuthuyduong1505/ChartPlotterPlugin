# ChartPlotterPlugin

**ChartPlotterPlugin** là một module/plugin vẽ biểu đồ hiệu năng cao, được phát triển dựa trên sự kết hợp giữa **C++ Core Engine** (sử dụng OpenGL thông qua `QQuickFramebufferObject`) và **Qt Quick (QML)**. Dự án được tối ưu hóa đặc biệt để xử lý và trực quan hóa các bộ dữ liệu lớn (từ hàng chục nghìn đến hàng triệu điểm) với tốc độ khung hình (FPS) mượt mà.

## Tính Năng Nổi Bật

- **Hiệu Năng Vượt Trội:** Duy trì ổn định 60 FPS ngay cả khi hiển thị hàng triệu điểm dữ liệu nhờ áp dụng thuật toán giảm mẫu thông minh **LTTB (Largest-Triangle-Three-Buckets)**.
- **Đa Dạng Loại Biểu Đồ:** Hỗ trợ biểu đồ Đường (Line), Cột (Bar), và Tròn (Pie) với thiết kế linh hoạt dựa trên **Strategy & Factory Pattern**.
- **Tương Tác Thời Gian Thực:** Hỗ trợ tính năng nạp luồng dữ liệu thời gian thực (Online Stream) với khả năng trượt tự động (Auto-pan).
- **Trải Nghiệm UI/UX Tối Ưu:**
  - Tích hợp Zoom (cuộn chuột) & Pan (kéo thả).
  - Tính năng Hit-Testing tốc độ cao kết hợp Crosshair và Tooltip thông minh.
  - Giao diện thiết kế theo phong cách Modern Dark Mode với các thành phần QML được module hóa.
- **Kiến Trúc An Toàn Luồng:** Hệ thống quản lý dữ liệu an toàn (Thread-safety) chống xung đột giữa GUI Thread và Render Thread.

## Yêu Cầu Hệ Thống

- **Ngôn ngữ:** C++17 hoặc C++20
- **Framework:** Qt 6.1+ (yêu cầu các module `Qt6::Gui`, `Qt6::Quick`, `Qt6::OpenGL`)
- **Trình biên dịch:** CMake 3.16 trở lên

## Kiến Trúc Hệ Thống (Layered Architecture)

1. **Tầng Dữ Liệu & Xử Lý Core:** `DataManager`, `DataProcessor` (LTTB Downsample), `FileLoader`, `OnlineStream`.
2. **Tầng Cầu Nối C++ & QML:** `ChartItem` (kế thừa `QQuickFramebufferObject`), `ChartRenderer`, `ViewportManager`, `ChartHitTester`.
3. **Tầng Đồ Họa Strategy Pattern:** `ChartStrategyFactory`, `LineChartStrategy`, `BarChartStrategy`, `PieChartStrategy`.
4. **Tầng Giao Diện QML:** `Main.qml` và các components (`TopNavBar`, `FooterStatusBar`, `CrosshairOverlay`, ...).

## Hướng Dẫn Cài Đặt & Khởi Chạy

Dự án sử dụng CMake để quản lý build, tạo ra thư viện động (`ChartPlotterPlugin`) và một ứng dụng demo (`appQmlTest`).

### Build bằng dòng lệnh

```bash
# 1. Tạo thư mục build
mkdir build
cd build

# 2. Cấu hình CMake
cmake ..

# 3. Biên dịch dự án
cmake --build .
```

### Chạy ứng dụng

Sau khi biên dịch thành công, bạn có thể chạy file thực thi:
- **Windows:** `build\Debug\appQmlTest.exe` hoặc `build\Release\appQmlTest.exe`
- **Linux/macOS:** `./build/appQmlTest`

---
*Dự án được xây dựng với mục tiêu tối ưu hiệu năng hiển thị và kiến trúc phần mềm sạch.*
