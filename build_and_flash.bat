rmdir /S /Q build
mkdir build
cd build
cmake -G Ninja -DCMAKE_TOOLCHAIN_FILE=cmake/gcc-arm-none-eabi.cmake ..
ninja
STM32_Programmer_CLI -c port=SWD sn=37FF71064E5734364E3E1343 -w Project_Nhung.bin 0x08000000 -v -rst