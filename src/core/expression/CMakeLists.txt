# EEL2 Expression Evaluator Library
# Platform-agnostic C library with optional x86 JIT

add_library(eel2 STATIC
  eel2/ns-eel-int.h
  eel2/ns-eel.h
  eel2/ns-eel-addfuncs.h
  eel2/nseel-compiler.c
  eel2/nseel-eval.c
  eel2/nseel-yylex.c
  eel2/nseel-caltab.c
  eel2/nseel-cfunc.c
  eel2/nseel-ram.c
)

# Optional: Add x86 JIT for better performance (Unix/GCC)
# This is optional - without it, EEL2 still works via interpreter
if(UNIX AND NOT APPLE)
  target_sources(eel2 PRIVATE eel2/asm-nseel-x86-gcc.c)
endif()

# EEL2 needs math library on Unix
if(UNIX)
  target_link_libraries(eel2 PRIVATE m)
endif()

# Public API
target_include_directories(eel2 PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/eel2
)

# Suppress warnings from C code compiled in C++
target_compile_options(eel2 PRIVATE
  -Wno-unused-variable
  -Wno-unused-function
)
