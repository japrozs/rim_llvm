# Rim `llvm`

Rim is a tiny compiler that I've been working on for the past 2 weeks. Right now it just compiles the code to `llir` but I'm need to add a lot of stuff to it.

## Usage

```bash
# make sure you have llvm installed
$ git clone https://github.com/japrozs/rim_llvm.git
$ cd rim_llvm
$ make -j$(nproc)
$ ./out/rimc samples/basic.rm
```
