load("datapath_64.mat")

plot(x, native_times, x, xmm_times, x, ymm_times, x, zmm_times)
title("Copy Time vs. Array Size")
xlabel("size[Byte]")
ylabel("CPU Cycles")
legend("Native RF (rax)", "XMM RF (xmm0)", "YMM RF (ymm0)", "ZMM RF (zmm0)", "Location", 'northeast')
