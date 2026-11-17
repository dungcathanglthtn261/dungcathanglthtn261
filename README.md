# Smart Irrigation – STM32F103 (Blue Pill)

| Thư mục | Nội dung |
|---|---|
| `firmware/` | Code STM32 + FreeRTOS (CubeMX `SmartIrrigation.ioc`, `Makefile`). Build: `cd firmware && make` → `build/Project.elf` |
| `firmware/*.resc, *.repl` | Mô phỏng Renode. Test: `renode --console --disable-gui -e "include @test_irrigation.resc"` (chạy trong `firmware/`) |
| `simulation/` | `sketch.ino` – bản Arduino để chạy trên Wokwi / Mokxi |
| `hardware/` | KiCad: schematic, PCB, thư viện footprint/symbol |
| `hardware/images/` | Ảnh preview PCB |
| `hardware/routing/` | File Freerouting (.dsn/.ses) và kết quả DRC |
| `backup/` | Bundle git, tarball trước khi ghép code, lịch sử file PCB |
