#!/bin/bash

# MBR offset=0+446
dd if=hd.img of=mbr ibs=1 skip=0 count=512
dd if=hd.img of=mbr_gpt ibs=1 skip=446 count=64

# 1 offset=2048*512=1048576
dd if=hd.img of=mbr1 ibs=1 skip=1048576 count=512
# 2 offset=67584*512=34603008
dd if=hd.img of=mbr2 ibs=1 skip=34603008 count=512
# 3 offset=133120*512=68157440
dd if=hd.img of=mbr3 ibs=1 skip=68157440 count=512

# 4 extened 
# EBR1 offset=198656*512+446=101712318
dd if=hd.img of=ebr1 ibs=1 skip=101711872 count=512
dd if=hd.img of=ebr1_gpt ibs=1 skip=101712318 count=64
# EBR2 offset=(266239+1)*512+446=136315326
dd if=hd.img of=ebr2 ibs=1 skip=136314880 count=512
dd if=hd.img of=ebr2_gpt ibs=1 skip=136315326 count=64
# EBR3 offset=(333823+1)*512+446=170918334
dd if=hd.img of=ebr3 ibs=1 skip=170917888 count=512
dd if=hd.img of=ebr3_gpt ibs=1 skip=170918334 count=64
# EBR4 offset=(401407+1)*512+446=205521342
dd if=hd.img of=ebr4 ibs=1 skip=205520896 count=512
dd if=hd.img of=ebr4_gpt ibs=1 skip=205521342 count=64
# EBR5 offset=(468991+1)*512+446=240124350
dd if=hd.img of=ebr5 ibs=1 skip=240123904 count=512
dd if=hd.img of=ebr5_gpt ibs=1 skip=240124350 count=64

# Device     Boot  Start    End Sectors  Size Id Type
# hd.img1           2048  67583   65536   32M 83 Linux
# hd.img2          67584 133119   65536   32M 83 Linux
# hd.img3         133120 198655   65536   32M 83 Linux
# hd.img4         198656 524159  325504  159M  5 Extended
# hd.img5         200704 266239   65536   32M 83 Linux
# hd.img6         268288 333823   65536   32M 83 Linux
# hd.img7         335872 401407   65536   32M 83 Linux
# hd.img8         403456 468991   65536   32M 83 Linux
# hd.img9    *    471040 524159   53120   26M 83 Linux

# Device     Boot  Start    End Sectors Id Type     Start-C/H/S End-C/H/S Attrs
# hd.img1           2048  67583   65536 83 Linux        0/32/33   4/52/48
# hd.img2          67584 133119   65536 83 Linux        4/52/49    8/73/1
# hd.img3         133120 198655   65536 83 Linux         8/73/2  12/93/17
# hd.img4         198656 524159  325504  5 Extended    12/93/18 32/159/63
# hd.img5         200704 266239   65536 83 Linux      12/125/50  16/146/2
# hd.img6         268288 333823   65536 83 Linux      16/178/35 20/198/50
# hd.img7         335872 401407   65536 83 Linux      20/231/20 24/251/35
# hd.img8         403456 468991   65536 83 Linux        25/29/5  29/49/20
# hd.img9    *    471040 524159   53120 83 Linux       29/81/53 32/159/63    80


