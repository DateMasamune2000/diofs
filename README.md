# DioFS
- A small FUSE-based virtual filesystem example
- Emulates a full filesystem, instead of mirroring the host's filesystem

## Implemented methods
- `getattr`
- `readdir`
- `open`
- `read`

## Roadmap
1. Full file and directory support - In progress
2. Support for modifying files

## Dependencies
- `libfuse` version 2.9 must be installed
 - Arch Linux: `pacman -S fuse2`
 - Debian/Ubuntu: `apt install fuse`

## Build instructions
- Run `make clean` to remove object files
- Run `make all` to build `diofs`
