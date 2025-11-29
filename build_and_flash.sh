#!/bin/bash
# ============================================
# STM32F103C8T6 Build & Flash Script (OpenOCD)
# Author : Nguyễn Duy Tuyên
# ============================================

set -e  # stop on any error

# --- 1. Clean and rebuild ---
rm -rf build
mkdir build
cd build

cmake -G Ninja -DCMAKE_TOOLCHAIN_FILE=../cmake/gcc-arm-none-eabi.cmake ..
ninja

# --- 2. Flash firmware with OpenOCD ---
# For STM32F103C8T6 (Flash base 0x08000000, 64 KB Flash)
echo "🔌 Connecting to target and flashing firmware..."
openocd -f interface/stlink.cfg \
        -f target/stm32f1x.cfg \
        -c "program Project_Nhung.bin verify reset exit 0x08000000"

echo "✅ Done! Firmware successfully flashed to STM32F103C8T6."
