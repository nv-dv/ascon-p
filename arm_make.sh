echo "gcc-ARMv7"

arm-linux-gnueabihf-g++ -fno-builtin -O3 -static -Wformat=0 arm_main.cpp RandomBuffer/*.cpp usuba_mask/masked_ascon_ua_vslice.c -o ./release/main_armv7.o

echo "gcc-aarch64"

aarch64-linux-gnu-g++ -fno-builtin -O3 -static -Wformat=0 arm_main.cpp RandomBuffer/*.cpp usuba_mask/masked_ascon_ua_vslice.c -o ./release/main_aarch64.o

echo "ARMv7"

./release/main_armv7.o

echo "aarch64"

./release/main_aarch64.o
