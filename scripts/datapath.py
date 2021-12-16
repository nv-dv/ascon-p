
import subprocess
from matplotlib import pyplot as plt

start = 16<<10
end = 16<<20
step = int((end - start) / 128)

native_times = []
xmm_times = []
ymm_times = []
x = list(range(start, end, step))
for size in range(start, end, step):
    c_output = subprocess.run(["wsl", "/home/dor/ascon-p/scripts/a.out", str(int(size/8))], capture_output=True)
    stdout = c_output.stdout.decode().split('\n')
    i = int(stdout[0][5:])
    j = int(stdout[1][5:])
    k = int(stdout[2][5:])
    native_times.append(i)
    xmm_times.append(j)
    ymm_times.append(k)

plt.plot(x, native_times, x, xmm_times, x, ymm_times)
plt.show()

print(list(range(start, end, step)))
print(native_times)
print(xmm_times)
print(ymm_times)
