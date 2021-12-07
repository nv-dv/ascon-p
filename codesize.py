#!/usr/bin/python3

import argparse
import subprocess
import os

verbose=False
usuba_funcs = {"isw_mult":0, "Sbox__V64":0, "AddConstant__V64":0, "LinearLayer__V64":0, "ascon12":0}
gen_funcs = {"dS_AND":0, "dS_ROUND":0 }

parser = argparse.ArgumentParser(description='Codesize of various implementations of ISAP permutation block.')
parser.add_argument('--bits', choices=['32', '64'], default=['64'], nargs=1, help='compile for 32/64 bit')
parser.add_argument('--maskrange', type=int, default=[2, 2], nargs=2, metavar='N', help='masking order range')

nmsp = vars(parser.parse_args())
p = nmsp['bits'][0]
drange = nmsp['maskrange']

print(f"{p} bit...")

y1, y2 = [[], []]

for d in range(drange[0], drange[1]+1):
    with open("consts.h", "r") as f:
        s = f.read().split('\n')
    s[0] = f"#define MASKING_ORDER {d}"
    s = '\n'.join(s)
    with open("consts.h", "w") as f:
        f.write(s)
    os.system(f"g++ -m{p} -O0 -fno-builtin -masm=intel main.cpp RandomBuffer/*.cpp usuba_mask/masked_ascon_ua_vslice.c -o release/main{p}.o")
    
    c_output = subprocess.run(["nm", "-S", f"./release/main{p}.o"], capture_output=True)
    stdout = c_output.stdout.decode().split('\n')
    for line in stdout:
        func = line.split(" ")[-1]
        for sig in usuba_funcs.keys():
            if func.find(sig) != -1:
                size = int(line.split(" ")[-3], 16)
                usuba_funcs[sig] = size
                if verbose:
                    print(f"{sig}: {size}")
        for sig in gen_funcs.keys():
            if func.find(sig) != -1:
                size = int(line.split(" ")[-3], 16)
                gen_funcs[sig] = size
                if verbose:
                    print(f"{sig}: {size}")

    y1.append(gen_funcs["dS_ROUND"]+12*5*gen_funcs["dS_AND"])
    if verbose:
        print(f"generic C codesize(d={d}): ", y1[-1])
    y2.append(usuba_funcs["ascon12"]+12*(usuba_funcs["AddConstant__V64"]+usuba_funcs["Sbox__V64"]+usuba_funcs["LinearLayer__V64"]+5*usuba_funcs["isw_mult"]))
    if verbose:
        print(f"usuba codesize(d={d}): ", y2[-1])

x = range(drange[0], drange[1]+1)
print(x, y1, y2)