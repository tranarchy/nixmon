# nixmon

<p align="center">
  <img src="https://github.com/user-attachments/assets/3abfee13-cec5-436f-a5ac-ae91da753ab8">
</p>
<p align="center">Hardware monitoring tool for UNIX like systems written in C</p>

## About

nixmon is a hardware monitoring tool for your CPU, GPU, RAM and storage with an easy-to-read interface

It also tracks the maximum values reached by your components (e.g., GPU power, CPU temp), making it ideal to use during benchmarking, gaming or other heavy workloads


## Build-time dependencies
- C99 compliant compiler
- libdrm
- make

## Supported systems
- Linux
- OpenBSD
- FreeBSD
- NetBSD

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
