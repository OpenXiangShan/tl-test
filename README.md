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
   --dump-db       Dump database file
Please check -DTRACE=1 is added to tl-test compiling cmake arguments before wave dump.

Tl-test will dump wave to directory tl-test/build/ if wave_begin < wave_end.
```

# Use TLLog

TLLog is a method that can capture messages on TileLink bus, which may be helpful for debugging. To use TLLog:

Make sure there is ChiselDB.scala under coupledL2.utils.

Make sure that SQlite3 has been installed in your machine.

Build CoupledL2 and clone tl-test following above steps.

```
> cmake .. -DDUT_DIR=${PATH_TO_HUANCUN}/build -DTLLOG=1
> make
> ./tlc_test --dump-db
```

The .db file will write to tl-test/build.

Use the following commands to open .db file:
```
> sqlite3 <database.db> "SELECT * FROM TLLOG"
```

To filter the table by a specific entry:
```
> sqlite3 <database.db> "SELECT * FROM TLLOG WHERE <ENTRY>=<VALUE>"
```

e.g. 
1. see all messages between L1D and L2:
```
> sqlite3 <database.db> "SELECT * FROM TLLOG WHERE SITE='L1D_L2'"
```

2. see all messages on channel A
```
> sqlite3 <database.db> "SELECT * FROM TLLOG WHERE CHANNEL='0'"
```

There is a script in tl-test/script, which help you get better experience on reading the database file:
```
> sqlite3 <database.db> "SELECT * FROM TLLOG" | sh script/parseTLLog.sh
```
