#!/usr/bin/python3
""" docstring for module
"""
import subprocess
import sys
import os


def main():
	# key / plaintext list
	keys = [b'1'*16] * 10
	plaintexts = [b'2'*16] * 10
	# add -D GPIO to gcc to trigger
	os.system("g++ -masm=intel -Wformat=0 -O3 main.cpp generic_isap.cpp RandomBuffer/RandomBuffer.cpp")
	proc = subprocess.Popen("./a.out", stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
	for i in range(10):
		proc.stdin.write(plaintexts[i])
		proc.stdin.write(keys[i])
		proc.stdin.flush()
		# handle trigger measurement here...
	proc.stdin.close()
	ciphers = proc.stdout.readlines()
	print(ciphers)
	return 0


if __name__ == '__main__':
	sys.exit(main())
