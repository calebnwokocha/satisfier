# 🧩 Satisfier API – Static Linking with Code::Blocks

Integrate the Satisfier API (`client.cpp`, `satisfier.hpp`, `libsatisfier.a`) into a Code::Blocks C++ Console project using **static linking**.

---

## ✅ Prerequisites

- **Windows** with Code::Blocks + MinGW  
- Project folder contains:
  - `client.cpp`
  - `satisfier.hpp`
  - `libsatisfier.a` (static library)

---

## 🏗️ Step-by-Step: Static Linking

1. **Create Project**  
   - `File → New → Project → Console Application (C++)`

2. **Add Your Source**  
   - Copy `client.cpp` and `satisfier.hpp` into the project directory.

3. **Configure Compiler Search Path**  
   - `Settings → Compiler → Global compiler settings → Search directories → Compiler`  
   - Add path containing `satisfier.hpp` (LearnCpp walkthrough) :contentReference[oaicite:1]{index=1}

4. **Link Static Library**  
   - Right-click project → **Build options → Linker settings**  
   - Click **Add** under “Link libraries”, then select `libsatisfier.a` :contentReference[oaicite:2]{index=2}

5. **Enable C++11 Standard**  
   - `Settings → Compiler → Global compiler settings → Compiler flags` → check `-std=c++11`

6. **Build and Run**  
   - Press **F9**. The executable will include Satisfier functions statically—no DLL required.

---
