## ascon-p
see https://www.overleaf.com/project/61941338e7e09ab75ea86d0c for details
# building & runnung
setup.sh - perform required installations on the machine.
make.sh - to build for x86 based processors, and run.
arm_make.sh - to build for ARM based processors, and emulate the run.
# scripts
inside scripts, there are:
benchmark.py - benchmark cycles per bit, randomness usage, for both our generic masking and usuba.
codesize.py - calculate the codesize for every implemntation.
datapath.py - simulate the time required for each technology to copy a large buffer.
