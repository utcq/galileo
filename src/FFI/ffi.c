#include <FFI/ffi.h>
#include <stdint.h>
#include <stdlib.h>
#include <pool/log.h>
#include <string.h>

extern void ll_ffi_call(const uint64_t *args, void *fnptr);

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
  return dl;
}

void *FFI_resolve_fn(void *lib_handle, const char *fn_name) {
  return dlsym(lib_handle, fn_name);
}

void FFI_close_lib(void *lib_handle) {
  dlclose(lib_handle);
}

void __assert_primitive(char *type) {
  if (!type){
    // Variadic argument
    return;
  }
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
  
  const uint64_t arg_array[8] = {
    rdi, rsi, rdx, rcx, r8, r9, stack_arg_count, (uint64_t)stack_args
  };

  ll_ffi_call(arg_array, func_addr);
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