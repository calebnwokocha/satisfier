# 🧩 Satisfier API Integration with Code::Blocks

A step-by-step tutorial demonstrating how to integrate the Satisfier API (`client.cpp`, `satisfier.hpp`, `satisfier.dll`, `libsatisfier.a`) into a Code::Blocks C++ project, using dynamic (DLL) linking.

---

## 🛠 Prerequisites

- **Operating System**: Windows with Code::Blocks + MinGW  
- **Files** required in your project folder:
  - `client.cpp`
  - `satisfier.hpp`
  - `satisfier.dll`—find it in `bin/Debug`
  - `libsatisfier.a`—find it in `bin/Debug`
---

## 🏗️ Step-by-Step Linking

1. **Create Console Project**  
   `File → New → Project → Console Application (C++)`

2. **Add files**  
   Copy `client.cpp`, `satisfier.hpp` into the project directory.

3. **Linker Settings**  
   - *Project → Build options → Linker settings*  
   - Add import library `libsatisfier.a`  

5. **Copy .dll**  
   - Comment code in main function of `client.cpp`
   - Build your project to compile `.exe`
   - Copy `satisfier.dll` next to the compiled `.exe`
   - Uncomment code in main function of `client.cpp`

6. **Build & Run**  
   - Press **F9**—the project dynamically loads the DLL at runtime.

---
