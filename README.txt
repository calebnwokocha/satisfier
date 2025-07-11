==========================
 Satisfier Library Setup
==========================

Purpose:
--------
Build the Satisfier shared library (DLL on Windows, .so on Linux) exposing only a clean C API plus a C++ chaining wrapper:
- C++ class sat::Logic with methods .And(), .Or(), .Not(), .value() for RAII and chaining

Files:
------
* satisfier.hpp  — Public API header
* satisfier.cpp  — Implementation
* README.txt     — This setup guide

Export Macro:
-------------
SATISFIER_API toggles symbol visibility:
- Windows: __declspec(dllexport/dllimport)
- Linux:   __attribute__((visibility("default"))) when BUILD_SATISFIER is set

Build with Code::Blocks (Windows & Linux):
------------------------------------------

1. Create Project
   - New → Project → Dynamic Linker Library
   - Name: satisfier, Location: project root

2. Add Sources
   - Add satisfier.hpp to Headers
   - Add satisfier.cpp to Sources

3. Define BUILD_SATISFIER
   - Project → Build options → #defines → add BUILD_SATISFIER

4. Platform Flags
   - Linux target:
     - Compiler settings → Other options: -fPIC
     - Linker settings → Other options: -shared
   - Windows target: no extra flags

5. Build
   - Press F9 or Build → Build
   - Artifacts:
     - Windows → satisfier.dll (+ import lib)
     - Linux   → libsatisfier.so

6. Test Client
   - Create a Console Application project
   - Include satisfier.hpp in main.cpp:
     ```cpp
     #include "satisfier.hpp"
     #include <stdio.h>
     
     int main() {
      suppose_literal(A, true);
      suppose_literal(Y, false);
      satisfy::Formula r = A.And(Y);
      printf("\nResult: %s\n", r.value() ? "true" : "false");
      return 0;
     }
     ```
   - In client build options:
     - Search directories for Compiler → path to header
     - Linker settings → add import lib or link against .so
     - Copy DLL/so to executable folder or set PATH/LD_LIBRARY_PATH
