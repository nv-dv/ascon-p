
load("benchmark64.mat")

subplot(2,2,1)
plot(d, gc, d, uc)
title("Ascon-p Cycles / bit vs. masking order (x86-64)")
xlabel("masking order")
ylabel("Cycles / bit")
legend("proposed (generic C)", "usuba compiled", "Location", 'southeast')


load("codesize64.mat")
subplot(2,2,2)
plot(d, gc, d, uc)
title("Ascon-p Codesize vs. masking order (x86-64)")
xlabel("masking order")
ylabel("Codesize [bytes]")
legend("proposed (generic C)", "usuba compiled", "Location", 'east')


load("benchmark32.mat")

subplot(2,2,3)
plot(d, gc, d, uc)
title("Ascon-p Cycles / bit vs. masking order (x86)")
xlabel("masking order")
ylabel("Cycles / bit")
legend("proposed (generic C)", "usuba compiled", "Location", 'southeast')


load("codesize32.mat")
subplot(2,2,4)
plot(d, gc, d, uc)
title("Ascon-p Codesize vs. masking order (x86)")
xlabel("masking order")
ylabel("Codesize [bytes]")
legend("proposed (generic C)", "usuba compiled", "Location", 'east')


