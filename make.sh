
echo "gcc-x86"
g++ -m32 -o0 -fno-builtin -masm=intel main.cpp RandomBuffer/*.cpp usuba_mask/masked_ascon_ua_vslice.c -o release/main32.o
echo "gcc-x86_64"
g++ -o0 -fno-builtin -masm=intel main.cpp RandomBuffer/*.cpp usuba_mask/masked_ascon_ua_vslice.c -o release/main64.o

echo "32 bit"
./release/main32.o
echo "64 bit"
./release/main64.o