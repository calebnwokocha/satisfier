# 🧩 Satisfier API Integration with Code::Blocks

A step-by-step tutorial demonstrating how to integrate the Satisfier API (`client.cpp`, `satisfier.hpp`, `satisfier.dll`, `libsatisfier.a`) into a Code::Blocks C++ project, using either dynamic (DLL) or static linking.

---

## 📋 Table of Contents

1. [Prerequisites](#prerequisites)  
2. [Setup](#setup)  
   - [🛠 Dynamic Linking (DLL)](#dynamic-linking-dll)  
   - [🧩 Static Linking (.a)](#static-linking-a)  
3. [Example `client.cpp`](#example-clientcpp)  
4. [📊 Comparison](#comparison)  
5. [Tips & Best Practices](#tips--best-practices)

---

## Prerequisites

- **Operating System**: Windows with Code::Blocks + MinGW  
- **Files** required in your project folder:
  - `client.cpp`
  - `satisfier.hpp`
  - `satisfier.dll`
  - `libsatisfier.a`

---

## Setup

### Dynamic Linking (DLL)

1. **Create Console Project**  
   `File → New → Project → Console Application (C++)`

2. **Add files**  
   Copy `client.cpp`, `satisfier.hpp` into the project directory.

3. **Linker Settings**  
   - *Project → Build options → Linker settings*  
   - If available, add import library (e.g. `.dll.a` or `.lib`).  
   - Otherwise, code must use `LoadLibrary()` / `GetProcAddress()` for runtime loading.

4. **Search Directories**  
   - *Compiler → Search directories* → Ensure the directory containing `satisfier.hpp` is included.

5. **Post-build Step**  
   - Copy `satisfier.dll` next to the compiled `.exe`.

6. **Build & Run**  
   - Press **F9**—the project dynamically loads the DLL at runtime.

---

### Static Linking (.a)

1. **Create Console Project** (same as above)

2. **Include source** files in project

3. **Project → Build options → Linker settings**  
   - Add `libsatisfier.a`

4. **Compiler Flags**  
   - Ensure support for C++11 or newer (e.g. add `-std=c++11`)

5. **Build & Run**  
   - Produces a self-contained `.exe`, no DLL required at runtime.

---
