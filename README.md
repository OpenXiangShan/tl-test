# TL-Test-New

> TL-Test-New  
> The **Unified TileLink Memory Subsystem Tester for XiangShan**
> 

>   
> **"WELCOME TO THE NEW WORLD !"**  
> The Next Generation TL-Test
> ```
> ████████╗██╗           ████████╗███████╗███████╗████████╗      ███╗   ██╗███████╗██╗    ██╗ 
> ╚══██╔══╝██║           ╚══██╔══╝██╔════╝██╔════╝╚══██╔══╝      ████╗  ██║██╔════╝██║    ██║ 
>    ██║   ██║     █████╗   ██║   █████╗  ███████╗   ██║   █████╗██╔██╗ ██║█████╗  ██║ █╗ ██║ 
>    ██║   ██║     ╚════╝   ██║   ██╔══╝  ╚════██║   ██║   ╚════╝██║╚██╗██║██╔══╝  ██║███╗██║ 
>    ██║   ███████╗         ██║   ███████╗███████║   ██║         ██║ ╚████║███████╗╚███╔███╔╝ 
>    ╚═╝   ╚══════╝         ╚═╝   ╚══════╝╚══════╝   ╚═╝         ╚═╝  ╚═══╝╚══════╝ ╚══╝╚══╝    
> ```  
> &emsp;



## Features
<img src="assets/README.overall.png" width = "220" alt="TL-Test-New overall" align=right />

### 1. Simulator compatibility
* Full simulation compatiblity support for both **Verilator Host Mode** and **DPI Guest Mode**  
    * **Verilator Host Mode**: Run under **verilator** simulator as test host
    * **DPI Guest Mode**: Run under **ALL** verilog simulator as test guest (tested on **VCS & Verdi**)  

### 2. Memory sub-system testing for XiangShan  
* Accurate **memory consistency check** using scoreboard  
* Support both **inclusive** and **non-inclusive** L1 to L2 behaviour  
* Specially constrained TileLink transaction sequence for L2 (**CoupledL2** for now) private design  
* Cache line alias control for L2 (**CoupledL2** for now) private design  
* Easy post-compile TileLink agent and port configuration (for core count, TL-C port count for L1D, TL-UL port count for L1I & PTW ...)   

### 3. Extensibility and legacy debugging support 
* [BullsEye Gravity](https://github.com/arch-simulator-sig/BullsEye/tree/master) **EventBus utility** for event-driven extensive controls  
* Full support for **ChiselDB** through plug-in  
* Detailed and fine-grained controllable verbose information    
* Now **zero memory leakage** in TileLink testing sub-system  
* Almost zero change required for current L1, L2 RTL design   
* **TODO:** TileLink Protocol Analyzer (TLPA - TLEye) through ChiselDB  
* **TODO:** XiangShan L1 Trace support  
* **TODO:** Fuzzing Trace for case reproduciblity  
* **TODO:** CHILog integration for CoupledL2-CHI  

### 4. Automated Range Iteration (ARI) fuzz testing  
* Multiple fuzzing constraints (targeting at different fuzz testing cases) could be specified on compile-time for every single run  
* Automated iteration of fuzzing constraints on runtime  
* Advanced control of TileLink transaction sequencing  

### 5. Automated Port Connection (PortGen) for verilator host
* No more manual wiring on TileLink ports
* Both pre-compile (**static mode**) and post-compile (**dynamic mode**) port connection support with dynamic library


## Project tree
```
.
├── README.md               
└── main            <- TL-Test-New main project root
    ├── Base                <- Basic components
    ├── CMakeLists.txt      <- CMake makefile
    ├── DPI                 <- DPI support components (DPI headers, DPI targeted configs ...)
    ├── Events              <- Testing system event components
    ├── Fuzzer              <- Fuzzer & ARI implementations
    ├── Plugins             <- Embedded plugins
    ├── PortGen             <- PortGen components
    ├── Sequencer           <- TL-Test-New testing system object
    ├── System              <- TL-Test-New testing system procedures
    ├── TLAgent             <- TileLink C & UL agents
    ├── Utils               <- Utilities
    └── V3                  <- Verilator host main
```

## Build
### 1. Build
#### 1.1 Before you build
&emsp;&emsp;Make sure that you are currently under the **main folder** of TL-Test-New ```cd main```.  
&emsp;&emsp;For first time build:
```shell 
mkdir build && cd build
cmake ..
```
#### 1.2 Verilating
> This step could be **skipped** if you chose **NOT TO BUILD Verilator Host Mode** components by: 
> ```shell
> cmake .. -DBUILD_V3=0
> ```   
> For more information, see chatper [**Build Configuration**](#build-configuration).
> 
&emsp;&emsp;Then you need to generate cpp files and library files using verilator.  
&emsp;&emsp;Requirements of verilating:
*  Use ```--cc``` parameter to generate C++ files  
*  Use ```--lib-create vltdut``` parameter to specify the output library to **```libvltdut.a```**   

&emsp;&emsp;Verilator build script for reference:  
```shell
verilator --trace --cc --build --lib-create vltdut --Mdir ./verilated ./coupledL2/build/*.v -Wno-fatal --top TestTop
```
> **INFO:**  
> For this error emitting on verilating CoupledL2: 
> ```log
> %Error: coupledL2/build/LogPerfHelper.v:14:24: Can't find definition of scope/variable: 'SimTop'
>  14 | assign timer         = SimTop.timer;
>     |                        ^~~~~~
> ```  
> In file **```LogPerfHelper.v```**: 
> ```verilog
> `ifndef SIM_TOP_MODULE_NAME
>     `define SIM_TOP_MODULE_NAME SimTop
> `endif
> ```
> Modify the default ```SIM_TOP_MODULE_NAME``` macro definition from ```SimTop``` to ```TestTop```. 
> 

#### 1.3 Building
```shell
make -j `nproc`
```  
> **NOTICE:** 
> Before you **make**:  
> * You need to set-up related project path and environment variables correctly, see chapter [**Build Configuration**](#build-configuration)  
> * You need to set-up verilator include path correctly for **Verilator Host Mode**, see chapter [**Build Configuration**](#build-configuration)  
> * You need to **set-up PortGen** correctly for **Verilator Host Mode**, see chapter [**Build Configuration**](#build-configuration)  
> 
&emsp;&emsp;The default working tree without user configuration is:  
```
.
├── README.md               
├── main            <- TL-Test-New main project root
└── verilated       <- Verilator output directory of DUT
    ├── VTestTop.h          <- The verilator generated cpp top header of DUT
    ├── libvltdut.a         <- The verilator generated static library of DUT
    └── ...
```  
&emsp;&emsp;After build, these main files would be emitted on full build:  
```
.
├── README.md
└── main
    ├── ...
    └── build
        ├── ...
        ├── libtltest_dpi.so        <- Shared library for DPI Guest Mode
        ├── portgen                 <- Static PortGen application
        ├── portgen.cpp             <- Static PortGen generated cpp source
        ├── tltest.ini              <- User configuration file for PortGen
        ├── tltest_portgen.so       <- Static PortGen runtime shared library
        └── tltest_v3lt             <- Verilator Host Mode executable
```

### 2. Clean
```shell
make clean
```

## Build Configuration
### 1. Paths and executables  
#### 1.1 ```CMAKE_CXX_COMPILER```  
&emsp;&emsp;Specify the user-defined C++ compiler for build-time and run-time procedures on demand.  
* Configuring: 
    * ```cmake .. -DCMAKE_CXX_COMPILER=<compiler>```   
* Passing to compiler:
    * ```-DCXX_COMPILER="${CMAKE_CXX_COMPILER}"``` as global macro ```CXX_COMPILER```

#### 1.2 ```CMAKE_CURRENT_SOURCE_DIR```  
&emsp;&emsp;Specify the user-defined current source directory on demand.  
* Configuring:
    * ```cmake .. -DCMAKE_CURRENT_SOURCE_DIR=<dir>```  
* Passing to compiler:  
    * ```-DCURRENT_PATH="${CMAKE_CURRENT_SOURCE_DIR}"``` as global macro ```CURRENT_PATH```  

#### 1.3 ```VERILATED_PATH```  
&emsp;&emsp;Specify the verilator output path of DUT.  
* Configuring:  
    * ```cmake .. -DVERILATED_PATH=<dir>```  
* Passing to compiler:  
    * ```-DVERILATED_PATH="${VERILATED_PATH}"``` as global macro ```VERILATED_PATH```  
    * ```-DVERILATED_PATH_TOKEN=${VERILATED_PATH}``` as global macro ```VERILATED_PATH_TOKEN```  

#### 1.4 ```VERILATOR_INCLUDE```  
&emsp;&emsp;Specify the verilator include path.
* Configuring:  
    * ```cmake .. -DVERILATOR_INCLUDE=<dir>```  
* Passing to compiler:  
    * ```-DVERILATOR_INCLUDE="${VERILATOR_INCLUDE}"``` as global macro ```VERILATOR_INCLUDE```  

### 2. PortGen  

#### 2.1 Dynamic PortGen (Experimental)
<img src="assets/README.portgen.dynamic.png" height=200 alt="TL-Test-New overall" align=right />

&emsp;&emsp;Choose to generate PortGen connection library on run-time (experimental function).  
* Configuring:
    * ```cmake .. -DTLTEST_PORTGEN_DYNAMIC=1```  

&emsp;&emsp;Dynamic PortGen mode is available only when the TL-Test Subsystem is delivered with all source code (including the verilator output of DUT), and is **disabled by default**.    
&emsp;&emsp;Dynamic PortGen enables you to change the TileLink port configuration at run-time through ```tltest.ini``` for **Verilator Host Mode**, without any extra coding.  
&emsp;&emsp;Dynamic PortGen requires accessible ```/tmp``` directory and working C++ compilers (support at least C++17) on verilator running environment.  
&emsp;  
&emsp;&emsp;***NOTICE**: When using Dynamic PortGen, the source code directory and verilator output directory on build must not be moved or removed.   

#### 2.2 Static PortGen
<img src="assets/README.portgen.static.png" height=200 alt="TL-Test-New overall" align=right />

&emsp;&emsp;Choose to generate PortGen connection library on build-time.
* Configuring:
    * ```cmake .. [-DTLTEST_PORTGEN_DYNAMIC=0]```  

&emsp;&emsp;Static PortGen enables you to change the TileLink port configuration at build-time through ```tltest.ini``` or ```portgen``` executable with parameters for **Verilator Host Mode**, without any extra coding.    
&emsp;&emsp;```VERILATED_PATH``` and ```VERILATOR_INCLUDE``` parameters above are required to be configured correctly.   

&emsp;&emsp;**By default**, the PortGen is under **static mode** as ```TLTEST_PORTGEN_DYNAMIC=0``` without specifying the parameter.

### 3. Build selection 

## Run as **Verilator Host**

## Load as **DPI Guest**

## Runtime Configuration


## Applications
&emsp;&emsp;Nothing here for now. 
