# Smart Irrigation System – Bài tập lớn LTHTN (HK261)

Thiết bị đo độ ẩm đất và tự động tưới cây, dùng **STM32F103C8 (Blue Pill)** + FreeRTOS.
Đồ án môn **Lập trình Hệ thống nhúng** – HCMUT.

Nhóm: Dũng · Ca · Thắng

![PCB mặt trên](hardware/images/pcb_top.png)

## Chức năng

- Đọc độ ẩm đất qua ADC mỗi 1000 ms, quy đổi 0–100 % (độ phân giải 1 %).
- **AUTO**: bật bơm khi độ ẩm < ngưỡng − 5 %, tắt khi ≥ ngưỡng + 5 % (hysteresis, relay không đóng/ngắt liên tục). Ngưỡng chỉnh bằng UP/DOWN, bước 5 %, 10–90 %.
- Trạng thái **IDLE** (chờ số đo đầu tiên) và **ERROR** (mất cảm biến → tắt bơm, tự phục hồi). Đổi chế độ luôn tắt bơm.
- **MANUAL**: bật/tắt bơm bằng nút PUMP, không phụ thuộc cảm biến.
- LCD 16x2 hiển thị độ ẩm, ngưỡng, chế độ, trạng thái bơm.
- Gửi dữ liệu lên PC qua UART, ví dụ: `SOIL: 45% | SET: 60% | MODE: AUTO | PUMP: ON`. Nhận lệnh 1 ký tự từ PC: `m` = MODE, `p` = PUMP, `+`/`-` = ngưỡng.

## Cấu trúc repo

```
hardware/kicad/      Project KiCad: schematic, PCB 10x10 cm, thư viện symbol/footprint riêng
hardware/images/     Ảnh xem nhanh schematic và PCB
firmware/SmartIrrigation/  Project STM32CubeIDE + FreeRTOS (5 task, Task_Control, driver soil_sensor/lcd16x02/relay_pump), Makefile, mô phỏng Renode (*.repl, *.resc)
hardware/routing/    File Freerouting (.dsn/.ses) và kết quả DRC
docs/bao-cao-mo-phong-renode.md  Báo cáo mô phỏng Renode
simulation/mokxi/    diagram.json (định dạng Wokwi) + sketch.ino để mô phỏng trên Mokxi/Wokwi
```

## Gán chân (Blue Pill)

| Khối | Chân | Ghi chú |
|---|---|---|
| Cảm biến độ ẩm | PA0 (ADC1_IN0) | Cấp 3V3, tụ lọc 100 nF |
| UART → PC | PA2 TX, PA3 RX (USART2) | 115200 8N1, nối chéo TX/RX với USB-UART |
| Relay | PA4 | Module relay 5V, kích mức **thấp** |
| LCD 16x2 (4-bit) | RS PB0, E PB1, D4–D7 PB10–PB13 | RW nối GND, biến trở 10k chỉnh tương phản, 220 Ω đèn nền |
| Nút MODE / UP / DOWN / PUMP | PB5 / PB6 / PB7 / PB8 | Pull-up nội, nhấn = LOW, tụ 100 nF chống dội |
| Nạp / debug | Header SWD trên Blue Pill | PA13/PA14 không có trên 2 hàng chân bên hông |

## Phần cứng (KiCad)

Mở `hardware/kicad/smart_irrigation.kicad_pro` bằng KiCad 9/10. Thư viện symbol và footprint
riêng đã khai báo trong `sym-lib-table` / `fp-lib-table` của project (đường dẫn `${KIPRJMOD}`).

- Board 100 × 100 mm, 2 lớp, mặt dưới phủ đồng GND, 4 lỗ M3.
- Blue Pill cắm qua 2 hàng **header cái 1x20** (hàng cách nhau 15.24 mm), không hàn trực tiếp.
- LCD cắm qua header cái 16 chân; relay module gắn ngoài (J7 điều khiển, J8 tiếp điểm COM/NO).
- Dây tín hiệu 0.5 mm, dây nguồn 1.0 mm, khoảng cách 0.3 mm.
- ERC: 0 lỗi. DRC: 0 lỗi, còn 4 cảnh báo silkscreen của footprint nút nhấn chuẩn KiCad.

**Cần kiểm tra trước khi đặt mạch:** in PCB tỉ lệ 1:1 để ướm Blue Pill thật (pinout, khoảng
cách hàng, chiều USB) và hướng lỗ luồn dây của cầu đấu vít.

## Mô phỏng

Ubuntu không chạy được Proteus, nhóm dùng **Mokxi** (mô phỏng trên trình duyệt):

- `simulation/mokxi/mokxi_project.json`: project định dạng Mokxi (`mokxi-ai/1`), đã qua
  `validate_project` của Mokxi, 0 lỗi. Dùng cảm biến độ ẩm điện dung, máy bơm mini chạy pin 4xAA
  qua relay kích mức thấp, diode chống dòng ngược. Serial Monitor in gói tin UART.
- **Project Mokxi (layout gọn, chạy được):** https://app.mokxi.com/p/RN2hMLpKIqXbyVL_  
  Vị trí/đường dây nằm trong `diagram_mokxi_native.json`; code Mokxi: `sketch_mokxi.ino`.
- `simulation/mokxi/diagram.json` + `sketch.ino`: bản định dạng Wokwi (biến trở thay cảm biến,
  LED thay máy bơm).

Hằng số `ADC_DRY` / `ADC_WET` trong code phải hiệu chuẩn lại trên mạch thật.

### Firmware FreeRTOS trên Renode

Cần: `arm-none-eabi-gcc`, `make`, Renode (đã thử với v1.17). Không cần CubeIDE: `Makefile` build thẳng code CubeMX đã sinh trong repo.

#### 1. Build và mở mô phỏng

```bash
cd firmware/SmartIrrigation
make
renode -e "i @irrigation.resc"
```

- Dùng `-e "i @irrigation.resc"`, **không** dùng `renode irrigation.resc`. Nếu đường dẫn có dấu cách (vd `Lap trinh He thong nhung`), cách sau làm Renode báo `Could not tokenize`.
- Phải chạy lệnh `renode` khi đang đứng trong `firmware/SmartIrrigation/`.
- Mở đúng: dấu nhắc là `(irrigation)` và có cửa sổ **usart2** in mỗi giây một dòng. Dòng đầu là `MODE: IDLE`, sau khoảng 1 s chuyển sang AUTO:
  `SOIL: 41% | SET: 60% | MODE: AUTO | PUMP: ON`
- `make: Nothing to be done for 'all'` không phải lỗi: bản build đã mới nhất.

#### 2. Thả 4 nút (làm một lần, ngay sau khi mở)

Nút dùng pull-up: `true` = nhả, `false` = nhấn. Lúc mở, các chân đang ở mức thấp, firmware coi như cả 4 nút đang bị giữ.

```
sysbus.gpioPortB OnGPIO 5 true
sysbus.gpioPortB OnGPIO 6 true
sysbus.gpioPortB OnGPIO 7 true
sysbus.gpioPortB OnGPIO 8 true
```

#### 3. Đổi độ ẩm đất

```
sysbus WriteDoubleWord 0x400127FC <giá trị ADC>
```

| ADC | Độ ẩm |
|---|---|
| > 3464 | cảm biến lỗi → `ERROR` (hở mạch) |
| 3164 | 0% (khô) |
| 2690 | ~29% |
| 2500 | ~41% (mặc định lúc mở) |
| 2370 | ~50% |
| 2050 | ~70% |
| 1576 | 100% (ướt) |
| < 1276 | cảm biến lỗi → `ERROR` (chập) |

Cảm biến được đọc mỗi 1 s, nên đợi 1–2 s để usart2 cập nhật.

#### 4. Nhấn nút

Một lần nhấn gồm `false` (nhấn) rồi `true` (nhả). Firmware tính một lần nhấn khi chân chuyển từ `true` sang `false`. Lệnh `true` sau đó là nhả nút, cần có thì lần nhấn kế tiếp mới được tính. Ví dụ nhấn MODE:

```
sysbus.gpioPortB OnGPIO 5 false
sysbus.gpioPortB OnGPIO 5 true
```

| Chân | Nút | Tác dụng |
|---|---|---|
| PB5 (`5`) | MODE | Đổi AUTO ↔ MANUAL, tắt bơm lúc đổi |
| PB6 (`6`) | UP | Ngưỡng +5% (tối đa 90%) |
| PB7 (`7`) | DOWN | Ngưỡng −5% (tối thiểu 10%) |
| PB8 (`8`) | PUMP | Bật/tắt bơm, **chỉ có tác dụng ở MANUAL** |

Lệnh phải có tiền tố `sysbus.gpioPortB`. Gõ mỗi `OnGPIO 5 false` sẽ báo `No such command or device`. Phím ↑ gọi lại lệnh vừa gõ, nhớ sửa số chân.

#### 5. Hoạt động của bơm

- **IDLE**: vừa khởi động, chưa có số đo hợp lệ, bơm tắt. Có số đo đầu tiên thì sang AUTO.
- **AUTO**: hysteresis ±5%: bơm ON khi `SOIL < SET − 5`, OFF khi `SOIL ≥ SET + 5`, ở giữa thì giữ nguyên. Nút PUMP bị bỏ qua.
- **ERROR**: đang AUTO mà cảm biến lỗi (ADC ngoài `[1276, 3464]`) thì tắt bơm; cảm biến bình thường lại thì tự về AUTO.
- **MANUAL**: bơm giữ nguyên trạng thái, chỉ đổi khi nhấn PUMP. Độ ẩm không ảnh hưởng tới bơm.
- Xem relay: `sysbus.gpioPortA.relay State` (`True` = bơm chạy). Mỗi lần relay đổi trạng thái, Monitor in `relay: LED state changed to ...`.

#### 6. Kịch bản demo

| # | Thao tác | usart2 |
|---|---|---|
| 1 | ADC `1576` | `SOIL: 100%` … `AUTO` … `PUMP: OFF` |
| 2 | ADC `2690` | `SOIL: 29%` … `PUMP: ON` (29% < 60% − 5) |
| 3 | Nhấn DOWN (7) 4 lần | `SET: 40%` |
| 4 | ADC `2370` | `SOIL: 50%` … `PUMP: OFF` (50% ≥ 40% + 5) |
| 5 | Nhấn MODE (5) hoặc gửi `m` qua UART | `MODE: MANUAL`, bơm OFF |
| 6 | Nhấn PUMP (8) hoặc gửi `p` (`sysbus.usart2 WriteChar 0x70`) | `PUMP: ON` |
| 7 | Nhấn PUMP (8) lần nữa | `PUMP: OFF` |
| 8 | Nhấn MODE (5) | quay lại `AUTO` |

Các lệnh khác: `pause` (tạm dừng), `start` (chạy tiếp, dùng khi usart2 đứng yên), `quit` (thoát).

#### 7. Test tự động

```bash
cd firmware/SmartIrrigation
renode --console --disable-gui -e "include @test_irrigation.resc"
```

Script tự chạy 13 bước T1–T13: IDLE → AUTO, đất khô/ướt, hysteresis ±5% quanh setpoint 60% (50% ON, 60% giữ ON, 70% OFF, 60% giữ OFF), cảm biến lỗi → ERROR → tự về AUTO, nút MODE/PUMP, lệnh UART `m`/`p`. Mỗi bước in dòng `=== Tn ... expect True/False` rồi trạng thái relay (`True` = bơm ON). Chạy lại sau mỗi lần sửa code và `make`.

#### 8. Debug bằng GDB (tuỳ chọn)

`irrigation.resc` mở GDB server ở cổng 3333:

```bash
gdb-multiarch build/Project.elf      # hoặc arm-none-eabi-gdb
(gdb) target remote :3333
```

#### Lưu ý

- Mô phỏng không có LCD, chỉ xem kết quả được qua usart2 và relay.
- `SOIL_ADC_DRY` / `SOIL_ADC_WET` trong `Core/Inc/app_config.h` phải hiệu chuẩn lại trên cảm biến thật.
- Sinh lại code bằng CubeMX: các dòng `osThreadDef` (priority, stack) nằm ngoài `USER CODE`, nên phải giữ đúng trong `SmartIrrigation.ioc`. Có file `.c` mới thì thêm vào `C_SOURCES` trong `Makefile`.

## Việc tiếp theo

- [x] Firmware STM32CubeIDE + FreeRTOS (5 task: SensorRead, ButtonProcess, Control, Display, UART_Tx), test Renode 13/13
- [ ] Xuất Gerber, làm mạch, hàn linh kiện
- [ ] Hiệu chuẩn cảm biến, chạy kịch bản kiểm thử
- [ ] Báo cáo
