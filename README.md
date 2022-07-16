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

## Example

[samples/basic.rm](samples/basic.rm)
```rust
fn my_fn(argument, list){
	let var_inside_function : int = 123;
}

let declaration : string = "this is a global constant declaration";
let number : int = 123123;
```

`output` 
```llvm
; ModuleID = 'samples/basic.rm'
source_filename = "samples/basic.rm"

@declaration = global [39 x i8] c"\22this is a global constant declaration\22"

define i32 @main() {
entry:
  %number = alloca i32, align 4
  store i32 123123, i32* %number, align 4
  ret i32 0
}

declare i8 @printf(i8 %0)

define i8 @my_fn(i8 %0) {
entry:
  %var_inside_function = alloca i32, align 4
  store i32 123, i32* %var_inside_function, align 4
}
```
