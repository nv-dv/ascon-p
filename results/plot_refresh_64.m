load("benchmark_64.mat")
ogc = gc; ogr = gr;
load("benchmark_64_REFRESH_ISW.mat")
isw_gc = gc; isw_gr = gr;
load("benchmark_64_REFRESH_HPC.mat")
hpb_gc = gc; hpb_gr = gr;
load("benchmark_64_REFRESH_ISW20.mat")
isw20_gc = gc; isw20_gr = gr;
load("benchmark_64_REFRESH_HPC20.mat")
hpb20_gc = gc; hpb20_gr = gr;

subplot(1,2,1);
plot(d, ogc, d, isw20_gc, d, isw_gc, d, hpb20_gc, d, hpb_gc);
title("Ascon-p Cycles / bit vs. masking order (x86-64)")
xlabel("masking order")
ylabel("Cycles / bit")
legend("proposed (C)", "proposed (C) with 20% ISW REFRESH", "proposed (C) with ISW REFRESH", "proposed (C) with 20% HPC REFRESH", "proposed (C) with HPC REFRESH", "Location", 'northwest')

subplot(1,2,2);
plot(d, ogr, d, isw20_gr, d, isw_gr, d, hpb20_gr, d, hpb_gr);
title("Ascon-p Randomness usage vs. masking order (x86-64)")
xlabel("masking order")
ylabel("Usage [Byte]")
legend("proposed (C)", "proposed (C) with 20% ISW REFRESH", "proposed (C) with ISW REFRESH", "proposed (C) with 20% HPC REFRESH", "proposed (C) with HPC REFRESH", "Location", 'northwest')
