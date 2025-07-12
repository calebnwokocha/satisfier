# Satisfier API – Code::Blocks Tutorial

This guide walks you through setting up a Code::Blocks project using the Satisfier API.

## 📁 Project Files

- `client.cpp` – example client code that calls the Satisfier API.  
- `satisfier.hpp` – C++ header declaring the API interface.  
- `satisfier.dll` – Windows dynamic library.  
- `libsatisfier.a` – Unix/Linux static library.  

## 1. Create a New Code::Blocks Project

1. Open *Code::Blocks* → **File** → **New** → **Project**.
2. Choose **Console Application** → **C++** → click **Go**.
3. Name it (e.g. `SatisfierTest`) and select a folder.
4. Use default compiler settings; click **Finish**.

## 2. Add Source & Header

- Copy `client.cpp` and `satisfier.hpp` into the project folder.
- In Code::Blocks, right-click **Sources** → **Add files...** → select `client.cpp`.
- Repeat under **Headers** → select `satisfier.hpp`.

## 3. Configure Build Settings

Right-click project → **Build Options…**  
Select your target (e.g. Debug/Release or both).

### a) Include Directories

Under **Search directories** → **Compiler**, add the project folder (where `satisfier.hpp` lives).

### b) Linker Setup

Under **Search directories** → **Linker**, add path to:

- `satisfier.dll` (on Windows)  
- or `libsatisfier.a` (on Linux)

Then go to the **Linker settings** tab and add:

- On **Windows**:  
  - Library filename: `satisfier`  *(Code::Blocks will find `satisfier.dll`/`.lib`)*  
- On **Linux**:  
  - Library filename: `satisfier`  *(will link `libsatisfier.a`)*  

> Tip: Do **not** include `lib` prefix or file extension—just `satisfier`.

### c) Additional Compiler Flags (if needed)

If your API requires specific flags (e.g., `-std=c++11`), add those under **Compiler settings** → **Other options**.

## 4. Provide the `.dll` at Runtime (Windows Only)

Windows needs the DLL beside the `.exe` or in your `PATH`.

- After building, locate your executable in `bin/Debug/` (or `bin/Release/`).
- Copy `satisfier.dll` into that same folder.

Linux static library `.a` requires no runtime copy.

## 5. Build & Run

Click **Build** → **Build**, then **Build** → **Run**.

If everything is configured correctly, your console should show the output defined in `client.cpp`—e.g., “Connected to Satisfier version 1.0”.

## 6. Troubleshooting

- **Cannot find `satisfier.hpp`**  
  → Check your **Compiler search directories** include the correct path.

- **Linker errors: undefined references**  
  → Ensure `-lsatisfier` is added under Linker settings and Search directories link to the right `.a` or `.lib`.

- **Runtime error (Windows): `The program can’t start because satisfier.dll is missing`**  
  → Copy `satisfier.dll` to the same folder as your `.exe`.

- **Linux: “cannot find -lsatisfier”**  
  → Confirm `libsatisfier.a` is in a directory listed under Linker search paths (Project → Build Options).

---
