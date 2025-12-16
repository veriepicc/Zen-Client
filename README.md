<p align="center">
  <img src="https://img.shields.io/badge/C%2B%2B-23-blue?style=for-the-badge&logo=cplusplus&logoColor=white" alt="C++23"/>
  <img src="https://img.shields.io/badge/Platform-Windows%2010%2F11-0078D6?style=for-the-badge&logo=windows&logoColor=white" alt="Windows"/>
  <img src="https://img.shields.io/badge/Target-Minecraft%20Bedrock-62B47A?style=for-the-badge&logo=minecraft&logoColor=white" alt="Minecraft Bedrock"/>
  <img src="https://img.shields.io/badge/Architecture-x64-red?style=for-the-badge" alt="x64"/>
</p>

<h1 align="center">⚡ Zen Client v3</h1>

<p align="center">
  <strong>A next-generation Minecraft Bedrock Edition utility client written in modern C++23.</strong>
  <br/>
  <em>Engineered for performance, built with zero external dependencies (except ImGui).</em>
</p>

---

## 🎯 Overview

**Zen Client** is a high-performance, fully modular hack client for Minecraft Bedrock Edition (Windows 10/11). Built from the ground up using **C++20/23 modules (`.ixx`)**, Zen achieves both blazing-fast compilation and runtime performance while maintaining a clean, extensible codebase.

Unlike other clients that pull in dozens of external libraries, Zen relies on **in-house developed tools** for core functionality — signature scanning, hooking, math, and rendering are all custom-built by the Zen Development team.

---

## ✨ Key Features

### 🏗️ Modern Architecture
- **C++23 Standard** — Leverages the latest C++ features including modules, concepts, and constexpr improvements
- **Module-Based Design** — Every component is a self-contained `.ixx` module for clean separation and fast incremental builds
- **Zero External Dependencies** — All core libraries (Zenova, Jonathan, Paul, BigRat) are developed in-house

### ⚡ Performance First
- **SIMD-Accelerated Scanning** — AVX2/SSE4.1 optimized signature scanning with compile-time pattern parsing
- **Minimal Hooking Overhead** — Custom trampoline-based hooking with automatic instruction relocation
- **Async Initialization** — Multi-threaded startup with detailed timing diagnostics
- **Link-Time Optimization** — Full LTO enabled for release builds

### 🎮 Module System
Categorized, toggleable modules with configurable settings and keybinds

### 🖥️ Rendering
- **BigRat** — Custom ImGui backend rendering directly through Minecraft's Tessellator
- Native texture loading via Windows Imaging Component (WIC)
- Hardware-accelerated UI with proper scissor clipping

---

## 🛠️ In-House Libraries

Zen Client is built on a foundation of custom libraries developed specifically for this project:

### 🔍 Zenova — Ultra-Optimized Signature Scanner
```cpp
// Compile-time signature parsing with zero runtime overhead
constexpr auto sig = ZEN_SIG("48 8B 05 ? ? ? ? 48 85 C0 74 ? 48 8B 00");
auto result = Zenova::find(sig, moduleBase, moduleSize);
```
- Hand-rolled AVX2/SSE4.1 SIMD implementations
- Compile-time signature parsing into optimized bytecode
- Automatic CPU feature detection with graceful fallbacks
- First-byte optimization for rapid candidate filtering

### 🪝 Jonathan — Tiny Hooking Library
```cpp
// Simple, reliable function hooking
Jonathan::create(target, detour, &original);
Jonathan::enable(target);
```
- 5-byte relative jumps when possible, absolute jumps otherwise
- Automatic instruction-length decoding for safe trampolines
- Thread-safe hook management with atomic operations
- Support for both x86 and x64 architectures

### 📐 Paul — Modern Math Library
```cpp
// Clean, constexpr-first math API
using namespace Paul;
Vec3f position = Vec3f::Zero();
Mat4 transform = Mat4::Identity();
Quat rotation = Quat::FromEuler(pitch, yaw, roll);
```
- Vector types: `Vec2`, `Vec3`, `Vec4`
- Matrix operations: `Mat3`, `Mat4`
- Quaternions with full interpolation support
- Geometric primitives: `Rect`, `AABB`, `BlockPos`
- All operations are `constexpr` where possible

### 🐀 BigRat — ImGui Bedrock Backend
Custom ImGui rendering implementation that interfaces directly with Minecraft's rendering pipeline:
- Renders through the native `Tessellator` for zero additional draw calls
- Proper texture management via `TextureGroup`
- Native scissor/clipping support
- Font atlas baked to Bedrock texture format

---

## 📁 Project Structure

```
Zen Client/
├── Client/
│   ├── Audio/              # Sound playback (XAudio2)
│   ├── Command/            # Chat command system
│   ├── Memory/             # Signatures, offsets, and hooks
│   │   └── Hooks/          # Function hooks organized by category
│   ├── Module/             # Cheat modules by category
│   │   ├── Combat/
│   │   ├── Misc/
│   │   ├── Movement/
│   │   ├── Player/
│   │   ├── Visual/
│   │   └── World/
│   ├── SDK/                # Minecraft class reconstructions
│   │   ├── Block/
│   │   ├── Network/
│   │   ├── Render/
│   │   └── Tesselator/
│   ├── Utils/              # Utilities (Paul math, Keys, etc.)
│   ├── Entry.ixx           # DLL entry point
│   └── Zen.ixx             # Main client initialization
├── Include/
│   ├── entt/               # EnTT (ECS, used internally)
│   ├── imgui/              # Dear ImGui + BigRat backend
│   ├── Jonathan.hpp        # Hooking library
│   └── Zenova.ixx          # Signature scanner
├── Output/                 # Build artifacts
├── Zen.vcxproj             # Visual Studio project
└── README.md
```

---

## 🔧 Build Requirements

| Requirement | Version |
|-------------|---------|
| **Visual Studio** | 2022 (v143 toolset) |
| **Windows SDK** | 10.0+ |
| **C++ Standard** | C++23 (Debug) / C++23 (Release) |
| **Platform** | x64 |

### Building

1. Open `Zen Client.sln` in Visual Studio 2022
2. Select configuration (`Debug` or `Release`) and platform (`x64`)
3. Build → Build Solution (`Ctrl+Shift+B`)
4. Output DLL: `Output/<Config>/x64/Zen.dll`

---

## 💉 Injection

Zen compiles to a standard Windows DLL. Use your preferred injector to load `Zen.dll` into `Minecraft.Windows.exe`.

---

## 🎹 Default Keybinds

| Key | Action |
|-----|--------|
| `INSERT` | Open ClickGUI |
| `END` | Uninject |

Module keybinds can be configured through the ClickGUI.

---

## 📜 License

This project is released under the **DogBeater License v1.0** — see [LICENSE](LICENSE) for the full (and highly entertaining) terms.

**TL;DR:** Look, don't touch (unless you're contributing to the official repo).

---

## 👥 Credits

**Zen Development Team**
- Architecture, core libraries (Zenova, Jonathan, Paul, BigRat)
- SDK reverse engineering
- Module development

**Third-Party**
- [Dear ImGui](https://github.com/ocornut/imgui) — Immediate mode GUI
- [EnTT](https://github.com/skypjack/entt) — Entity Component System (internal use)

---

<p align="center">
  <sub>Built with 💜 and way too much caffeine by the Zen Development team.</sub>
</p>
