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

		
cwd = os.getcwd()
linuxwd = cwd
parser = argparse.ArgumentParser(description='Bench test the performance of various implementations of ISAP permutation block.')
parser.add_argument('--arch', choices=['x86', 'x64', 'armv7', 'aarch64'], type=str, default=['x64'], nargs=1, help='compile for 32/64 bit; compile for armv7/aarch64;')
parser.add_argument('--maskrange', type=int, default=None, nargs=2, metavar='N', help='masking order range')
parser.add_argument('-v', help='verbose', action='store_true')

nmsp = vars(parser.parse_args())
p = nmsp['arch'][0]
drange = nmsp['maskrange']
verbose = nmsp['v']
design_list = []
per_d = dict()
y1, y2 = [[], []]
r1, r2 =[[], []]
uma_str = ""

if drange:
    for d in range(drange[0], drange[1]+1):
        if verbose:
            print(f"d = {d}")
        with open(cwd+"/globals.h", "r") as f:
            s = f.read().split('\n')
            if "#define REFRESH_ISW" in s:
                refresh_str = "_REFRESH_ISW_20"
            if "#define REFRESH_HPC" in s:
                refresh_str = "_REFRESH_HPC_20"
        s[0] = f"#define MASKING_ORDER {d}"
        s = '\n'.join(s)
        with open(cwd+"/globals.h", "w") as f:
            f.write(s)
        if p in ['x86', 'x64']:
            os.system(f"make")
        elif p=='aarch64':
            os.system(f"make aarch64")
        elif p=='armv7':
            os.system(f"make armv7")
        c_output = subprocess.run(f"{linuxwd}/bin_{p}/main_{p}", capture_output=True)
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
        r1.append(design_list[0].randomness_used)
        r2.append(design_list[1].randomness_used)
        design_list = []
    if verbose:
        print(per_d)
    x = list(range(drange[0], drange[1]+1))
    print("---->")
    print(f"{x=}")
    print(f"{y1=}")
    print(f"{y2=}")
    plt.subplot(2,1,1)
    plt.title(f"Cycles per bit {p}")
    plt.plot(x, y1, x, y2)
    plt.subplot(2,1,2)
    plt.title(f"Randomness used {p}")
    plt.plot(x, r1, x, r2)
    plt.show()
    savemat(f"{cwd}/results/benchmark_{p}{uma_str}.mat", {"d": x, "gc": y1, "uc": y2, "gr": r1, "ur": r2})
else:
    if p in ['x86', 'x64']:
        os.system(f"make")
    elif p=='aarch64':
        os.system(f"make aarch64")
    elif p=='armv7':
        os.system(f"make armv7")
    c_output = subprocess.run(f"{linuxwd}/bin_{p}/main_{p}", capture_output=True)
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

