// format: t{T}.{Mode}.cpu{C}.stts{s} | nPkts0=... | nPkts1=... | nBytes0=... |  , where
// T is the slot cnt (read from the input file)
// Mode is the algorithm / solver used.
// nPktsi, nBytesi indicate the number of pkts/bytes sent in direction j.
// The directions are determined as follows:
// directions 0, 1, ..., lvlOfRoot-1 indicate pkts whose src is 0,1, ... lvlOfRoot-1, and the direction is north (to prnt).
// directions lvlOfRoot, lvlOfRoot+1, ..., 2*lvlOfRoot-1, indicate pkts whose src is 1,2, ... lvlOfRoot, destined to the child.

t30599_Async_cpu2422_p0.5_sd1_stts1 | nPkts0 = 22937 | nPkts1 = 21066 | nPkts2 = 3229 | nPkts3 = 2931 | nPkts4 = 1874 | nPkts5 = 22937 | nPkts6 = 21066 | nPkts7 = 3229 | nPkts8 = 2931 | nPkts9 = 1874 | nBytes0 = 495950 | nBytes1 = 477240 | nBytes2 = 85360 | nBytes3 = 82380 | nBytes4 = 71810 | nBytes5 = 495950 | nBytes6 = 477240 | nBytes7 = 85360 | nBytes8 = 82380 | nBytes9 = 71810
// format: t{T}.{Mode}.cpu{C}.stts{s} | nPkts0=... | nPkts1=... | nBytes0=... |  , where
// T is the slot cnt (read from the input file)
// Mode is the algorithm / solver used.
// nPktsi, nBytesi indicate the number of pkts/bytes sent in direction j.
// The directions are determined as follows:
// directions 0, 1, ..., lvlOfRoot-1 indicate pkts whose src is 0,1, ... lvlOfRoot-1, and the direction is north (to prnt).
// directions lvlOfRoot, lvlOfRoot+1, ..., 2*lvlOfRoot-1, indicate pkts whose src is 1,2, ... lvlOfRoot, destined to the child.

t30004_Async_cpu94_p0.3_sd1_stts2 | nPkts0 = 498 | nPkts1 = 279 | nPkts2 = 127 | nPkts3 = 60 | nPkts4 = 20 | nPkts5 = 444 | nPkts6 = 250 | nPkts7 = 120 | nPkts8 = 56 | nPkts9 = 17 | nBytes0 = 12385 | nBytes1 = 9270 | nBytes2 = 4934 | nBytes3 = 4170 | nBytes4 = 3590 | nBytes5 = 14094 | nBytes6 = 11196 | nBytes7 = 7353 | nBytes8 = 6014 | nBytes9 = 4731
// format: t{T}.{Mode}.cpu{C}.stts{s} | nPkts0=... | nPkts1=... | nBytes0=... |  , where
// T is the slot cnt (read from the input file)
// Mode is the algorithm / solver used.
// nPktsi, nBytesi indicate the number of pkts/bytes sent in direction j.
// The directions are determined as follows:
// directions 0, 1, ..., lvlOfRoot-1 indicate pkts whose src is 0,1, ... lvlOfRoot-1, and the direction is north (to prnt).
// directions lvlOfRoot, lvlOfRoot+1, ..., 2*lvlOfRoot-1, indicate pkts whose src is 1,2, ... lvlOfRoot, destined to the child.

t30008_Async_cpu103_p0.3_sd1_stts2 | nPkts0 = 899 | nPkts1 = 522 | nPkts2 = 225 | nPkts3 = 128 | nPkts4 = 42 | nPkts5 = 842 | nPkts6 = 478 | nPkts7 = 195 | nPkts8 = 108 | nPkts9 = 31 | nBytes0 = 22690 | nBytes1 = 16685 | nBytes2 = 8625 | nBytes3 = 7550 | nBytes4 = 5895 | nBytes5 = 25160 | nBytes6 = 21504 | nBytes7 = 12933 | nBytes8 = 11790 | nBytes9 = 7767
// format: t{T}.{Mode}.cpu{C}.stts{s} | nPkts0=... | nPkts1=... | nBytes0=... |  , where
// T is the slot cnt (read from the input file)
// Mode is the algorithm / solver used.
// nPktsi, nBytesi indicate the number of pkts/bytes sent in direction j.
// The directions are determined as follows:
// directions 0, 1, ..., lvlOfRoot-1 indicate pkts whose src is 0,1, ... lvlOfRoot-1, and the direction is north (to prnt).
// directions lvlOfRoot, lvlOfRoot+1, ..., 2*lvlOfRoot-1, indicate pkts whose src is 1,2, ... lvlOfRoot, destined to the child.

t30008_Async_cpu113_p0.3_sd1_stts2 | nPkts0 = 817 | nPkts1 = 494 | nPkts2 = 225 | nPkts3 = 125 | nPkts4 = 39 | nPkts5 = 744 | nPkts6 = 445 | nPkts7 = 183 | nPkts8 = 100 | nPkts9 = 29 | nBytes0 = 20585 | nBytes1 = 15315 | nBytes2 = 8085 | nBytes3 = 7055 | nBytes4 = 5220 | nBytes5 = 21941 | nBytes6 = 19027 | nBytes7 = 13507 | nBytes8 = 10352 | nBytes9 = 6633
// format: t{T}.{Mode}.cpu{C}.stts{s} | nPkts0=... | nPkts1=... | nBytes0=... |  , where
// T is the slot cnt (read from the input file)
// Mode is the algorithm / solver used.
// nPktsi, nBytesi indicate the number of pkts/bytes sent in direction j.
// The directions are determined as follows:
// directions 0, 1, ..., lvlOfRoot-1 indicate pkts whose src is 0,1, ... lvlOfRoot-1, and the direction is north (to prnt).
// directions lvlOfRoot, lvlOfRoot+1, ..., 2*lvlOfRoot-1, indicate pkts whose src is 1,2, ... lvlOfRoot, destined to the child.

t30321_Async_cpu122_p0.3_sd1_stts2 | nPkts0 = 32820 | nPkts1 = 21463 | nPkts2 = 10024 | nPkts3 = 4337 | nPkts4 = 1097 | nPkts5 = 32167 | nPkts6 = 21249 | nPkts7 = 9814 | nPkts8 = 4261 | nPkts9 = 1065 | nBytes0 = 825210 | nBytes1 = 664270 | nBytes2 = 372835 | nBytes3 = 291710 | nBytes4 = 230195 | nBytes5 = 810088 | nBytes6 = 668871 | nBytes7 = 384588 | nBytes8 = 308547 | nBytes9 = 240097
// format: t{T}.{Mode}.cpu{C}.stts{s} | nPkts0=... | nPkts1=... | nBytes0=... |  , where
// T is the slot cnt (read from the input file)
// Mode is the algorithm / solver used.
// nPktsi, nBytesi indicate the number of pkts/bytes sent in direction j.
// The directions are determined as follows:
// directions 0, 1, ..., lvlOfRoot-1 indicate pkts whose src is 0,1, ... lvlOfRoot-1, and the direction is north (to prnt).
// directions lvlOfRoot, lvlOfRoot+1, ..., 2*lvlOfRoot-1, indicate pkts whose src is 1,2, ... lvlOfRoot, destined to the child.

t30250_Async_cpu132_p0.3_sd1_stts2 | nPkts0 = 23509 | nPkts1 = 16072 | nPkts2 = 7529 | nPkts3 = 3340 | nPkts4 = 861 | nPkts5 = 23159 | nPkts6 = 15981 | nPkts7 = 7441 | nPkts8 = 3311 | nPkts9 = 844 | nBytes0 = 584445 | nBytes1 = 488030 | nBytes2 = 276590 | nBytes3 = 216790 | nBytes4 = 171105 | nBytes5 = 574148 | nBytes6 = 488885 | nBytes7 = 279774 | nBytes8 = 228476 | nBytes9 = 179465
// format: t{T}.{Mode}.cpu{C}.stts{s} | nPkts0=... | nPkts1=... | nBytes0=... |  , where
// T is the slot cnt (read from the input file)
// Mode is the algorithm / solver used.
// nPktsi, nBytesi indicate the number of pkts/bytes sent in direction j.
// The directions are determined as follows:
// directions 0, 1, ..., lvlOfRoot-1 indicate pkts whose src is 0,1, ... lvlOfRoot-1, and the direction is north (to prnt).
// directions lvlOfRoot, lvlOfRoot+1, ..., 2*lvlOfRoot-1, indicate pkts whose src is 1,2, ... lvlOfRoot, destined to the child.

t30599_Async_cpu141_p0.3_sd1_stts1 | nPkts0 = 52323 | nPkts1 = 37310 | nPkts2 = 17868 | nPkts3 = 8184 | nPkts4 = 2144 | nPkts5 = 51851 | nPkts6 = 37235 | nPkts7 = 17767 | nPkts8 = 8142 | nPkts9 = 2118 | nBytes0 = 1291830 | nBytes1 = 1105725 | nBytes2 = 643110 | nBytes3 = 523380 | nBytes4 = 424445 | nBytes5 = 1277082 | nBytes6 = 1108679 | nBytes7 = 651365 | nBytes8 = 536396 | nBytes9 = 440643
// format: t{T}.{Mode}.cpu{C}.stts{s} | nPkts0=... | nPkts1=... | nBytes0=... |  , where
// T is the slot cnt (read from the input file)
// Mode is the algorithm / solver used.
// nPktsi, nBytesi indicate the number of pkts/bytes sent in direction j.
// The directions are determined as follows:
// directions 0, 1, ..., lvlOfRoot-1 indicate pkts whose src is 0,1, ... lvlOfRoot-1, and the direction is north (to prnt).
// directions lvlOfRoot, lvlOfRoot+1, ..., 2*lvlOfRoot-1, indicate pkts whose src is 1,2, ... lvlOfRoot, destined to the child.

t30599_Async_cpu150_p0.3_sd1_stts1 | nPkts0 = 49085 | nPkts1 = 36225 | nPkts2 = 17477 | nPkts3 = 8196 | nPkts4 = 2182 | nPkts5 = 48720 | nPkts6 = 36167 | nPkts7 = 17413 | nPkts8 = 8167 | nPkts9 = 2171 | nBytes0 = 1205500 | nBytes1 = 1056205 | nBytes2 = 618995 | nBytes3 = 508650 | nBytes4 = 415255 | nBytes5 = 1193772 | nBytes6 = 1055455 | nBytes7 = 620339 | nBytes8 = 516330 | nBytes9 = 427381
// format: t{T}.{Mode}.cpu{C}.stts{s} | nPkts0=... | nPkts1=... | nBytes0=... |  , where
// T is the slot cnt (read from the input file)
// Mode is the algorithm / solver used.
// nPktsi, nBytesi indicate the number of pkts/bytes sent in direction j.
// The directions are determined as follows:
// directions 0, 1, ..., lvlOfRoot-1 indicate pkts whose src is 0,1, ... lvlOfRoot-1, and the direction is north (to prnt).
// directions lvlOfRoot, lvlOfRoot+1, ..., 2*lvlOfRoot-1, indicate pkts whose src is 1,2, ... lvlOfRoot, destined to the child.

t30599_Async_cpu160_p0.3_sd1_stts1 | nPkts0 = 46058 | nPkts1 = 35097 | nPkts2 = 17264 | nPkts3 = 8254 | nPkts4 = 2255 | nPkts5 = 45919 | nPkts6 = 35069 | nPkts7 = 17236 | nPkts8 = 8243 | nPkts9 = 2245 | nBytes0 = 1124080 | nBytes1 = 1009385 | nBytes2 = 602375 | nBytes3 = 501640 | nBytes4 = 417440 | nBytes5 = 1119060 | nBytes6 = 1008452 | nBytes7 = 603246 | nBytes8 = 507063 | nBytes9 = 426705
// format: t{T}.{Mode}.cpu{C}.stts{s} | nPkts0=... | nPkts1=... | nBytes0=... |  , where
// T is the slot cnt (read from the input file)
// Mode is the algorithm / solver used.
// nPktsi, nBytesi indicate the number of pkts/bytes sent in direction j.
// The directions are determined as follows:
// directions 0, 1, ..., lvlOfRoot-1 indicate pkts whose src is 0,1, ... lvlOfRoot-1, and the direction is north (to prnt).
// directions lvlOfRoot, lvlOfRoot+1, ..., 2*lvlOfRoot-1, indicate pkts whose src is 1,2, ... lvlOfRoot, destined to the child.

t30599_Async_cpu169_p0.3_sd1_stts1 | nPkts0 = 43668 | nPkts1 = 34005 | nPkts2 = 16818 | nPkts3 = 8226 | nPkts4 = 2275 | nPkts5 = 43558 | nPkts6 = 33993 | nPkts7 = 16800 | nPkts8 = 8217 | nPkts9 = 2272 | nBytes0 = 1064095 | nBytes1 = 962970 | nBytes2 = 576075 | nBytes3 = 483780 | nBytes4 = 404140 | nBytes5 = 1060088 | nBytes6 = 962227 | nBytes7 = 576848 | nBytes8 = 487080 | nBytes9 = 411745
// format: t{T}.{Mode}.cpu{C}.stts{s} | nPkts0=... | nPkts1=... | nBytes0=... |  , where
// T is the slot cnt (read from the input file)
// Mode is the algorithm / solver used.
// nPktsi, nBytesi indicate the number of pkts/bytes sent in direction j.
// The directions are determined as follows:
// directions 0, 1, ..., lvlOfRoot-1 indicate pkts whose src is 0,1, ... lvlOfRoot-1, and the direction is north (to prnt).
// directions lvlOfRoot, lvlOfRoot+1, ..., 2*lvlOfRoot-1, indicate pkts whose src is 1,2, ... lvlOfRoot, destined to the child.

t30599_Async_cpu179_p0.3_sd1_stts1 | nPkts0 = 40543 | nPkts1 = 32650 | nPkts2 = 16350 | nPkts3 = 8257 | nPkts4 = 2379 | nPkts5 = 40518 | nPkts6 = 32650 | nPkts7 = 16345 | nPkts8 = 8256 | nPkts9 = 2379 | nBytes0 = 986385 | nBytes1 = 905770 | nBytes2 = 547980 | nBytes3 = 466165 | nBytes4 = 404580 | nBytes5 = 985467 | nBytes6 = 905761 | nBytes7 = 548092 | nBytes8 = 466673 | nBytes9 = 405728
// format: t{T}.{Mode}.cpu{C}.stts{s} | nPkts0=... | nPkts1=... | nBytes0=... |  , where
// T is the slot cnt (read from the input file)
// Mode is the algorithm / solver used.
// nPktsi, nBytesi indicate the number of pkts/bytes sent in direction j.
// The directions are determined as follows:
// directions 0, 1, ..., lvlOfRoot-1 indicate pkts whose src is 0,1, ... lvlOfRoot-1, and the direction is north (to prnt).
// directions lvlOfRoot, lvlOfRoot+1, ..., 2*lvlOfRoot-1, indicate pkts whose src is 1,2, ... lvlOfRoot, destined to the child.

t30599_Async_cpu188_p0.3_sd1_stts1 | nPkts0 = 38619 | nPkts1 = 31708 | nPkts2 = 16020 | nPkts3 = 8211 | nPkts4 = 2386 | nPkts5 = 38605 | nPkts6 = 31706 | nPkts7 = 16019 | nPkts8 = 8211 | nPkts9 = 2385 | nBytes0 = 937900 | nBytes1 = 868790 | nBytes2 = 528030 | nBytes3 = 449415 | nBytes4 = 389215 | nBytes5 = 937450 | nBytes6 = 868620 | nBytes7 = 528373 | nBytes8 = 450143 | nBytes9 = 390443
// format: t{T}.{Mode}.cpu{C}.stts{s} | nPkts0=... | nPkts1=... | nBytes0=... |  , where
// T is the slot cnt (read from the input file)
// Mode is the algorithm / solver used.
// nPktsi, nBytesi indicate the number of pkts/bytes sent in direction j.
// The directions are determined as follows:
// directions 0, 1, ..., lvlOfRoot-1 indicate pkts whose src is 0,1, ... lvlOfRoot-1, and the direction is north (to prnt).
// directions lvlOfRoot, lvlOfRoot+1, ..., 2*lvlOfRoot-1, indicate pkts whose src is 1,2, ... lvlOfRoot, destined to the child.

t30599_Async_cpu197_p0.3_sd1_stts1 | nPkts0 = 37176 | nPkts1 = 30919 | nPkts2 = 15694 | nPkts3 = 8132 | nPkts4 = 2396 | nPkts5 = 37174 | nPkts6 = 30919 | nPkts7 = 15694 | nPkts8 = 8132 | nPkts9 = 2396 | nBytes0 = 902590 | nBytes1 = 840020 | nBytes2 = 511630 | nBytes3 = 436010 | nBytes4 = 378650 | nBytes5 = 902510 | nBytes6 = 839975 | nBytes7 = 511600 | nBytes8 = 435995 | nBytes9 = 378635
// format: t{T}.{Mode}.cpu{C}.stts{s} | nPkts0=... | nPkts1=... | nBytes0=... |  , where
// T is the slot cnt (read from the input file)
// Mode is the algorithm / solver used.
// nPktsi, nBytesi indicate the number of pkts/bytes sent in direction j.
// The directions are determined as follows:
// directions 0, 1, ..., lvlOfRoot-1 indicate pkts whose src is 0,1, ... lvlOfRoot-1, and the direction is north (to prnt).
// directions lvlOfRoot, lvlOfRoot+1, ..., 2*lvlOfRoot-1, indicate pkts whose src is 1,2, ... lvlOfRoot, destined to the child.

t30599_Async_cpu207_p0.3_sd1_stts1 | nPkts0 = 35707 | nPkts1 = 30143 | nPkts2 = 15335 | nPkts3 = 8090 | nPkts4 = 2396 | nPkts5 = 35707 | nPkts6 = 30143 | nPkts7 = 15335 | nPkts8 = 8090 | nPkts9 = 2396 | nBytes0 = 865995 | nBytes1 = 810355 | nBytes2 = 493655 | nBytes3 = 421205 | nBytes4 = 364265 | nBytes5 = 865995 | nBytes6 = 810355 | nBytes7 = 493655 | nBytes8 = 421205 | nBytes9 = 364265
