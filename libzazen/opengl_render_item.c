#include "opengl_render_item.h"

#include <wayland-server.h>

#include "opengl_render_component_back_state.h"
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

static void commit_texture_2d(struct zazen_opengl_render_item* render_item);
static bool commit_shader_program(struct zazen_opengl_render_item* render_item);
static void commit_vertex_buffer(struct zazen_opengl_render_item* render_item);
static void commit_vertex_array(struct zazen_opengl_render_item* render_item);

void zazen_opengl_render_item_commit(struct zazen_opengl_render_item* render_item)
{
  wl_list_remove(&render_item->back_state.link);
  wl_list_init(&render_item->back_state.link);

  commit_texture_2d(render_item);
  if (commit_shader_program(render_item) == false) {
    // TODO: Error handle
    return;
  }
  commit_vertex_buffer(render_item);

  zazen_opengl_render_component_back_state_set_topology_mode(&render_item->back_state, render_item->topology);

  commit_vertex_array(render_item);

  wl_list_insert(&render_item->manager->render_component_back_state_list, &render_item->back_state.link);
}

static void commit_texture_2d(struct zazen_opengl_render_item* render_item)
{
  zazen_opengl_render_component_back_state_delete_texture_2d(&render_item->back_state);

  if (render_item->texture_2d_data == NULL) return;

  // TODO: Enable to create texture 2d with render item
}

static bool commit_shader_program(struct zazen_opengl_render_item* render_item)
{
  zazen_opengl_render_component_back_state_delete_shader_program(&render_item->back_state);

  if (render_item->vertex_shader_source == NULL || render_item->fragment_shader_source == NULL) return true;

  return zazen_opengl_render_component_back_state_create_shader_program(
      &render_item->back_state, render_item->vertex_shader_source, render_item->fragment_shader_source);
}

static void commit_vertex_buffer(struct zazen_opengl_render_item* render_item)
{
  zazen_opengl_render_component_back_state_delete_vertex_buffer(&render_item->back_state);

  if (render_item->vertex_buffer_data == NULL) return;

  zazen_opengl_render_component_back_state_create_vertex_buffer(
      &render_item->back_state, render_item->vertex_buffer_size, render_item->vertex_buffer_data,
      render_item->vertex_buffer_stride);
}

static void commit_vertex_array(struct zazen_opengl_render_item* render_item)
{
  zazen_opengl_render_component_back_state_delete_vertex_array(&render_item->back_state);

  if (render_item->back_state.vertex_buffer_id == 0 || render_item->back_state.shader_program_id == 0) {
    return;
  }

  zazen_opengl_render_component_back_state_create_vertex_array(&render_item->back_state,
                                                               &render_item->vertex_input_attributes);
}
