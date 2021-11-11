#ifndef LIBZAZEN_UTIL_H
#define LIBZAZEN_UTIL_H

#include <stdlib.h>
#include <sys/mman.h>
#include <wayland-server.h>

#define UNUSED(x) ((void)x)

#define _Nullable
#define _NonNull

/* fd */
int create_shared_file(off_t size, void* content);

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

void zazen_weak_ref_set_data(struct zazen_weak_ref* ref, void* data,
                             struct wl_signal* destroy_signal,
                             zazen_weak_ref_destroy_func_t on_destroy);

// timespec

#define NSEC_PER_SEC 1000000000

static inline void timespec_from_nsec(struct timespec* time, int64_t nsec)
{
  time->tv_sec = nsec / NSEC_PER_SEC;
  time->tv_nsec = nsec % NSEC_PER_SEC;
}

static inline void timespec_from_usec(struct timespec* time, int64_t usec)
{
  timespec_from_nsec(time, usec * 1000);
}

static inline int64_t timespec_to_msec(const struct timespec* time)
{
  return (int64_t)time->tv_sec * 1000 + time->tv_nsec / 1000000;
}

#endif  //  LIBZAZEN_UTIL_H
