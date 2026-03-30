# ildzium-engine
>The engine is not functional and I'm resurrecting it. I have various versions of it, some in C++, C and in Ildz.

>“Toy engine” here means a personal project, not production-ready. Open sourcing or external contributions would only occur if a functional demo were achieved, likely years in the future.

## Overview
Ildzium Engine is a Windows-based toy game engine built on the Vulkan API, designed for maximum performance and aesthetic control. It emphasizes rapid iteration through extensive hot-reloading capabilities across core subsystems, including engine architecture, rendering pipelines, and shader programs. It trades general-purpose flexibility for a tightly integrated, constraint-based design that prioritizes efficiency and creative expression through computation and post-processing rather than raw scene complexity.

The engine targets both 2D and 3D game development, with a focus on highly efficient rendering workflows. It leverages modern Vulkan extensions to maximize GPU utilization, while maintaining a minimal and predictable runtime structure. Integrated systems include gameplay logic, animation, audio, and editor-driven material workflows. Shader compilation is automated, transforming GLSL into optimized SPIR-V binaries.

## Design Philosophy
Ildzium Engine enforces performance-oriented best practices while remaining adaptable. The goal is to reduce developer overhead in low-level optimization decisions without removing control.

The engine is intentionally restrictive in certain areas, favoring rigid, optimal patterns over general-purpose flexibility. This constraint-driven design encourages predictable performance and simplifies reasoning about system behavior.

Core principles include:

Maximizing GPU utilization over CPU-bound workflows
Favoring cache-friendly, contiguous data layouts
Eliminating unnecessary abstraction layers
Reducing feature scope to what directly supports the intended experience

Customization is limited to configuration of fixed, highly optimized data structures in the ildz core library (e.g., RenderGraph, UIGraph, AudioGraph, InputGraph). You cannot disable core subsystems such as input rebinding or audio channel behavior. Instead, developers choose setups within these systems—number of inputs, render passes, audio sources, UI elements, colors, meshes, shaders, and more—while the engine ensures optimal performance automatically.

## Rendering Philosophy

Ildzium prioritizes computational and artistic complexity over geometric or scene complexity. This means physics simulations, particle systems, procedural animation, post-processing effects, AI behavior, world simulation, and weather systems. These systems define your project’s complexity rather than the sheer number of objects rendered; there will be a focus on the off-screen "life" in your games, and in-built systems to regulate workload by ensuring there can never be too much processing happening, too much drawn to the screen, etc.

Rather than rendering large numbers of detailed objects (e.g., dense meshes, heavy lighting, decals), Ildzium focuses on:

- Simple, low-poly geometry
- Minimal scene composition
- Rich post-processing and stylized visual effects
- Numerically driven transformations and behaviors

This approach shifts complexity toward:

- GPU-based computation (compute shaders, procedural effects)
- Custom rendering techniques inspired by modern research
- Carefully designed pipelines that avoid unnecessary overhead

Certain features may be intentionally excluded or limited:

- Traditional real-time lighting models
- Expensive ray tracing techniques
- General-purpose rendering paths

Optimized workflows may include:

- Single large vertex buffer architectures
- Multi-draw indirect rendering with instancing
- Strict batching and draw-call minimization

## Graphics and GPU Systems

The rendering backend is built around explicit control of GPU resources and modern Vulkan techniques. Key areas include:

- Compute-driven pipelines for simulation and rendering
- Simplified, high-performance approach to ray tracing; final design TBD.
- Level of Detail (LOD) systems and mesh simplification (e.g., QEM-based techniques)
- Efficient batching and instanced rendering

Asset pipelines are designed for performance and compactness:

- Custom binary formats for meshes and textures
- Aggressive compression strategies to maximize FPS, reduce load times, and optimize streaming, focusing on runtime speed over file size alone
- Preprocessing steps to optimize runtime access patterns

## Data-Oriented Core

The engine builds on core Ildz libraries that provide optimized foundational data structures and memory systems. Ildz integrates the best of STL, custom allocators, and arena-based memory management into a single, high-performance core memory library, tailored specifically for the engine’s constrained architecture.

Key structures include:

- Circular buffers for streaming and transient data
- Bucket arrays for stable, cache-friendly storage
- Custom hash maps tuned for predictable performance

These structures are designed to:

- Minimize memory fragmentation
- Maximize cache locality
- Provide deterministic performance characteristics

The overlap between the core library and engine systems is intentional, ensuring that high-level features remain grounded in low-level efficiency.

## Development Workflow

A defining feature of the engine is its iterative workflow:

- Hot-reloading across engine subsystems
- Tight integration between engine, tooling, and game code
- Co-development of engine architecture, custom language (conlang), compiler/toolchain, and game logic. 

This unified approach allows design decisions to evolve dynamically, ensuring that the engine remains aligned with the specific needs of the project. Developers write in the Ildz language within the engine UI. Code transpiles to GLSL, SPIR-V, or other targets. Hotkeys trigger compilation and hot-reloading, enabling immediate iteration on gameplay or shaders without external editors.

>I initially implemented DLL swapping in C/C++, supporting renderer and shader hot-reload. The full core engine hot-reload was cumbersome, motivating the creation of a custom language and compiler. Current strategy involves invoking the Ildz compiler and applying runtime techniques to update subsystems as seamlessly as possible. Final implementation details are pending.

## Design philosophy
The engine enforces best practices for performance while remaining customizable, allowing developers to focus on gameplay and creative iteration without compromising efficiency.
proprietary hyperoptimized Vulkan engine tailor-made for complex games with a unique aesthetic, where the complexity is not in the rendering of a million small particles, advanced lights, decals, meshes and clutter, but in the numeric/computational behavior of objects and the artistic/original/creative visual effects, postprocessing etc laid on top of very simple and performant structures... leveraging GPU wherever possible, cache optimization, rigid, enforced optimal structures and patterns, and minimizing any unnecesasry features. i design my conlang, compiler, engine and game in tandem and make adjustments along the way to achieve the desired result (my desire also evolves). for example, i might force the user to only have one giant optimized vertex buffer on the GPU, enforce the most optimal vulkan multi-draw indirect instanced thing or whatever, decide to not support any kind of expensive ray tracing or lights, 
for the gpu and rendering side: custom ray tracing, compute shaders, QEM, LODs, drawing inspiration from all the advanced techniques, newest approaches in rendering etc, but always focus on simplicity, minimalism and lowpoly. texture and mesh pipelines, binary format, supercompression, 
optimized data structures, circular buffers, bucket arrays, hash maps, this builds on the core ildz library and is the point where these overlap. 

## Planned expansions
- Integration with the Ildz compiler, enabling transpilation from C to Ildz for a fully self-contained, high-performance development stack.
- In/engine visual node representation of files (.ildz, .glsl, .py, etc.) for even faster hot-reload iteration (no need for an external code editor)
- Backend for WebGPU/WebAssembly:
  - Browser event system (keyboard/mouse via JS)
  - Second renderer targeting WebGPU concepts
  - Rewrite shaders to WGSL
  - Bridge C to browser: Emscripten for compilation and JS interop?

## License & Usage
Copyright © 2024-2026 Christine Spades. All rights reserved.

This repository is proprietary software. Unauthorized copying, modification, redistribution, hosting, or commercial use is strictly prohibited.

No license is granted except by explicit written permission from the author.
