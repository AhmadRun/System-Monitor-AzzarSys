# SysMon — Linux System Monitor

A real-time terminal-based system monitor for Linux written in C++17 with zero external dependencies.

## Build

```bash
cd sysmon
cmake -S . -B build
cmake --build build
```

## Run

```bash
./build/sysmon
```

Press `q` to quit.

## What It Shows

- **CPU** — per-core usage with delta-based calculation, load averages, ASCII bar graphs
- **Memory** — RAM usage percentage and absolute values
- **Disks** — physical mount points with usage and size
- **Network** — per-interface RX/TX traffic (auto-scaled B/KB/MB/GB)
- **Processes** — top 10 by CPU with PID, name, CPU%, and memory
- **System Info** — hostname, uptime, core count

## Source Files

| File | Purpose |
|---|---|
| `src/main.cpp` | Entry point |
| `src/cpu_monitor.cpp` | CPU stats from `/proc/stat` and `/proc/loadavg` |
| `src/memory_monitor.cpp` | Memory stats from `/proc/meminfo` |
| `src/disk_monitor.cpp` | Disk usage via `/proc/mounts` + `statvfs()` |
| `src/process_monitor.cpp` | Process list from `/proc/[pid]/` |
| `src/network_monitor.cpp` | Network stats from `/proc/net/dev` |
| `src/system_info.cpp` | Hostname, uptime, kernel version |
| `src/ui.cpp` | ANSI terminal UI with raw-mode input |

## Install (optional)

```bash
cmake --install build
```
