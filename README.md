# Compile HuanCun

```
> git clone https://github.com/OpenXiangShan/XiangShan.git
> git checkout nanhu-v2
> cd XiangShan
> make init

Next we need to generate a DUT wrapper. We provide two pre-written wrappers, you can also build new DUT wrappers.
> make verilog
> python3 scripts/parser.py --config DefaultConfig XSTop
```

# Compile tl-test

```
> git clone https://github.com/OpenXiangShan/tl-test.git
> git checkout nanhu-v2-fullsys
> cd tl-test
> mkdir Dut/rtl
> mkdir Dut/sram
> cp ${PATH_TO_XIANGSHAN}/XSTop-*/XSTop/* Dut/rtl/
> cp ${PATH_TO_XIANGSHAN}/XSTop-*/SRAM/* Dut/sram/
> rm Dut/rtl/XSCore.v Dut/rtl/AXI4ToTL.v
> mkdir build
> cd build

> cmake ..
For trace-dump, add -DTRACE=1
For multi-thread, add -DTHREAD=${NR_THREADS}

> make
```

# Run tl-test

```
> ./tlc_test

-s --seed=N        Set N as random seed
-c --cycle=N       Simulate for N cycles
-b --wave-begin=N  Wave dump starts from cycle N
-e --wave-end=N    Wave dump ends to cycle N
-v --verbose       Verbose mode
-m --monitor       Enable TL monitors
-f --fullwave      Dump all wave. It will override --wave-begin=0 --wave-end=cycle
-r --random-mode   Turn off random test mode, more information in Case.h
Please check -DTRACE=1 is added to tl-test compiling cmake arguments before wave dump.

Tl-test will dump wave to directory tl-test/build/ if wave_begin < wave_end.
```