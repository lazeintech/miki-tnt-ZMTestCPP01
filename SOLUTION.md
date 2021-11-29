# Overview of algorithm
The complexity of my algorithm is somewhere between O(2n) - O(3n).  
I borrowed Macroblock term which is used in AVC video codec in my implementation to speed up the computation.  

There are 2 main loops over the image:
- First loop: mark all the non-black macroblock.  
- First loop: mark the boundary of object and save (top, left, bottom, right) macroblock as result.  

# Code features
1. Support multi macroblock size (1x1, 2x2, 4x4 and 8x8)  
1x1 option guarantee the accuracy of result.  
Other options enable the program to process faster, but the accuracy is reduced.
2. Visualize border box  
Enable program to output pictures frame with drawn white rectangle border box.

# Future improvement
To increase the accuracy when macroblock size is not 1x1.

# Benchmarking
This program is tested on 2 platform:  
1. WSL on Windows 10
```
CPU: Intel(R) Core(TM) i5-10400 CPU @ 2.90GHz
Compiler: g++ (Debian 8.3.0-6) 8.3.0
Build option: -O3
Time consumed:
MB1x1 -> ~ 39 ms
MB2x2 -> ~ 11 ms
MB4x4 -> ~  4 ms
MB8x8 -> ~  2 ms
```

2. MacOS 11.4 
```
CPU: Intel(R) Core(TM) i5-5257U CPU @ 2.70GHz
Compiler: clang-1205.0.22.11
Build option: -O3
Time consumed:
MB1x1 -> ~ 52 ms
MB2x2 -> ~ 15 ms
MB4x3 -> ~  5 ms
MB8x8 -> ~  2 ms
```