# Smart Irrigation – STM32F103 (Blue Pill)

| Thư mục | Nội dung |
|---|---|
| `firmware/SmartIrrigation/Core/Src/` | Phần của Dũng: `lcd16x02.c`, `soil_sensor.c`, `relay_pump.c`, `irrigation_control.c`, thân các task trong `freertos.c` |
| `firmware/SmartIrrigation/*.resc, *.repl, Makefile` | Build bằng `make` và mô phỏng trên Renode (xem phần bên dưới) |
| `simulation/` | `sketch.ino` – bản Arduino để chạy trên Wokwi / Mokxi |
| `hardware/` | KiCad: schematic, PCB, thư viện footprint/symbol |
| `hardware/images/` | Ảnh preview PCB |
| `hardware/routing/` | File Freerouting (.dsn/.ses) và kết quả DRC |
| `docs/block-diagram/` | Sơ đồ khối hệ thống (draw.io, PNG, Visio) |
| `docs/flowcharts/` | Lưu đồ thuật toán `main` và 5 task (Mermaid `.mmd` + PNG), khớp với code firmware |
| `docs/` | [Báo cáo mô phỏng Renode](docs/bao-cao-mo-phong-renode.md): mô hình phần cứng, kịch bản kiểm thử, log kết quả |

## Phân chia với nhánh `Ca`

Nhánh này **chỉ chứa phần của Dũng**. Khung CubeMX (`main.c`, `gpio`, `adc`, `usart`, `.mxproject`…), HAL, FreeRTOS, các header trong `Core/Inc`, `button.c` (chống dội), `uart_comm.c` là của Ca, nằm ở nhánh [`Ca`](../../tree/Ca), cùng đường dẫn `firmware/SmartIrrigation/`.

- `make ca` tải phần của Ca từ `origin/Ca` về `.ca/` (không commit).
- Khi build, file ở nhánh này được ưu tiên hơn file cùng tên của Ca: `freertos.c`, `SmartIrrigation.ioc` (stack `displayTask` 384 word cho `snprintf`).
- Sau khi gộp hai nhánh, `Makefile` tự dùng code tại chỗ, không cần `.ca/` nữa.

## Mô phỏng FreeRTOS trên Renode

Cần: `arm-none-eabi-gcc`, `make`, Renode (đã thử với v1.17). Không cần CubeMX: code sinh sẵn lấy từ nhánh `Ca` bằng `make ca`.

### 1. Build và mở mô phỏng

```bash
cd firmware/SmartIrrigation
make ca        # lần đầu, hoặc khi Ca cập nhật nhánh
make
renode -e "i @irrigation.resc"
```

- Dùng `-e "i @irrigation.resc"`, **không** dùng `renode irrigation.resc`. Đường dẫn có dấu cách (`Lap trinh He thong nhung`) làm Renode báo `Could not tokenize`.
- Phải chạy lệnh `renode` khi đang đứng trong `firmware/SmartIrrigation/`.
- `make` báo `Chua co phan cua Ca. Chay: make ca` thì chạy `make ca` trước.
- Mở đúng: dấu nhắc là `(irrigation)` và có cửa sổ **usart2** in mỗi giây một dòng:
  `SOIL: 41% | SET: 60% | MODE: AUTO | PUMP: ON | VALID: 1`
- `make: Nothing to be done for 'all'` không phải lỗi: bản build đã mới nhất.

### 2. Thả 4 nút (làm một lần, ngay sau khi mở)

Nút dùng pull-up: `true` = nhả, `false` = nhấn. Lúc mở, các chân đang ở mức thấp, firmware coi như cả 4 nút đang bị giữ.

```
sysbus.gpioPortB OnGPIO 5 true
sysbus.gpioPortB OnGPIO 6 true
sysbus.gpioPortB OnGPIO 7 true
sysbus.gpioPortB OnGPIO 8 true
```

### 3. Đổi độ ẩm đất

```
sysbus WriteDoubleWord 0x400127FC <giá trị ADC>
```

| ADC | Độ ẩm |
|---|---|
| ≥ 3164 | 0% (khô) |
| 2690 | ~29% |
| 2500 | ~41% (mặc định lúc mở) |
| 2370 | ~50% |
| 2050 | ~70% |
| ≤ 1576 | 100% (ướt) |

Cảm biến được đọc mỗi 1 s, nên đợi 1–2 s để usart2 cập nhật.

### 4. Nhấn nút

Một lần nhấn gồm `false` (nhấn) rồi `true` (nhả). Firmware tính một lần nhấn khi chân chuyển từ `true` sang `false`. Lệnh `true` sau đó là nhả nút, cần có thì lần nhấn kế tiếp mới được tính. Ví dụ nhấn MODE:

```
sysbus.gpioPortB OnGPIO 5 false
sysbus.gpioPortB OnGPIO 5 true
```

| Chân | Nút | Tác dụng |
|---|---|---|
| PB5 (`5`) | MODE | Đổi AUTO ↔ MANUAL, bơm giữ nguyên trạng thái lúc đổi |
| PB6 (`6`) | UP | Ngưỡng +5% (tối đa 90%) |
| PB7 (`7`) | DOWN | Ngưỡng −5% (tối thiểu 10%) |
| PB8 (`8`) | PUMP | Bật/tắt bơm, **chỉ có tác dụng ở MANUAL** |

Lệnh phải có tiền tố `sysbus.gpioPortB`. Gõ mỗi `OnGPIO 5 false` sẽ báo `No such command or device`. Phím ↑ gọi lại lệnh vừa gõ, nhớ sửa số chân.

### 5. Hoạt động của bơm

- **AUTO**: bơm ON khi `SOIL < SET`, OFF khi `SOIL ≥ SET`. Nút PUMP bị bỏ qua.
- **MANUAL**: bơm giữ nguyên trạng thái, chỉ đổi khi nhấn PUMP. Độ ẩm không ảnh hưởng tới bơm.
- Xem relay: `sysbus.gpioPortA.relay State` (`True` = bơm chạy). Mỗi lần relay đổi trạng thái, Monitor in `relay: LED state changed to ...`.

### 6. Kịch bản demo

| # | Thao tác | usart2 |
|---|---|---|
| 1 | ADC `1576` | `SOIL: 100%` … `AUTO` … `PUMP: OFF` |
| 2 | ADC `2690` | `SOIL: 29%` … `PUMP: ON` (29% < 60%) |
| 3 | Nhấn DOWN (7) 4 lần | `SET: 40%` |
| 4 | ADC `2370` | `SOIL: 50%` … `PUMP: OFF` (50% ≥ 40%) |
| 5 | Nhấn MODE (5) | `MODE: MANUAL`, bơm vẫn OFF |
| 6 | Nhấn PUMP (8) | `PUMP: ON` |
| 7 | Nhấn PUMP (8) lần nữa | `PUMP: OFF` |
| 8 | Nhấn MODE (5) | quay lại `AUTO` |

Các lệnh khác: `pause` (tạm dừng), `start` (chạy tiếp, dùng khi usart2 đứng yên), `quit` (thoát).

### 7. Test tự động

```bash
cd firmware/SmartIrrigation
renode --console --disable-gui -e "include @test_irrigation.resc"
```

Script tự chạy 3 tình huống (đất khô → bơm ON, đất ướt → bơm OFF, MODE + PUMP → MANUAL/ON) và in trạng thái relay. Chạy lại sau mỗi lần sửa code và `make`.

### 8. Debug bằng GDB (tuỳ chọn)

`irrigation.resc` mở GDB server ở cổng 3333:

```bash
gdb-multiarch build/Project.elf      # hoặc arm-none-eabi-gdb
(gdb) target remote :3333
```

### Lưu ý

- Mô phỏng không có LCD, chỉ xem kết quả được qua usart2 và relay.
- `SOIL_ADC_DRY` / `SOIL_ADC_WET` trong `Core/Inc/app_config.h` (file của Ca) phải hiệu chuẩn lại trên cảm biến thật.
- Khi Ca đổi cấu hình CubeMX và đẩy lên nhánh `Ca`, chạy lại `make ca`. Nếu có file `.c` mới thì thêm vào `C_SOURCES` trong `Makefile`. Nếu Ca sinh lại code, đối chiếu `SmartIrrigation.ioc` và `freertos.c` của nhánh này với bản mới.
