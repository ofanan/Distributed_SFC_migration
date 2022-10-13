// format: t{T}.{Mode}.cpu{C}.stts{s} | nPkts0=... | nPkts1=... | nBytes0=... |  , where
// T is the slot cnt (read from the input file)
// Mode is the algorithm / solver used.
// nPktsi, nBytesi indicate the number of pkts/bytes sent in direction j.
// The directions are determined as follows:
// directions 0, 1, ..., lvlOfRoot-1 indicate pkts whose src is 0,1, ... lvlOfRoot-1, and the direction is north (to prnt).
// directions lvlOfRoot, lvlOfRoot+1, ..., 2*lvlOfRoot-1, indicate pkts whose src is 1,2, ... lvlOfRoot, destined to the child.

t30000_Async_cpu1_p1.0_sd1_stts2 | nPkts0 = 3043 | nPkts1 = 452 | nPkts2 = 1 | nPkts3 = 0 | nPkts4 = 0 | nPkts5 = 1519 | nPkts6 = 226 | nPkts7 = 0 | nPkts8 = 0 | nPkts9 = 0 | nBytes0 = 88450 | nBytes1 = 60500 | nBytes2 = 10 | nBytes3 = 0 | nBytes4 = 0 | nBytes5 = 0 | nBytes6 = 0 | nBytes7 = 0 | nBytes8 = 0 | nBytes9 = 0
