#!/usr/bin/python3

import os
import sys
import argparse
import subprocess

# print(f"{os.system('echo here')=}")

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
parser.add_argument('--bits', choices=['32', '64'], type=str, default=['64'], nargs=1, help='compile for 32/64 bit')
parser.add_argument('--maskrange', type=int, default=None, nargs=2, metavar='N', help='masking order range')

nmsp = vars(parser.parse_args())
p = nmsp['bits'][0]
drange = nmsp['maskrange']
design_list = []
per_d = dict()
y1, y2 = [[], []]

if drange:
    for d in range(drange[0], drange[1]+1):
        with open("consts.h", "r") as f:
            s = f.read().split('\n')
        s[0] = f"#define MASKING_ORDER {d}"
        s = '\n'.join(s)
        with open("consts.h", "w") as f:
            f.write(s)
        os.system(f"g++ -m{p} -O0 -fno-builtin -masm=intel main.cpp RandomBuffer/*.cpp usuba_mask/masked_ascon_ua_vslice.c -o release/main{p}.o")
        c_output = subprocess.run([f"./release/main{p}.o", "1"], capture_output=True)
        stdout = c_output.stdout.decode().split('\n')
        i = 0
        while i<len(stdout)-1:
            design_list.append(Design(
                                      stdout[i],
                                      int(stdout[i+1][10:]),
                                      float(stdout[i+2][14:]),
                                      int(stdout[i+4][5:]),
                                      d
                                ))
            i += 5
        per_d[d] =  design_list
        y1.append(design_list[0].cycles_per_bit)
        y2.append(design_list[1].cycles_per_bit)
        design_list = []
    # print(per_d)
    x = range(drange[0], drange[1]+1)
    print(x, y1, y2)
else:
    os.system(f"g++ -m{p} -O0 -fno-builtin -masm=intel main.cpp RandomBuffer/*.cpp usuba_mask/masked_ascon_ua_vslice.c -o release/main{p}.o")
    c_output = subprocess.run([f"./release/main{p}.o"], capture_output=True)
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

