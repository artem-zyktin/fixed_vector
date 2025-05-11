# fixed_vector

**fixed_vector** is a minimal, header-only C++ container that behaves similarly to `std::vector`, but with one major difference:  
it almost never reallocates memory once constructed.  

This container is optimized for predictable memory behavior, cache-friendly iteration, and low-level systems such as game engines or ECS frameworks.

## Features

- Contiguous memory layout
- Almost no dynamic reallocation after construction
- Deterministic performance characteristics
- Simple interface, inspired by `std::vector`
- Custom allocator support
- Fully templated and dependency-free

## Motivation

In performance-critical applications such as game engines, memory reallocation during runtime can cause frame drops, fragmentation, and cache inefficiencies.  
**fixed_vector** was designed to avoid these issues by enforcing a fixed memory footprint after initialization.

## Important notice

In case of removing items with `index != size()-1` the last item and `fvec[index]` will be swapped and the last item (`fvec[index]` now) will be remove. That means that index of items can't be fixated.
