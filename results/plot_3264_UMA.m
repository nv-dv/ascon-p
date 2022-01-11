
load("benchmark_64.mat")
ogc = gc; ogr = gr; ouc = uc;
load("benchmark_64_UMA.mat")

subplot(3,2,1)
plot(d, gc, d, ogc, d, ouc)
title("Ascon-p Cycles / bit vs. masking order (x86-64)")
xlabel("masking order")
ylabel("Cycles / bit")
legend("proposed (generic C) with UMA", "proposed (generic C)", "usuba compiled", "Location", 'northwest')

subplot(3,2,3)
plot(d, gr, d, ogr, d, ur)
title("Ascon-p Randomness usage vs. masking order (x86-64)")
xlabel("masking order")
ylabel("Random Bytes[byte]")
legend("proposed (generic C)with UMA", "proposed (generic C)", "usuba compiled", "Location", 'northwest')

load("codesize_64.mat")
ogc = gc; ouc = uc;
load("codesize_64_UMA.mat")
subplot(3,2,5)
plot(d, gc, d, ogc, d, ouc)
title("Ascon-p Codesize vs. masking order (x86-64)")
xlabel("masking order")
ylabel("Codesize [bytes]")
legend("proposed (generic C) with UMA", "proposed (generic C)", "usuba compiled", "Location", 'northwest')


load("benchmark_32.mat")
ogc = gc; ogr = gr;

load("benchmark_32_UMA.mat")

subplot(3,2,2)
plot(d, gc, d, ogc, d, uc)
title("Ascon-p Cycles / bit vs. masking order (x86)")
xlabel("masking order")
ylabel("Cycles / bit")
legend("proposed (generic C) with UMA", "proposed (generic C)", "usuba compiled", "Location", 'northwest')

subplot(3,2,4)
plot(d, gr, d, ogr, d, ur)
title("Ascon-p Randomness usage vs. masking order (x86)")
xlabel("masking order")
ylabel("Random Bytes[byte]")
legend("proposed (generic C)with UMA", "proposed (generic C)", "usuba compiled", "Location", 'northwest')

load("codesize_32.mat")
ogc = gc;
load("codesize_32_UMA.mat")
subplot(3,2,6)
plot(d, gc, d, ogc, d, uc)
title("Ascon-p Codesize vs. masking order (x86)")
xlabel("masking order")
ylabel("Codesize [bytes]")
legend("proposed (generic C) with UMA", "proposed (generic C)", "usuba compiled", "Location", 'northwest')


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%