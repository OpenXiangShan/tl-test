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

# Use ChiselDB

```
First make sure that your HuanCun has huancun.utils.ChiselDB, e.g. HuanCun of branch 'nanhu-chiseldb'.
Build HuanCun and clone tl-test...
Set env variable {TLTEST_HOME} and {HUANCUN_HOME} to your tl-test repositary and HuanCun repositary respectively.

> cd tl-test
> make init (will copy chisel_db.h and chisel_db.cpp to ./Emu)
> mkdir build
> cd build

> cmake .. -DDUT_DIR=${PATH_TO_HUANCUN}/build [ARGS ...]
> make
> ./tlc_test --dump-db [opts]

Then the .db file will write to ${TLTEST_HOME}/build.
*Attention: it won't save the .db file until the tl-test program ends.

Use the following commands to open .db file:
> sqlite3 <database.db> "SELECT * FROM TLLOG"

There is a script in ./script:
> sqlite3 <database.db> "SELECT * FROM TLLOG" | sh ${TO_SCRIPT_DIR}/convert.sh
```