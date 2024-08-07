#include <FFI/ffi.h>
#include <stdint.h>
#include <stdlib.h>
#include <pool/log.h>
#include <dlfcn.h>
#include <string.h>

const char *PRIMITIVE_TYPES[] = {
  "int",
  "string",
  "float",
  "char",
  "void",
  "bool",
};

void *FFI_load_lib(const char *lib) {
  void *dl = dlopen(lib, RTLD_LAZY);
  if (!dl) {
    ERROR("FFI Error: %s\n", dlerror());
    exit(EXIT_FAILURE);
  }
  return dl;
}

void *FFI_resolve_fn(void *lib_handle, const char *fn_name) {
  return dlsym(lib_handle, fn_name);
}

void FFI_close_lib(void *lib_handle) {
  dlclose(lib_handle);
}

void __assert_primitive(char *type) {
  for (unsigned i=0; i < sizeof(PRIMITIVE_TYPES)/sizeof(PRIMITIVE_TYPES[0]); i++) {
    if (strcmp(type, PRIMITIVE_TYPES[i]) == 0) {
      return;
    }
  }
  ERROR("FFI Unsupported type %s\n", type);
  exit(EXIT_FAILURE);
}

void __ffi_call(void* func_addr, int num_args, int64_t *args) {
    uint64_t rdi, rsi, rdx, rcx, r8, r9;
    rdi = rsi = rdx = rcx = r8 = r9 = 0;

    uint64_t stack_args[10];
    int stack_arg_count = 0;

    for (int i = 0; i < num_args; i++) {
        switch (i) {
            case 0: rdi = args[i]; break;
            case 1: rsi = args[i]; break;
            case 2: rdx = args[i]; break;
            case 3: rcx = args[i]; break;
            case 4: r8 = args[i]; break;
            case 5: r9 = args[i]; break;
            default: stack_args[stack_arg_count++] = args[i]; break;
        }
    }

    __asm__ volatile(
        "movq %0, %%rdi\n"
        "movq %1, %%rsi\n"
        "movq %2, %%rdx\n"
        "movq %3, %%rcx\n"
        "movq %4, %%r8\n"
        "movq %5, %%r9\n"
        // Stack arguments
        "mov %6, %%eax\n"
        "1: test %%rax, %%rax\n"
        "jz 2f\n"
        "dec %%rax\n"
        "movq %7, %%r10\n"
        "lea (%%r10, %%rax, 8), %%r11\n"
        "pushq (%%r11)\n"
        "jmp 1b\n"
        "2:\n"
        "call *%8\n"
        :
        : "r"(rdi), "r"(rsi), "r"(rdx), "r"(rcx), "r"(r8), "r"(r9),
          "r"(stack_arg_count), "r"(stack_args), "r"(func_addr)
        : "rax", "r10", "r11", "memory"
    );
}

void FFI_translation_level(void *lib_handle, struct declaration_map_child *decl, int num_args, int64_t *args) {
  __assert_primitive(decl->value->data.fn_decl->type);
  struct function_parameter *arg = decl->value->data.fn_decl->parameters;
  while (arg) {
    __assert_primitive(arg->type);
    arg = arg->next;
  }
  //TODO: Use pointer if not primitive

  void *fnptr = FFI_resolve_fn(lib_handle, decl->key);
  if (!fnptr) {
    ERROR("FFI Function %s not found\n", decl->key);
    exit(EXIT_FAILURE);
  }
  
  __ffi_call(fnptr, num_args, args);
}