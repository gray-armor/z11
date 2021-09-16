#define GNU_SOURCE 1
#include "z_window.h"

#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <wayland-client.h>

static void global_registry_handler(void *data, struct wl_registry *registry,
                                    uint32_t id, const char *interface,
                                    uint32_t version)
{
  ZWindow *zwindow = (ZWindow *)data;
  zwindow->GlobalRegistryHandler(registry, id, interface, version);
}

static void global_registry_remover(void *data, struct wl_registry *registry,
                                    uint32_t id)
{
  ZWindow *zwindow = (ZWindow *)data;
  zwindow->GlobalRegistryRemover(registry, id);
}

static const struct wl_registry_listener registry_listener = {
    global_registry_handler,
    global_registry_remover,
};

static void shm_format(void *data, struct wl_shm *wl_shm, uint32_t format)
{
  ZWindow *zwindow = (ZWindow *)data;
  zwindow->ShmFormat(wl_shm, format);
}

static const struct wl_shm_listener shm_listener = {
    shm_format,
};

bool ZWindow::Connect(const char *socket)
{
  display_ = wl_display_connect(socket);
  if (display_ == NULL) goto out;

  registry_ = wl_display_get_registry(display_);
  if (registry_ == NULL) goto out_display;

  wl_registry_add_listener(registry_, &registry_listener, this);

  wl_display_dispatch(display_);
  wl_display_roundtrip(display_);

  if (compositor_ == NULL || gl_ == NULL || shm_ == NULL ||
      render_component_manager_ == NULL || shell_ == NULL)
    goto out_registry;

  return true;

out_registry:
  wl_registry_destroy(registry_);

out_display:
  wl_display_disconnect(display_);

out:
  return false;
}

void ZWindow::GlobalRegistryHandler(struct wl_registry *registry, uint32_t id,
                                    const char *interface, uint32_t version)
{
  if (strcmp(interface, "z11_compositor") == 0) {
    compositor_ = (z11_compositor *)wl_registry_bind(
        registry, id, &z11_compositor_interface, version);
  } else if (strcmp(interface, "wl_shm") == 0) {
    shm_ = (wl_shm *)wl_registry_bind(registry, id, &wl_shm_interface, version);
    wl_shm_add_listener(shm_, &shm_listener, this);
  } else if (strcmp(interface, "z11_opengl") == 0) {
    gl_ = (z11_opengl *)wl_registry_bind(registry, id, &z11_opengl_interface,
                                         version);
  } else if (strcmp(interface, "z11_opengl_render_component_manager") == 0) {
    render_component_manager_ =
        (z11_opengl_render_component_manager *)wl_registry_bind(
            registry, id, &z11_opengl_render_component_manager_interface,
            version);
  } else if (strcmp(interface, "z11_shell") == 0) {
    shell_ = (z11_shell *)wl_registry_bind(registry, id, &z11_shell_interface,
                                           version);
  }
}

void ZWindow::GlobalRegistryRemover(struct wl_registry *registry, uint32_t id)
{
  (void)registry;
  (void)id;
}

void ZWindow::ShmFormat(struct wl_shm *wl_shm, uint32_t format)
{
  (void)wl_shm;
  (void)format;
}

int ZWindow::CreateSharedFD(off_t size)
{
  const char *name = "z11-simple-box";

  int fd = memfd_create(name, MFD_CLOEXEC | MFD_ALLOW_SEALING);
  if (fd < 0) return fd;
  unlink(name);

  if (ftruncate(fd, size) < 0) {
    close(fd);
    return -1;
  }

  return fd;
}
