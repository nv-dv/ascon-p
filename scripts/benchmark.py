#!/usr/bin/python3

from matplotlib import pyplot as plt
import os
from scipy.io import savemat
import argparse
import subprocess

# print(f"{os.system('echo here')=}")
INIT_COUNT = 200000

def find_design_by_name(lst, name):
    for i in range(len(lst)):
        if name in lst[i]:
            return i
    return -1;
        

class Design(object):
	"""docstring for Design"""
	def __init__(self, name, code_size, cpb, rand_bytes, masking_order=1):
		super(Design, self).__init__()
		self.name = name
		self.code_size = code_size
		self.cycles_per_bit = cpb
		self.randomness_used = rand_bytes
		self.masking_order = masking_order
	def __repr__(self):
		return f":{self.name=}\n:{self.code_size=}\n:{self.cycles_per_bit=}\n:{self.randomness_used=}\n:{self.masking_order=}:"

		

parser = argparse.ArgumentParser(description='Bench test the performance of various implementations of ISAP permutation block.')
parser.add_argument('--arch', choices=['32', '64', 'armv7', 'aarch64'], type=str, default=['64'], nargs=1, help='compile for 32/64 bit; compile for armv7/aarch64;')
parser.add_argument('--maskrange', type=int, default=None, nargs=2, metavar='N', help='masking order range')
parser.add_argument('-v', help='verbose', action='store_true')

nmsp = vars(parser.parse_args())
p = nmsp['arch'][0]
drange = nmsp['maskrange']
verbose = nmsp['v']
design_list = []
per_d = dict()
y1, y2 = [[], []]

if drange:
    for d in range(drange[0], drange[1]+1):
        with open("A:\\home\\dor\\ascon-p\\consts.h", "r") as f:
            s = f.read().split('\n')
        s[0] = f"#define MASKING_ORDER {d}"
        s = '\n'.join(s)
        with open("consts.h", "w") as f:
            f.write(s)
        if p in ['32', '64']:
            os.system(f"wsl g++ -m{p} -O3 -fno-builtin -masm=intel ascon-p/main.cpp ascon-p/RandomBuffer/*.cpp ascon-p/usuba_mask/masked_ascon_ua_vslice.c -o ascon-p/release/main_{p}.o")
        elif p=='aarch64':
            os.system(f"wsl aarch64-linux-gnu-g++ -fno-builtin -O3 -static -Wformat=0 ./ascon-p/arm_main.cpp ./ascon-p/RandomBuffer/*.cpp ./ascon-p/usuba_mask/masked_ascon_ua_vslice.c -o ./ascon-p/release/main_{p}.o")
        elif p=='armv7':
            os.system(f"wsl arm-linux-gnueabihf-g++ -fno-builtin -O3 -static -Wformat=0 ./ascon-p/arm_main.cpp ./ascon-p/RandomBuffer/*.cpp ./ascon-p/usuba_mask/masked_ascon_ua_vslice.c -o ./ascon-p/release/main_{p}.o")

        c_output = subprocess.run(["wsl", f"./ascon-p/release/main_{p}.o", "0", f"{int(INIT_COUNT/d)}"], capture_output=True)
        stdout = c_output.stdout.decode().split('\n')
        i = find_design_by_name(stdout, "C generic masking")
        design_list.append(Design(
                                stdout[i],
                                int(stdout[i+1][10:]),
                                float(stdout[i+2][14:]),
                                int(stdout[i+4][5:]),
                                d
                        ))
        i = find_design_by_name(stdout, "usuba generic masking")
        design_list.append(Design(
                                stdout[i],
                                int(stdout[i+1][10:]),
                                float(stdout[i+2][14:]),
                                int(stdout[i+4][5:]),
                                d
                        ))
        per_d[d] =  design_list
        y1.append(design_list[0].cycles_per_bit)
        y2.append(design_list[1].cycles_per_bit)
        design_list = []
    if verbose:
        print(per_d)
    x = list(range(drange[0], drange[1]+1))
    print("---->")
    print(f"{x=}")
    print(f"{y1=}")
    print(f"{y2=}")
    plt.plot(x, y1, x, y2)
    plt.show()
    savemat(f".\\results\\benchmark_{p}.mat", {"d": x, "gc": y1, "uc": y2})
else:
    if p in ['32', '64']:
        os.system(f"wsl g++ -m{p} -O3 -fno-builtin -masm=intel ascon-p/main.cpp ascon-p/RandomBuffer/*.cpp ascon-p/usuba_mask/masked_ascon_ua_vslice.c -o ascon-p/release/main_{p}.o")
    elif p=='aarch64':
        os.system(f"wsl aarch64-linux-gnu-g++ -fno-builtin -O3 -static -Wformat=0 ./ascon-p/arm_main.cpp ./ascon-p/RandomBuffer/*.cpp ./ascon-p/usuba_mask/masked_ascon_ua_vslice.c -o ./ascon-p/release/main_{p}.o")
    elif p=='armv7':
        os.system(f"wsl arm-linux-gnueabihf-g++ -fno-builtin -O3 -static -Wformat=0 ./ascon-p/arm_main.cpp ./ascon-p/RandomBuffer/*.cpp ./ascon-p/usuba_mask/masked_ascon_ua_vslice.c -o ./ascon-p/release/main_{p}.o")
    c_output = subprocess.run(["wsl", f"./ascon-p/release/main{p}.o"], capture_output=True)
    stdout = c_output.stdout.decode().split('\n')
    i = 0
    d = 1
    while i<len(stdout)-1:
        design_list.append(Design(
                                  stdout[i],
                                  int(stdout[i+1][10:]),
                                  float(stdout[i+2][14:]),
                                  int(stdout[i+4][5:]),
                                  d
                            ))
        i += 5

    for d in design_list:
        print(d)

