#ifndef FFI_H
#define FFI_H

#include <ast/scope.h>


void *FFI_load_lib(const char *lib);
void *FFI_resolve_fn(void *lib_handle, const char *fn_name);
void FFI_close_lib(void *lib_handle);
void FFI_translation_level(struct declaration_map_child *decl);

#endif