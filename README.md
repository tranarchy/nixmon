# nixmon

<p>
  <img src="https://github.com/user-attachments/assets/7b7e3ae0-0cbd-4162-9596-13a88f1edc1d">
</p>
<p align="center">Hardware monitoring tool for UNIX like systems written in C</p>

## About

nixmon is a hardware monitoring tool for your CPU, GPU, RAM and storage with an easy-to-read interface

It also tracks the maximum values reached by your components (e.g., GPU power, CPU temp), making it ideal to use during benchmarking, gaming or other heavy workloads

## Supported systems
- Linux
- macOS*
- FreeBSD
- OpenBSD
- NetBSD

*No GPU info for macOS

## Build-time dependencies
- C99 compliant compiler
- libdrm (on Linux and *BSD)
- make

## Installing

```
git clone https://github.com/tranarchy/nixmon
cd nixmon
make
make install
```

`make` defaults to Linux, use the appropriate make command for your system `make fbsd / obsd / nbsd / macos`

## FreeBSD

To get CPU temp info on FreeBSD you need to load either coretemp (for Intel) or amdtemp (for AMD).

```
kldload coretemp amdtemp
```
