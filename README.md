# ASTViz

Visualize and grep through the AST nodes for your codebases.

![holy](./assets/ast.png)


## Build Instructions

1. Clone this repo

2. Run the following commands in the project folder
```sh
mkdir build && cd build
cmake ../
make
```

# Usage

> It is advised to have `clang` on your system for generating the AST.

```sh
# Where x.c is a random C source file.
clang -Xclang -ast-dump=json -fsyntax-only x.c > ast.json
```


```sh
./plot <path-to-json-file>
```
