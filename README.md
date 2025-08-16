# DWalker - Windows Dependency Walker

A modern, command-line dependency analysis tool for Windows PE files (executables and DLLs). DWalker analyzes the dependency tree of Windows binaries, following the same module resolution logic as the Windows NT Loader.

## Features

✅ **Complete dependency analysis** - Recursively analyzes all dependencies  
✅ **Windows loader emulation** - Follows NT Loader's search strategy  
✅ **Multiple search locations** - API Set, Known DLLs, System32, PATH, etc.  
✅ **Tree visualization** - Hierarchical dependency display with emojis  
✅ **Architecture detection** - Supports both 32-bit and 64-bit binaries  
✅ **Circular dependency detection** - Prevents infinite recursion  
✅ **Command-line interface** - Configurable depth and verbosity  
✅ **Cross-platform build** - CMake-based build system  

## Usage

```bash
# Basic analysis
dwalker.exe notepad.exe

# Verbose output with full paths and search strategies
dwalker.exe --verbose kernel32.dll

# Limit recursion depth
dwalker.exe --depth 3 --verbose C:\Windows\System32\ntdll.dll

# Show help
dwalker.exe --help
```

### Command Line Options

- `-h, --help` - Show help message
- `-v, --verbose` - Show detailed information (search strategy, architecture, full paths)
- `-d, --depth N` - Maximum recursion depth (default: 10, max: 100)

### Example Output

```
🔍 Analyzing dependencies for: notepad.exe
📁 Full path: C:\Windows\System32\notepad.exe

📦 notepad.exe
  📦 ntdll.dll
    📦 kernel32.dll
      📦 KERNELBASE.dll
      📦 msvcrt.dll
    📦 user32.dll
      📦 win32u.dll
      📦 gdi32.dll
        📦 gdi32full.dll
  📦 comctl32.dll
    📦 uxtheme.dll
  📦 shell32.dll
  ❌ [NOT_FOUND] some_missing.dll
```

## Build Instructions

### Prerequisites

- Windows 10/11
- Visual Studio 2019 or later with C++ support
- CMake 3.14 or later

### Build Steps

1. **Clone and setup:**
   ```cmd
   git clone <repository-url>
   cd dwalker
   mkdir build
   cd build
   ```

2. **Generate project:**
   ```cmd
   cmake ..
   ```

3. **Build:**
   ```cmd
   cmake --build . --config Release
   ```

4. **Run:**
   ```cmd
   .\Release\dwalker\dwalker.exe --help
   ```

## Architecture

DWalker consists of several key components:

- **`PEManager`** - PE file parsing and analysis using Process Hacker library
- **`BinaryCache`** - Module resolution and caching with proper hashing
- **`FindPE`** - Windows NT Loader path resolution emulation
- **`DWalker`** - Main orchestrator with tree visualization
- **`SXSManifest`** - Side-by-Side assembly processing (partial)
- **`ApiSet`** - Windows API Set schema handling

### Module Search Strategy

DWalker follows the Windows NT Loader's search order:

1. **SxS manifests** (partial support)
2. **Known DLLs** registry
3. **Application directory**
4. **System32/SysWOW64**
5. **Windows directory**
6. **Current working directory**
7. **PATH environment variable** ✅
8. **Custom search folders**

## Dependencies

- **Process Hacker Library (phlib)** - Low-level PE parsing
- **Xerces-C++** - XML manifest processing
- **Windows APIs** - System folder detection and registry access

## Current Status

🟢 **Core functionality complete** - Dependency analysis works  
🟡 **Partial SxS support** - Basic manifest processing  
🔴 **Missing features** - JSON/XML output, advanced SxS, GUI  

## Troubleshooting

**"Failed to initialize PHLib"** - Ensure you're running on Windows with proper permissions

**"File not found"** - Check file path and ensure the target PE file exists

**"❌ [NOT_FOUND]"** - Dependency couldn't be resolved using standard Windows search paths

## Contributing

1. Check existing TODO comments in the source code
2. Focus on completing SxS manifest processing
3. Add JSON/XML output formats
4. Improve error handling and reporting

## License

See [LICENSE](LICENSE) file for details.
