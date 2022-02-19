# Compile
```shell
> mkdir build
> cd build
> cmake .. -DDUT_DIR=PATH_TO_HUANCUN/build

For trace-dump, add -DTRACE=1
For multi-thread, add -DTHREAD=${NR_THREADS}
```

# Run
```shell
> ./tlc_test

-s --seed=N        Set N as random seed
-b --wave-begin=N  Wave dump starts from cycle N
-e --wave-end=N    Wave dump ends to cycle N
```