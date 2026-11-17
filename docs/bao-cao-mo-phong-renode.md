# Báo cáo mô phỏng firmware trên Renode

**Đề tài:** Hệ thống tưới cây tự động (Smart Irrigation), STM32F103C8 (Blue Pill) + FreeRTOS
**Môn:** Lập trình Hệ thống nhúng, HK261
**Ngày chạy mô phỏng:** 17/11/2026

> **Lưu ý:** kết quả trong báo cáo này đo trên commit `2958e75` của nhánh `Dung`. Từ commit `df55ced`, nhánh `Dung` không còn `lcd16x02.c`, `soil_sensor.c`, `relay_pump.c`, nên muốn chạy lại phải có lại 3 file này (xem README).

---

## 1. Mục tiêu

Kiểm tra firmware **đúng bản nạp cho mạch thật** (`build/Project.elf`) trước khi có phần cứng:

- Cả 5 task FreeRTOS chạy đúng chu kỳ, chia sẻ dữ liệu qua mutex không lỗi.
- Đọc cảm biến độ ẩm qua ADC và quy đổi đúng ra phần trăm.
- Logic điều khiển bơm ở hai chế độ AUTO và MANUAL đúng đặc tả.
- Nút nhấn (MODE, UP, DOWN, PUMP) được nhận và xử lý đúng.
- Gói tin UART gửi lên PC đúng định dạng, đúng chu kỳ.

Renode chạy chính file ELF biên dịch cho Cortex-M3, nên khác với mô phỏng Arduino trên Wokwi/Mokxi: code HAL, FreeRTOS, ngắt và bộ lập lịch đều được thực thi thật.

## 2. Môi trường

| Thành phần | Phiên bản |
|---|---|
| Hệ điều hành | Ubuntu 26.04.1 LTS |
| Trình mô phỏng | Renode v1.17.0 |
| Trình biên dịch | arm-none-eabi-gcc 14.2.1 |
| Firmware | STM32CubeMX (HAL F1) + FreeRTOS (CMSIS-RTOS v1), build bằng `Makefile`. Khung CubeMX, `button`, `uart_comm` lấy từ nhánh `Ca`; `lcd16x02`, `soil_sensor`, `relay_pump`, thân task từ nhánh `Dung` (commit `2958e75`) |

Kích thước firmware:

| | Dùng | Dung lượng chip | Tỉ lệ |
|---|---|---|---|
| Flash (text + data) | 22 028 B | 64 KB | 34 % |
| RAM (data + bss) | 11 816 B | 20 KB | 58 % |

## 3. Mô hình phần cứng trong Renode

Nền tảng gốc là `platforms/cpus/stm32f103.repl` có sẵn trong Renode. Bản này thiếu hoặc mô phỏng chưa đúng một số ngoại vi mà HAL F1 cần, nên nhóm bổ sung trong `firmware/SmartIrrigation/irrigation.repl`:

| Ngoại vi | Vấn đề với nền tảng gốc | Cách nhóm xử lý |
|---|---|---|
| Bit-band (`0x4200 0000`) | Không có. HAL bật HSI/PLL qua vùng alias này | Thêm `Miscellaneous.BitBanding` |
| RCC | Cờ READY và `CFGR.SWS` không phản ánh PLL, nên `HAL_RCC_ClockConfig` bị timeout | Peripheral Python: cờ READY theo bit ON, SWS = SW. Firmware chạy được cấu hình HSE 8 MHz × PLL 9 = 72 MHz |
| FLASH_ACR | HAL ghi LATENCY rồi đọc lại để kiểm tra | Peripheral Python lưu lại giá trị đã ghi |
| ADC1 | Không có. `STM32_ADC` của Renode là bản F4 (khác bit SWSTART, không có hiệu chuẩn) | Peripheral Python mô phỏng ADC F1: cờ EOC luôn sẵn sàng, bit CAL/SWSTART tự xoá, thanh ghi DR trả về giá trị "cảm biến" |
| DWT | Không có | Thêm `Miscellaneous.DWT`, 72 MHz |
| Relay (PA4) | – | `Miscellaneous.LED` gắn vào PA4, `invert: true` vì relay kích mức thấp |
| Nút nhấn (PB5–PB8) | – | Điều khiển trực tiếp chân GPIO từ Monitor |
| USART2 (PA2/PA3) | Có sẵn | Mở cửa sổ analyzer để xem gói tin |

**Đưa giá trị cảm biến vào:** mô hình ADC có thêm một thanh ghi phụ ở offset `0x3FC` (địa chỉ `0x400127FC`). Ghi vào đây giá trị 0–4095 thì đó là kết quả ADC firmware đọc được:

```
sysbus WriteDoubleWord 0x400127FC 2500
```

**CPU:** `PerformanceInMips 72` để tick 1 ms của FreeRTOS khớp với thời gian thực ở 72 MHz.

**Không mô phỏng:** LCD 16x2 (chỉ ghi GPIO, không có màn hình để xem).

## 4. Firmware được kiểm tra

| Task | Chu kỳ | Stack (word) | Nhiệm vụ |
|---|---|---|---|
| `sensorTask` | 1000 ms | 128 | Đọc ADC 8 mẫu lấy trung bình, quy đổi ra %, ghi vào `g_system_data` |
| `buttonTask` | 20 ms | 128 | Quét 4 nút, chống dội 3 mẫu (60 ms), đếm số lần nhấn |
| `controlTask` | 100 ms | 128 | Xử lý nút, tính trạng thái bơm, điều khiển relay |
| `displayTask` | 200 ms | 384 | Hiển thị lên LCD |
| `uartTask` | 1000 ms | 384 | Gửi gói tin qua USART2 (truyền bằng ngắt) |

Dữ liệu dùng chung `g_system_data` được bảo vệ bằng mutex `systemDataMutex`.

Quy đổi độ ẩm (`soil_sensor.c`), phần thập phân bị cắt bỏ, kết quả kẹp trong 0–100 %:

```
SOIL(%) = (raw − 3164) × 100 / (1576 − 3164)
```

Luật điều khiển (`irrigation_control.c`):

- **AUTO:** bơm ON khi `SOIL < SET` và cảm biến hợp lệ, ngược lại OFF. Nút PUMP bị bỏ qua.
- **MANUAL:** nút PUMP bật/tắt bơm, độ ẩm không ảnh hưởng.
- UP/DOWN đổi ngưỡng ±5 %, giới hạn 10–90 %. Ngưỡng mặc định 60 %.

Định dạng gói tin UART (115200 8N1):

```
SOIL: <độ ẩm>% | SET: <ngưỡng>% | MODE: AUTO|MANUAL | PUMP: ON|OFF | VALID: 0|1
```

## 5. Kịch bản kiểm thử và kết quả

Lúc bắt đầu, cả 4 nút được đặt ở mức cao (nhả). Một lần nhấn là kéo chân xuống thấp 100 ms rồi trả lên cao.

| # | Thao tác | Kỳ vọng | Kết quả usart2 | Đạt |
|---|---|---|---|---|
| 1 | Khởi động, ADC mặc định 2500 | Gói tin đầu `VALID: 0`, sau lần đọc đầu: 41 %, AUTO, 41 < 60 nên bơm ON | `SOIL: 41% \| SET: 60% \| MODE: AUTO \| PUMP: ON \| VALID: 1` | ✅ |
| 2 | ADC = 1576 (đất ướt) | 100 %, bơm OFF, relay tắt | `SOIL: 100% \| SET: 60% \| MODE: AUTO \| PUMP: OFF` | ✅ |
| 3 | ADC = 2690 (đất khô) | 29 %, bơm ON, relay bật | `SOIL: 29% \| SET: 60% \| MODE: AUTO \| PUMP: ON` | ✅ |
| 4 | Nhấn DOWN 4 lần | Ngưỡng 60 → 40 % | `SOIL: 29% \| SET: 40% \| MODE: AUTO \| PUMP: ON` | ✅ |
| 5 | ADC = 2370 | 50 % ≥ 40 % nên bơm OFF | `SOIL: 50% \| SET: 40% \| MODE: AUTO \| PUMP: OFF` | ✅ |
| 6 | Nhấn MODE | Sang MANUAL, bơm giữ nguyên OFF | `SOIL: 50% \| SET: 40% \| MODE: MANUAL \| PUMP: OFF` | ✅ |
| 7 | Nhấn PUMP | Bơm ON bằng tay dù đất đủ ẩm | `SOIL: 50% \| SET: 40% \| MODE: MANUAL \| PUMP: ON` | ✅ |
| 8 | Nhấn PUMP lần nữa | Bơm OFF | `SOIL: 50% \| SET: 40% \| MODE: MANUAL \| PUMP: OFF` | ✅ |
| 9 | Nhấn MODE | Quay về AUTO, 50 % ≥ 40 % nên OFF | `SOIL: 50% \| SET: 40% \| MODE: AUTO \| PUMP: OFF` | ✅ |
| 10 | ADC = 3300 (khô hơn mức khô chuẩn) | Kẹp về 0 %, bơm ON | `SOIL: 0% \| SET: 40% \| MODE: AUTO \| PUMP: ON` | ✅ |

**Kết quả: 10/10 trường hợp đạt.** Ở mọi bước, trạng thái relay PA4 (log `relay -> True/False`) khớp với trường `PUMP` trong gói tin.

### Log usart2 (thời gian là thời gian ảo của mô phỏng)

```
[0.33ms] SOIL: 0% | SET: 60% | MODE: AUTO | PUMP: OFF | VALID: 0
    relay -> True
[1s]  SOIL: 41% | SET: 60% | MODE: AUTO | PUMP: ON | VALID: 1
>>> ADC 1576
    relay -> False
[4s]  SOIL: 100% | SET: 60% | MODE: AUTO | PUMP: OFF | VALID: 1
>>> ADC 2690
    relay -> True
[6s]  SOIL: 29% | SET: 60% | MODE: AUTO | PUMP: ON | VALID: 1
>>> DOWN x4
[7s]  SOIL: 29% | SET: 40% | MODE: AUTO | PUMP: ON | VALID: 1
>>> ADC 2370
    relay -> False
[10s] SOIL: 50% | SET: 40% | MODE: AUTO | PUMP: OFF | VALID: 1
>>> MODE
[11s] SOIL: 50% | SET: 40% | MODE: MANUAL | PUMP: OFF | VALID: 1
>>> PUMP
    relay -> True
[12s] SOIL: 50% | SET: 40% | MODE: MANUAL | PUMP: ON | VALID: 1
>>> PUMP
    relay -> False
[13s] SOIL: 50% | SET: 40% | MODE: MANUAL | PUMP: OFF | VALID: 1
>>> MODE
[14s] SOIL: 50% | SET: 40% | MODE: AUTO | PUMP: OFF | VALID: 1
>>> ADC 3300
    relay -> True
[17s] SOIL: 0% | SET: 40% | MODE: AUTO | PUMP: ON | VALID: 1
```

Log đã lược bỏ các dòng lặp lại giữa hai lần thao tác. Gói tin cách nhau đúng 1 s thời gian ảo, đúng chu kỳ của `uartTask`.

### Kiểm thử tự động

Script `firmware/SmartIrrigation/test_irrigation.resc` chạy không cần giao diện, dùng để kiểm tra lại sau mỗi lần sửa code:

```bash
cd firmware/SmartIrrigation
renode --console --disable-gui -e "include @test_irrigation.resc"
```

| Bước | Kiểm tra | Kết quả |
|---|---|---|
| Đất khô (ADC 2500) | relay = `True`, `soil_percent` = `0x29` (41) | ✅ |
| Đất ướt (ADC 1300) | relay = `False`, `soil_percent` = `0x64` (100) | ✅ |
| Nhấn MODE rồi PUMP | `MODE: MANUAL \| PUMP: ON`, relay = `True` | ✅ |

## 6. Cách chạy lại

Xem hướng dẫn từng bước trong [README.md](../README.md#mô-phỏng-freertos-trên-renode). Tóm tắt:

Cần có đủ `lcd16x02.c`, `soil_sensor.c`, `relay_pump.c` trong `Core/Src/` (xem lưu ý đầu báo cáo).

```bash
cd firmware/SmartIrrigation
make ca
make
renode -e "i @irrigation.resc"
```

Đường dẫn thư mục có dấu cách nên phải dùng `-e "i @..."` thay cho `renode irrigation.resc`.

## 7. Nhận xét và hạn chế

**Đã kiểm chứng được:**

- Khởi tạo clock 72 MHz (HSE + PLL), HAL, FreeRTOS và cả 5 task đều chạy trên đúng file ELF dùng để nạp mạch.
- Mutex bảo vệ dữ liệu chung hoạt động: không treo, không có gói tin sai lệch.
- Logic AUTO/MANUAL, giới hạn ngưỡng, kẹp giá trị 0–100 % đều đúng đặc tả.
- UART truyền bằng ngắt in đều mỗi giây.

**Hạn chế:**

- **LCD không được mô phỏng.** Nội dung hiển thị phải kiểm tra trên mạch thật hoặc bản Mokxi.
- **ADC lý tưởng:** giá trị không có nhiễu, nên bộ lọc trung bình 8 mẫu chưa được kiểm tra với tín hiệu thực.
- **Nút nhấn lý tưởng:** cạnh xung sạch, không có dội cơ khí. Bộ chống dội 3 mẫu chỉ được kiểm tra ở mức logic.
- **Các mô hình RCC, FLASH_ACR, ADC do nhóm tự viết**, chỉ mô phỏng đủ hành vi HAL cần, không giống chip thật đến từng bit.
- **Thời gian ảo không trùng thời gian thực:** Renode chạy theo `PerformanceInMips`, nên chu kỳ task chỉ đúng tương đối.
- **Hằng số hiệu chuẩn** `SOIL_ADC_DRY = 3164`, `SOIL_ADC_WET = 1576` (`app_config.h`) là giá trị mô phỏng. Phải đo lại trên cảm biến điện dung thật.

**Việc tiếp theo:** nạp firmware lên Blue Pill, hiệu chuẩn cảm biến, kiểm tra LCD và relay thật theo cùng kịch bản ở mục 5.
