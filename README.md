# Compile HuanCun

```
> git clone https://github.com/OpenXiangShan/HuanCun.git
> git checkout BRANCH_UNDER_TEST
> make init

Next we need to generate a DUT wrapper. We provide two pre-written wrappers, you can also build new DUT wrappers.
Please see https://github.com/OpenXiangShan/HuanCun/blob/non-inclusive/src/test/scala/huancun/TestTop.scala for more information.
> make test-top-l2    // For StandAlone L2 test
> make test-top-l2l3  // For L2-L3 test
```

# Compile tl-test

```
> git clone https://github.com/OpenXiangShan/tl-test.git
> mkdir build
> cd build

> cmake .. -DDUT_DIR=${PATH_TO_HUANCUN}/build
For trace-dump, add -DTRACE=1
For multi-thread, add -DTHREAD=${NR_THREADS}
For gdb support, add -DCMAKE_BUILD_TYPE=Debug

> make
```

# Run tl-test

```
> ./tlc_test

-s --seed=N        Set N as random seed
-c --cycle=N       Simulate for N cycles
-b --wave-begin=N  Wave dump starts from cycle N
-e --wave-end=N    Wave dump ends to cycle N
-f --wave-full     Dump wave in full cycles
-v --verbose       Verbose mode
Please check -DTRACE=1 is added to tl-test compiling cmake arguments before wave dump.

Tl-test will dump wave to directory tl-test/build/ if wave_begin < wave_end.
```
