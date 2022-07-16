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