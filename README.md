# ASTViz

Visualize and grep through the AST nodes for your codebases.

![holy](./assets/ast.png)


## Build Instructions

```sh
# Where x.c is a random C source file.
clang -Xclang -ast-dump=json -fsyntax-only x.c > ast.json
```

> This projects requires you to download the [raylib](https://www.raylib.com/) library.

```sh
clang -o outfile plot.cpp -l{YOUR RAYLIB INSTALL PATH}
```
