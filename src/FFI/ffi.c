#include <FFI/ffi.h>
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
  return dlopen(lib, RTLD_LAZY);
}

void *FFI_resolve_fn(void *lib_handle, const char *fn_name) {
  return dlsym(lib_handle, fn_name);
}

void FFI_close_lib(void *lib_handle) {
  dlclose(lib_handle);
}

void __assert_primitive(char *type) {
  for (int i=0; i < sizeof(PRIMITIVE_TYPES)/sizeof(PRIMITIVE_TYPES[0]); i++) {
    if (strcmp(type, PRIMITIVE_TYPES[i]) == 0) {
      return;
    }
  }
  ERROR("FFI Unsupported type %s\n", type);
  exit(EXIT_FAILURE);
}

void FFI_translation_level(struct declaration_map_child *decl) {
  __assert_primitive(decl->value->data.fn_decl->type);
  struct function_parameter *arg = decl->value->data.fn_decl->parameters;
  while (arg) {
    __assert_primitive(arg->type);
    arg = arg->next;
  }
  //TODO: Use pointer if not primitive

  // runtmie typedef
}