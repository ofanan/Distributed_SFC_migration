// format: t{T}.{Mode}.cpu{C}.stts{s} | nPkts0=... | nPkts1=... | nBytes0=... |  , where
// T is the slot cnt (read from the input file)
// Mode is the algorithm / solver used.
// nPktsi, nBytesi indicate the number of pkts/bytes sent in direction j.
// The directions are determined as follows:
// directions 0, 1, ..., lvlOfRoot-1 indicate pkts whose src is 0,1, ... lvlOfRoot-1, and the direction is north (to prnt).
// directions lvlOfRoot, lvlOfRoot+1, ..., 2*lvlOfRoot-1, indicate pkts whose src is 1,2, ... lvlOfRoot, destined to the child.

t9_Async_cpu1_p0.5_sd1_stts2 | nPkts0 = 24 | nPkts1 = 17 | nPkts2 = 19 | nPkts3 = 12 | nBytes0 = 355 | nBytes1 = 270 | nBytes2 = 594 | nBytes3 = 392
