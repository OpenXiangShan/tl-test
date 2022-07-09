# Compile HuanCun

```
> git clone https://github.com/OpenXiangShan/HuanCun.git
> make init
> make compile

We provide two kinds of tests here, you can also build new tests.
Please see https://github.com/OpenXiangShan/HuanCun/blob/non-inclusive/src/test/scala/huancun/TestTop.scala for more information.
> make test-top-l2    // For L2 test
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

> make
```

# Run tl-test

```
> ./tlc_test

-s --seed=N        Set N as random seed
-c --cycle=N       Simulate for N cycles
-b --wave-begin=N  Wave dump starts from cycle N
-e --wave-end=N    Wave dump ends to cycle N
Please check -DTRACE=1 is added to tl-test compiling cmake arguments before wave dump.

Tl-test will dump wave to directory tl-test/build/ if wave_begin < wave_end.
```