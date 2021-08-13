#ifndef LIBZAZEN_UTIL_H
#define LIBZAZEN_UTIL_H

#include <stdlib.h>
#include <wayland-server.h>

#define UNUSED(x) ((void)x)

#define _Nullable
#define _NonNull

/* helper function */
inline void* zalloc(size_t size) { return calloc(1, size); }

/* log */
void zazen_log(const char* fmt, ...);

/* weak ref */
struct zazen_weak_ref;

typedef void (*zazen_weak_ref_destroy_func_t)(struct zazen_weak_ref* ref);

struct zazen_weak_ref {
  void* data;  // NULLABLE
  struct wl_listener destroy_listener;
  zazen_weak_ref_destroy_func_t destroy_func;
};

void zazen_weak_ref_init(struct zazen_weak_ref* weak_ref);

void zazen_weak_ref_destroy(struct zazen_weak_ref* ref);

void zazen_weak_ref_set_data(struct zazen_weak_ref* ref, void* data, struct wl_signal* destroy_signal,
                             zazen_weak_ref_destroy_func_t on_destroy);

#endif  //  LIBZAZEN_UTIL_H
