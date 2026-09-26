
<img src="public/FluidGL_Circle.png" width=100>

# FluidGL

> [!WARNING]
> The project is in a very early state, please be understanding for all the missing features.

## Overview

GPU fluid simulation, built from scratch in C++ and OpenGL compute shaders. Implements a PIC/FLIP solver running entirely on the GPU, with a custom conjugate gradient solver for the pressure projection step.

### App showcase

**PIC-FLIP Simulation**


https://github.com/user-attachments/assets/7273a69d-89ad-41b8-8772-fd34758d17e1



600.000 particles on Laptop RTX 4060 ~ 60fps

### Build

```bash
git clone https://github.com/SlyZ1/Fluid-Sim.git
cd Fluid-Sim
cmake -B build
cmake --build build
```

The executable (`myprogram`) is placed at the root of the project.

Convenience scripts are included for Linux:

```bash
./resetBuild.sh   # removes build/, reconfigures from scratch with default arguments
./start.sh        # builds (tests disabled) and runs the program
./test.sh         # builds (tests enabled) and runs the test suite via ctest
```

## Building

### Prerequisites

- CMake 3.30+
- A C++17 compiler (MSVC, GCC, or Clang)
- A GPU and driver supporting OpenGL 4.3+ (compute shaders)
- OpenMP support

Dependencies (GLFW, nativefiledialog-extended, and Catch2 for tests) are fetched automatically via CMake's `FetchContent`

### Run

```bash
./myprogram
```

of with the following command to recompile and run if you are on Linux

```bash
./start.sh
```

`start.sh` runs the program through [`prime-run`](https://wiki.archlinux.org/title/PRIME). If you're on a laptop with hybrid NVIDIA/integrated graphics, this forces execution on the discrete GPU, which compute shaders require for reasonable performance. If you don't have `prime-run` installed or don't need it (desktop GPU, or a single-GPU laptop), run `./myprogram` directly instead.

### Tests

Unit tests (Catch2) are really basic. Only the GPU matrix/vector operations and the conjugate gradient solver are tested. Enable them at configure time:

```bash
cmake -B build -DBUILD_TEST=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

or with the following command if you are on Linux

```bash
./test.sh
```

## Tech stack

| Library | Purpose |
|---|---|
| [GLFW](https://www.glfw.org/) | Window and input handling |
| [glad](https://glad.dav1d.de/) | OpenGL function loading |
| [GLM](https://github.com/g-truc/glm) | Math (vectors, matrices) |
| [Dear ImGui](https://github.com/ocornut/imgui) | Debug/control UI |
| [nativefiledialog-extended](https://github.com/btzy/nativefiledialog-extended) | Native file dialogs |
| [Catch2](https://github.com/catchorg/Catch2) | Unit testing |
| OpenMP | CPU-side parallelism |