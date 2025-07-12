# Satisfier API + Code::Blocks Integration Tutorial

This tutorial shows how to set up a client project in Code::Blocks to link with the **Satisfier** API provided via:

- `satisfier.hpp` — Header definitions  
- `satisfier.dll` — Windows dynamic library (runtime linkage)  
- `libsatisfier.a` — Static library (for static linking on MinGW/GCC)

Client code example: `client.cpp`

---

## Prerequisites

- **Code::Blocks** (bundled with MinGW on Windows) :contentReference[oaicite:1]{index=1}  
- Your four files placed in a common directory

---

## 🛠 Option (A): Using DLL (Dynamic Linking)

1. **Create a new Code::Blocks project**  
   - *File → New → Project → Console Application → C++*

2. **Copy files**  
   - Paste `client.cpp` and `satisfier.hpp` into the project folder.

3. **Link the DLL at runtime**  
   - Place `satisfier.dll` next to the generated `.exe` after building.

4. **Configure build options**  
   - Open *Project → Build options → Linker settings*  
   - Under **Link libraries**, add the import library (e.g. `libsatisfier.dll.a` or `.lib`) if available; else,
   - Rely on dynamic loading (e.g., `LoadLibrary`, `GetProcAddress`) in your code.

5. **Include headers**  
   - Ensure in *Search directories → Compiler* you have the path to `satisfier.hpp`.
