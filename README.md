# ildzium-engine
>Very rough document; the engine is not currently functional or being worked on. I have various versions of it, some in C++, C and in Ildz, but it's been a while since I've looked at it. I'll get to it eventually.

Ildzium Engine is a toy game engine for Windows built on Vulkan, designed for maximum performance, flexibility, and aesthetic control. The engine features hot-reloading of its core components—including architecture, renderer, shaders, and more—enabling rapid iteration during development.

It is optimized for creating both 2D and 3D games with high-performance rendering pipelines, employing the latest Vulkan extensions to the max. Gameplay systems, animation, audio, and in-editor material workflows are fully integrated, with automation tools for shader compilation from GLSL to optimized SPIR-V.

The engine enforces best practices for performance while remaining customizable, allowing developers to focus on gameplay and creative iteration without compromising efficiency.

## Planned
- Integration with the Ildz compiler, enabling transpilation from C to Ildz for a fully self-contained, high-performance development stack.
- Visual node representation of files (.ildz, .glsl, .py, etc.) for even faster hot-reload iteration (no need for an external code editor)
