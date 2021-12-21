
echo "gcc-x86"
g++ -m32 -O2 -fno-builtin -masm=intel main.cpp RandomBuffer/*.cpp usuba_mask/masked_ascon_ua_vslice.c -o release/main32.o
echo "gcc-x86_64"
g++ -m64 -O2 -fno-builtin -masm=intel main.cpp RandomBuffer/*.cpp usuba_mask/masked_ascon_ua_vslice.c -o release/main64.o

echo "32 bit"
./release/main32.o
$(python3 -c "import time;time.sleep(1)")
echo "64 bit"
./release/main64.o