#ifndef FFI_H
#define FFI_H

#include <ast/scope.h>
#include <dlfcn.h>

void *FFI_load_lib(const char *lib);
void *FFI_resolve_fn(void *lib_handle, const char *fn_name);
void FFI_close_lib(void *lib_handle);
void FFI_translation_level(void *lib_handle, struct declaration_map_child *decl, int num_args, int64_t *args);

#endif