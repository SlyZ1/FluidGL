<img src="public/FluidGL_Circle.png" width="100" alt="FluidGL logo">

# FluidGL

Real-time 3D liquid and smoke simulation, written from scratch in C++ and OpenGL. The whole simulation pipeline runs on the GPU.

## Features

- [x] **PIC/FLIP solver** with RK2 particle advection

- [x] **Pressure projection** with a hand-written MGPCG
- [x] Particles are **sorted every frame** with a counting sort built on a parallel prefix sum, so neighbour lookup is faster for P2G (particle to grid), particle separation and diffusion.
- [x] **Liquids**: Collisions against SDF
- [x] **Smoke**: Temperature and density carried by the particles, buoyancy, cooling, vorticity confinement, and diffusion between particles
- [x] **Open or closed** domain boundaries
- [x] **Live editing** parameters with ImGui
- [x] **Profiling**: a GPU timer per stage (sort, P2G, etc) is displayed in the app.

## Build and run

You need CMake 3.30+, a C++20 compiler, OpenMP, and a GPU and driver with OpenGL 4.3+.

```bash
git clone https://github.com/SlyZ1/fluidgl.git
cd fluidgl
cmake -B build
cmake --build build -j
./myprogram
```

On Linux, `./start.sh` rebuilds and runs through `prime-run`, which forces the discrete GPU on hybrid NVIDIA laptops. On a single-GPU machine, run `./myprogram` directly.

## Benchmarks

Everything was benchmarked on a Laptop GPU RTX 4060.

### Suzanne Water Close Up

https://github.com/user-attachments/assets/ec980886-636d-498e-8d64-2c9ef0f155e0

| Stat | Value |
| ----------- | ----------- |
| Particle number | 876 K |
| Cell number | 973 K |
| GPU Storage | 162 MB |
| FPS | 60 |
| Simulation time | 14 ms |

### Big Water

https://github.com/user-attachments/assets/59a388d3-5023-4894-96d9-a606be26afdd

| Stat | Value |
| ----------- | ----------- |
| Particle number | 2.72 billion |
| Cell number | 5.62 billion |
| GPU Storage | 762 MB |
| FPS | 14 |
| Simulation time | 69 ms |

### Simple Smoke Plume 

https://github.com/user-attachments/assets/4fe21d96-c868-499e-9ae7-c9c0ac28421e

| Stat | Value |
| ----------- | ----------- |
| Particle number | 1.85 billion |
| Cell number | 539 K |
| GPU Storage | 244 MB |
| FPS | 41 |
| Simulation time | 24 ms |

### High Res Smoke Plume

https://github.com/user-attachments/assets/489b9e86-754a-45d7-8ae8-ae05fca0c769

| Stat | Value |
| ----------- | ----------- |
| Particle number | 8.19 billion |
| Cell number | 2.43 billion |
| GPU Storage | 1 GB |
| FPS | 10 |
| Simulation time | 99 ms |

## Built with

[GLFW](https://www.glfw.org/), [glad](https://glad.dav1d.de/), [GLM](https://github.com/g-truc/glm), [Dear ImGui](https://github.com/ocornut/imgui) and [nativefiledialog-extended](https://github.com/btzy/nativefiledialog-extended). Licensed under Apache 2.0, see [LICENSE](LICENSE).
