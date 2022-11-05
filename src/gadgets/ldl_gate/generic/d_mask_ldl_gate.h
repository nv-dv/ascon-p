#pragma once

#define uint64_rotr(x,n) (((x)>>(n))|((x)<<(64-(n))))

#define dS_LDL(i, j, rot1, rot2) \
		t[j].s[i] = x[j].s[i]; \
		rot[j].s[i] = uint64_rotr(x[j].s[i], rot1); \
		t[j].s[i] ^= rot[j].s[i]; \
		rot[j].s[i] = uint64_rotr(x[j].s[i], rot2); \
		x[j].s[i] = t[j].s[i] ^ rot[j].s[i];
