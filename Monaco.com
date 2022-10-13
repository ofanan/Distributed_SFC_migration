// format: t{T}.{Mode}.cpu{C}.stts{s} | nPkts0=... | nPkts1=... | nBytes0=... |  , where
// T is the slot cnt (read from the input file)
// Mode is the algorithm / solver used.
// nPktsi, nBytesi indicate the number of pkts/bytes sent in direction j.
// The directions are determined as follows:
// directions 0, 1, ..., lvlOfRoot-1 indicate pkts whose src is 0,1, ... lvlOfRoot-1, and the direction is north (to prnt).
// directions lvlOfRoot, lvlOfRoot+1, ..., 2*lvlOfRoot-1, indicate pkts whose src is 1,2, ... lvlOfRoot, destined to the child.

t30599_Async_cpu2422_p0.5_sd1_stts1 | nPkts0 = 9097 | nPkts1 = 8755 | nPkts2 = 2159 | nPkts3 = 1886 | nPkts4 = 1277 | nPkts5 = 9097 | nPkts6 = 8755 | nPkts7 = 2159 | nPkts8 = 1886 | nPkts9 = 1277 | nBytes0 = 206275 | nBytes1 = 202855 | nBytes2 = 56585 | nBytes3 = 53855 | nBytes4 = 47765 | nBytes5 = 206275 | nBytes6 = 202855 | nBytes7 = 56585 | nBytes8 = 53855 | nBytes9 = 47765
// format: t{T}.{Mode}.cpu{C}.stts{s} | nPkts0=... | nPkts1=... | nBytes0=... |  , where
// T is the slot cnt (read from the input file)
// Mode is the algorithm / solver used.
// nPktsi, nBytesi indicate the number of pkts/bytes sent in direction j.
// The directions are determined as follows:
// directions 0, 1, ..., lvlOfRoot-1 indicate pkts whose src is 0,1, ... lvlOfRoot-1, and the direction is north (to prnt).
// directions lvlOfRoot, lvlOfRoot+1, ..., 2*lvlOfRoot-1, indicate pkts whose src is 1,2, ... lvlOfRoot, destined to the child.

t30599_Async_cpu2422_p0.5_sd1_stts1 | nPkts0 = 9097 | nPkts1 = 8755 | nPkts2 = 2159 | nPkts3 = 1886 | nPkts4 = 1277 | nPkts5 = 9097 | nPkts6 = 8755 | nPkts7 = 2159 | nPkts8 = 1886 | nPkts9 = 1277 | nBytes0 = 206275 | nBytes1 = 202855 | nBytes2 = 56585 | nBytes3 = 53855 | nBytes4 = 47765 | nBytes5 = 206275 | nBytes6 = 202855 | nBytes7 = 56585 | nBytes8 = 53855 | nBytes9 = 47765
// format: t{T}.{Mode}.cpu{C}.stts{s} | nPkts0=... | nPkts1=... | nBytes0=... |  , where
// T is the slot cnt (read from the input file)
// Mode is the algorithm / solver used.
// nPktsi, nBytesi indicate the number of pkts/bytes sent in direction j.
// The directions are determined as follows:
// directions 0, 1, ..., lvlOfRoot-1 indicate pkts whose src is 0,1, ... lvlOfRoot-1, and the direction is north (to prnt).
// directions lvlOfRoot, lvlOfRoot+1, ..., 2*lvlOfRoot-1, indicate pkts whose src is 1,2, ... lvlOfRoot, destined to the child.

t30599_Async_cpu2422_p0.5_sd1_stts1 | nPkts0 = 9097 | nPkts1 = 8755 | nPkts2 = 2159 | nPkts3 = 1886 | nPkts4 = 1277 | nPkts5 = 9097 | nPkts6 = 8755 | nPkts7 = 2159 | nPkts8 = 1886 | nPkts9 = 1277 | nBytes0 = 206275 | nBytes1 = 202855 | nBytes2 = 56585 | nBytes3 = 53855 | nBytes4 = 47765 | nBytes5 = 206275 | nBytes6 = 202855 | nBytes7 = 56585 | nBytes8 = 53855 | nBytes9 = 47765
