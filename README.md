# DWalker

[![Build](https://github.com/shmmsra/dwalker/actions/workflows/build.yml/badge.svg)](https://github.com/shmmsra/dwalker/actions/workflows/build.yml)
[![Release](https://github.com/shmmsra/dwalker/actions/workflows/release.yml/badge.svg)](https://github.com/shmmsra/dwalker/actions/workflows/release.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)
[![Contributors](https://img.shields.io/github/contributors/shmmsra/dwalker)](https://github.com/shmmsra/dwalker/graphs/contributors)
[![Stars](https://img.shields.io/github/stars/shmmsra/dwalker?style=social)](https://github.com/shmmsra/dwalker)

DWalker is a fast and efficient Dependency Walker tool for Windows executables (.exe) and dynamic libraries (.dll). It parses PE files, tracks and resolves their import dependencies recursively, and intelligently filters out duplicate API-set forwarding libraries natively to provide a clean, accurate dependency graph. 

Results can be output to standard output as a plain hierarchical text tree or strictly structured JSON for downstream tooling.

## Features

- **Accurate Resolution**: Traces `api-ms-win-*` and `ext-ms-*` virtual endpoints dynamically down to their underlying backing DLLs (e.g., `bisrv.dll`, `KERNELBASE.dll`).
- **Deduplication**: Automatically filters out redundant sibling imports ensuring the output tree is uniquely structured per depth level.
- **Machine Readable Output**: Support for JSON dependency trees out of the box natively using the `--json` flag.

## Quick Download and Install (Windows)

You can easily download and extract the latest available target executable dynamically for the current user via PowerShell:

```powershell
irm https://raw.githubusercontent.com/shmmsra/dwalker/main/scripts/download.ps1 | iex
```

## Usage

Execute the tool from an elevated command prompt or terminal, specifying the target PE file as the primary argument.

```cmd
:: Output as plain hierarchical text tree
dwalker.exe "C:\Windows\System32\cmd.exe"

:: Output as structured JSON mapping
dwalker.exe "C:\Windows\System32\cmd.exe" --json > output.json
```

## Build Steps

You can build the project manually from source using `CMake` and `Microsoft Visual Studio C++ Build Tools` directly on your host machine.

```batch
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

The resulting executable binary will be located inside the build directory at: `build\Release\dwalker\dwalker.exe`.

## Core Libraries & Acknowledgements

This tool bridges several powerful third-party libraries for core operation:
- [Process Hacker Native Libraries (PHLib)](https://github.com/processhacker/processhacker) - Powers advanced PE header loading, NT symbol lookups, and mapping behaviors.
- [Xerces-C++](https://xerces.apache.org/xerces-c/) - Embedded module for handling SxS Manifest parsing accurately.
