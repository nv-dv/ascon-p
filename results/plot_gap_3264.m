load("opt_masking_64.mat")

load("benchmark_64.mat")

subplot(2,2,1)
plot(d, gc, d, uc, 2, xmm_cpb, 'dm', 4, ymm_cpb, 'dm', 8, zmm_cpb, 'dm', 1, 0.301477, 'dk')

title("Ascon-p Cycles / bit vs. masking order (x86-64)")
xlabel("masking order")
ylabel("Cycles / bit")
legend("proposed (generic C)", "usuba compiled", "Optimized assembly", "Location", 'northwest')

load("codesize_64.mat")
subplot(2, 2, 3)
plot(d, gc, d, uc, 2, xmm_code, 'dm', 4, ymm_code, 'dm', 8, zmm_code, 'dm', 1, 0x12b, 'dk')
title("Ascon-p Codesize vs. masking order (x86-64)")
xlabel("masking order")
ylabel("Codesize [bytes]")
legend("proposed (generic C)", "usuba compiled", "Optimized assembly", "Location", 'northwest')


load("opt_masking_32.mat")

load("benchmark_32.mat")

subplot(2,2,2)
plot(d, gc, d, uc, 2, xmm_cpb, 'dm', 4, ymm_cpb, 'dm', 1, 1.093541, 'dk')
title("Ascon-p Cycles / bit vs. masking order (x86)")
xlabel("masking order")
ylabel("Cycles / bit")
legend("proposed (generic C)", "usuba compiled", "Optimized assembly", "Location", 'northwest')

load("codesize_32.mat")
subplot(2, 2, 4)
plot(d, gc, d, uc, 2, xmm_code, 'dm', 4, ymm_code, 'dm', 1, 0x24c, 'dk')
title("Ascon-p Codesize vs. masking order (x86)")
xlabel("masking order")
ylabel("Codesize [bytes]")
legend("proposed (generic C)", "usuba compiled", "Optimized assembly", "Location", 'northwest')
