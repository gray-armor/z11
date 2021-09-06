#include "opengl_render_item.h"

#include <libzazen.h>
#include <opengl_util.h>
#include <wayland-server.h>

#include "util.h"

struct zazen_opengl_render_item* zazen_opengl_render_item_create(
    struct zazen_opengl_render_component_manager* manager)
{
  struct zazen_opengl_render_item* render_item;
  render_item = zalloc(sizeof *render_item);
  if (render_item == NULL) {
    zazen_log("Unable to create render item");
    goto out;
  }

  render_item->manager = manager;

  render_item->state_changed = true;

  render_item->vertex_buffer_data = NULL;
  render_item->texture_2d_data = NULL;

  render_item->vertex_shader_source = NULL;
  render_item->fragment_shader_source = NULL;

  wl_array_init(&render_item->vertex_input_attributes);

  wl_list_init(&render_item->back_state.link);

  return render_item;

out:
  return NULL;
}

void zazen_opengl_render_item_destroy(struct zazen_opengl_render_item* render_item)
{
  wl_array_release(&render_item->vertex_input_attributes);
  wl_list_remove(&render_item->back_state.link);
  // TODO: clean up back state
  free(render_item->vertex_buffer_data);
  free(render_item->texture_2d_data);
  free(render_item);
}

void zazen_opengl_render_item_commit(struct zazen_opengl_render_item* render_item)
{
  wl_list_remove(&render_item->back_state.link);
  wl_list_init(&render_item->back_state.link);

  if (gl_commit_shader_program(&render_item->back_state, render_item->vertex_shader_source,
                               render_item->fragment_shader_source) == false) {
    // TODO: Error handle
    return;
  }
  gl_commit_vertex_buffer(&render_item->back_state, render_item->vertex_buffer_size,
                          render_item->vertex_buffer_data, render_item->vertex_buffer_stride);

  gl_commit_topology_mode(&render_item->back_state, render_item->topology);

  gl_commit_vertex_array(&render_item->back_state, &render_item->vertex_input_attributes);

  render_item->state_changed = false;

  wl_list_insert(&render_item->manager->render_component_back_state_list, &render_item->back_state.link);
}
