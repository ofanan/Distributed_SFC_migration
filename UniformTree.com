
t2_Async_cpu1_p0.5_sd1_stts1 | nPkts0 = 6 | nPkts1 = 4 | nPkts2 = 6 | nPkts3 = 3 | nBytes0 = 90 | nBytes1 = 50 | nBytes2 = 152 | nBytes3 = 83
// format: t{T}.{Mode}.cpu{C}.stts{s} | nPkts0=... | nPkts1=... | nBytes0=... |  , where
// T is the slot cnt (read from the input file)
// Mode is the algorithm / solver used.
// nPktsi, nBytesi indicate the number of pkts/bytes sent in direction j.
// The directions are determined as follows:
// directions 0, 1, ..., lvlOfRoot-1 indicate pkts whose src is 0,1, ... lvlOfRoot-1, and the direction is north (to prnt).
// directions lvlOfRoot, lvlOfRoot+1, ..., 2*lvlOfRoot-1, indicate pkts whose src is 1,2, ... lvlOfRoot, destined to the child.

t2_Async_cpu1_p0.5_sd1_stts1 | nPkts0 = 6 | nPkts1 = 4 | nPkts2 = 6 | nPkts3 = 3 | nBytes0 = 130 | nBytes1 = 85 | nBytes2 = 152 | nBytes3 = 83
// format: t{T}.{Mode}.cpu{C}.stts{s} | nPkts0=... | nPkts1=... | nBytes0=... |  , where
// T is the slot cnt (read from the input file)
// Mode is the algorithm / solver used.
// nPktsi, nBytesi indicate the number of pkts/bytes sent in direction j.
// The directions are determined as follows:
// directions 0, 1, ..., lvlOfRoot-1 indicate pkts whose src is 0,1, ... lvlOfRoot-1, and the direction is north (to prnt).
// directions lvlOfRoot, lvlOfRoot+1, ..., 2*lvlOfRoot-1, indicate pkts whose src is 1,2, ... lvlOfRoot, destined to the child.

t2_Async_cpu1_p0.5_sd1_stts1 | nPkts0 = 6 | nPkts1 = 4 | nPkts2 = 6 | nPkts3 = 3 | nBytes0 = 130 | nBytes1 = 85 | nBytes2 = 152 | nBytes3 = 83
// format: t{T}.{Mode}.cpu{C}.stts{s} | nPkts0=... | nPkts1=... | nBytes0=... |  , where
// T is the slot cnt (read from the input file)
// Mode is the algorithm / solver used.
// nPktsi, nBytesi indicate the number of pkts/bytes sent in direction j.
// The directions are determined as follows:
// directions 0, 1, ..., lvlOfRoot-1 indicate pkts whose src is 0,1, ... lvlOfRoot-1, and the direction is north (to prnt).
// directions lvlOfRoot, lvlOfRoot+1, ..., 2*lvlOfRoot-1, indicate pkts whose src is 1,2, ... lvlOfRoot, destined to the child.

t2_Async_cpu1_p0.5_sd1_stts1 | nPkts0 = 6 | nPkts1 = 4 | nPkts2 = 6 | nPkts3 = 3 | nBytes0 = 130 | nBytes1 = 85 | nBytes2 = 152 | nBytes3 = 83
// format: t{T}.{Mode}.cpu{C}.stts{s} | nPkts0=... | nPkts1=... | nBytes0=... |  , where
// T is the slot cnt (read from the input file)
// Mode is the algorithm / solver used.
// nPktsi, nBytesi indicate the number of pkts/bytes sent in direction j.
// The directions are determined as follows:
// directions 0, 1, ..., lvlOfRoot-1 indicate pkts whose src is 0,1, ... lvlOfRoot-1, and the direction is north (to prnt).
// directions lvlOfRoot, lvlOfRoot+1, ..., 2*lvlOfRoot-1, indicate pkts whose src is 1,2, ... lvlOfRoot, destined to the child.

t2_Async_cpu2422_p0.5_sd1_stts1 | nPkts0 = 2 | nPkts1 = 1 | nPkts2 = 2 | nPkts3 = 1 | nBytes0 = 65 | nBytes1 = 25 | nBytes2 = 65 | nBytes3 = 25
// format: t{T}.{Mode}.cpu{C}.stts{s} | nPkts0=... | nPkts1=... | nBytes0=... |  , where
// T is the slot cnt (read from the input file)
// Mode is the algorithm / solver used.
// nPktsi, nBytesi indicate the number of pkts/bytes sent in direction j.
// The directions are determined as follows:
// directions 0, 1, ..., lvlOfRoot-1 indicate pkts whose src is 0,1, ... lvlOfRoot-1, and the direction is north (to prnt).
// directions lvlOfRoot, lvlOfRoot+1, ..., 2*lvlOfRoot-1, indicate pkts whose src is 1,2, ... lvlOfRoot, destined to the child.

t2_Async_cpu2422_p0.5_sd1_stts1 | nPkts0 = 2 | nPkts1 = 1 | nPkts2 = 2 | nPkts3 = 1 | nBytes0 = 65 | nBytes1 = 25 | nBytes2 = 65 | nBytes3 = 25
