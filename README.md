# nixmon

<p>
  <img src="https://github.com/user-attachments/assets/3a1ac18e-aa5f-4268-8f6b-6288731ad31f">
</p>
<p align="center">Hardware monitoring tool for UNIX like systems written in C</p>

## About

nixmon is a hardware monitoring tool for your CPU, GPU, RAM and storage with an easy-to-read interface

It also tracks the maximum values reached by your components (e.g., GPU power, CPU temp), making it ideal to use during benchmarking, gaming or other heavy workloads


## Build-time dependencies
- C99 compliant compiler
- libdrm
- make


## Installing

```
git clone https://github.com/tranarchy/nixmon
cd nixmon
make
make install
```

## FreeBSD

To get CPU temp info on FreeBSD you need to load either coretemp (for Intel) or amdtemp (for AMD).

```
kldload coretemp amdtemp
```
