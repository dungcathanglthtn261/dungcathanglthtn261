# Smart Irrigation — STM32F103C8T6

Project bài tập lớn môn Lập trình Hệ thống nhúng: giám sát độ ẩm đất và điều khiển tưới cây tự động.

## Công cụ

- STM32CubeIDE.
- STM32CubeMX qua file `SmartIrrigation.ioc`.
- FreeRTOS, giao diện CMSIS-RTOS v1.

## Trạng thái giai đoạn 1

- Đã tạo bộ khung 5 task FreeRTOS.
- Đã tạo mutex `systemDataMutex`.
- Đã khai báo giao diện các mô-đun trong 8 header.
- Build thành công, không có lỗi hoặc cảnh báo.
- Chưa xác nhận hoạt động trên phần cứng.
- Các driver và thuật toán điều khiển sẽ được triển khai ở giai đoạn tiếp theo.

## Tác vụ

| Task | Thời gian chờ trong bộ khung |
|---|---:|
| Sensor | 1000 ms |
| Button | 20 ms |
| Control | 100 ms |
| Display | 200 ms |
| UART | 1000 ms |

Các task hiện chỉ có vòng lặp và lệnh chờ.

## Kết nối phần cứng

| Chức năng | Chân |
|---|---|
| Cảm biến độ ẩm | PA0 — ADC1_IN0 |
| Relay | PA4 — active-low, khởi tạo High |
| USART2 TX/RX | PA2/PA3 — 115200, 8-N-1 |
| LCD RS/EN | PB0/PB1 |
| LCD D4–D7 | PB10–PB13 |
| MODE/UP/DOWN/PUMP | PB5–PB8 — Input Pull-up |

## Cấu trúc mã nguồn

- `Core/Inc`: header cấu hình ngoại vi và giao diện mô-đun.
- `Core/Src`: mã khởi tạo ngoại vi và các task.
- `Drivers`: thư viện phần cứng.
- `Middlewares`: FreeRTOS và các thành phần liên quan.
- `SmartIrrigation.ioc`: cấu hình CubeMX.

Các header ứng dụng:
`app_types.h`, `app_config.h`, `soil_sensor.h`, `button.h`,
`relay_pump.h`, `lcd16x02.h`, `uart_comm.h`,
`irrigation_control.h`.

## Mở và build

1. Mở STM32CubeIDE.
2. Chọn File → Import → General → Existing Projects into Workspace.
3. Chọn thư mục chứa project và hoàn tất import.
4. Chọn Project → Build Project.

Khi sửa cấu hình CubeMX, giữ mã tự viết trong các vùng USER CODE.

## Lưu ý triển khai

- Chỉ cho phép Auto điều khiển tưới sau khi có dữ liệu cảm biến hợp lệ.
- Hiệu chuẩn lại ADC_DRY và ADC_WET trên mạch thật.
- Kiểm tra lại stack, heap và thời gian thực thi khi bổ sung driver.