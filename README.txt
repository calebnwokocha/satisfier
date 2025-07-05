==========================
 Satisfier Library Setup
==========================

Purpose:
--------
Build the Satisfier shared library with Code::Blocks on Windows (DLL) and Linux (.so):
- Exposes only Logic_new, Logic_delete, Logic_and, Logic_or, Logic_not, Logic_value
- Single codebase, platform-specific export macros

Files:
------
* satisfier.hpp
* satisfier.cpp
* README.txt  (this file)

Platform-specific Export Macro:
--------------------------------
Using SATISFIER_API, defined in satisfier.hpp:
- On Windows: __declspec(dllexport/dllimport)
- On Linux:   __attribute__((visibility("default"))) when building; else empty

Build Instructions:
-------------------

1) Start Code::Blocks

2) Create Project:
   File → New → Project → Dynamic Linker Library → Go
   Name: satisfier
   Folder: project root
   Compiler: choose appropriate

3) Add files:
   - Headers → Add satisfier.hpp
   - Sources → Add satisfier.cpp

4) Set build defines:
   click Project → Build options…
   Under “#defines”, add:
     BUILD_SATISFIER

5) Linux target flags:
   Select Linux build target (or global if only one)
   - Compiler settings → Other compiler options:
         -fPIC
   - Linker settings → Other linker options:
         -shared

6) Windows target:
   No extra flags needed—DLL export macro managed via BUILD_SATISFIER

7) Build:
   Build → Build or press F9
   Output:
   - Windows → satisfier.dll (+ import .lib/.a)
   - Linux   → libsatisfier.so

8) Test client:
   Create separate Console Application project
   In test `main.c`:
     ```
     #include "satisfier.hpp"
     #include <stdio.h>

     int main(void) {
         Logic* t = Logic_new(true);
         Logic* f = Logic_new(false);
         Logic* r = Logic_and(t, f);
         printf("true AND false = %d\n", Logic_value(r));
         Logic_delete(t);
         Logic_delete(f);
         Logic_delete(r);
         return 0;
     }
     ```
   In test project's build options:
   - Add library search path and link with satisfier DLL/.so

Outcome:
--------
- Windows: `satisfier.dll` for use in Windows apps
- Linux:   `libsatisfier.so` for use on Linux
- Clean, portable C API

=== End of README.txt ===
