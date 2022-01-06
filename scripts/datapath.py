import os
import subprocess
from matplotlib import pyplot as plt

start = 16<<10
end = 16<<20
step = int((end - start) / 128)
cwd = os.getcwd()
linuxwd = "/mnt/"+cwd.replace('\\', '/').replace("C:", "c")
native_times = []
xmm_times = []
ymm_times = []
zmm_times = []
x = list(range(start, end, step))
for size in range(start, end, step):
    c_output = subprocess.run(["wsl", f"{linuxwd}/scripts/a.out", str(int(size/8))], capture_output=True)
    stdout = c_output.stdout.decode().split('\n')
    i = int(stdout[0][5:])
    j = int(stdout[1][5:])
    k = int(stdout[2][5:])
    try:
        l = int(stdout[3][5:])
        zmm_times.append(l)
    except:
        pass
    native_times.append(i)
    xmm_times.append(j)
    ymm_times.append(k)

plt.plot(x, native_times, x, xmm_times, x, ymm_times)
if zmm_times != []:
    plt.plot(x, zmm_times)
plt.show()

print(list(range(start, end, step)))
print(native_times)
print(xmm_times)
print(ymm_times)
if zmm_times != []:
    print(zmm_times)

"""
zmm_times
[13354, 122236, 271018, 960508, 956210, 611444, 845012, 1004940, 1524290, 1374112, 1604716, 1692744, 2613694, 2418142, 2596934, 2974580, 3164758, 3332504, 3866490, 3565918, 3995988, 6232678, 4690206, 4624788, 6988041, 5260682, 5152080, 12034626, 5811896, 5974204, 6346234, 6423430, 8442584, 7621100, 7712026, 7759584, 8203672, 9972188, 8779824, 8844846, 8612712, 9135012, 9013642, 9352968, 9752328, 9437974, 12039156, 10981758, 11076434, 10834080, 10768960, 12243994, 11780150, 11509692, 10976754, 11966164, 12683738, 12560186, 11586192, 14875606, 15179554, 17348324, 15801726, 16309980, 14516542, 16071336, 14829126, 14925390, 13983876, 15580248, 13819128, 19979846, 16223920, 16402254, 14996482, 16885548, 17967328, 19304086, 18457258, 19058344, 16579148, 15854572, 15728998, 20104326, 18338782, 16802794, 18249340, 19702348, 20807040, 18572108, 17477650, 18500600, 21604698, 18316544, 18233440, 19233066, 21183286, 20901778, 19337184, 19629222, 19847168, 20371812, 20791672, 20828418, 23762256, 20730364, 21091532, 21358579, 21434546, 21683294, 21304806, 22141672, 22882136, 22327834, 22413674, 22622026, 23178284, 22543174, 23312420, 23180282, 24508212, 28770474, 23878968, 24831132, 25284210, 24083386, 26065964, 25169928]
"""