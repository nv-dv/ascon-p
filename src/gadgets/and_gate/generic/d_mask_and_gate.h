#pragma once
#include "ciphers/isapa128av20/generic/d_mask_ascon_p.h"
#ifdef REFRESH
#include "gadgets/refresh/d_mask_refresh.h"
#endif

extern uint64_t Rd[];
extern dshare dS_AND(dshare &a, dshare &b, size_t randindex);
