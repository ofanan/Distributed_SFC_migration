// format: t{T}.{Mode}.cpu{C}.stts{s} | nPkts0=... | nPkts1=... | nBytes0=... |  , where
// T is the slot cnt (read from the input file)
// Mode is the algorithm / solver used.
// nPktsi, nBytesi indicate the number of pkts/bytes sent in direction j.
// The directions are determined as follows:
// directions 0, 1, ..., lvlOfRoot-1 indicate pkts whose src is 0,1, ... lvlOfRoot-1, and the direction is north (to prnt).
// directions lvlOfRoot, lvlOfRoot+1, ..., 2*lvlOfRoot-1, indicate pkts whose src is 1,2, ... lvlOfRoot, destined to the child.

t30599_Async_cpu2422_p0.5_sd1_stts1 | nPkts0 = 22937 | nPkts1 = 21066 | nPkts2 = 3229 | nPkts3 = 2931 | nPkts4 = 1874 | nPkts5 = 22937 | nPkts6 = 21066 | nPkts7 = 3229 | nPkts8 = 2931 | nPkts9 = 1874 | nBytes0 = 495950 | nBytes1 = 477240 | nBytes2 = 85360 | nBytes3 = 82380 | nBytes4 = 71810 | nBytes5 = 495950 | nBytes6 = 477240 | nBytes7 = 85360 | nBytes8 = 82380 | nBytes9 = 71810
