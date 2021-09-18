#include "opengl_render_item.h"

#include <wayland-server.h>

#include "opengl_render_component_back_state.h"
#include "util.h"

struct zazen_opengl_render_item {
  struct zazen_opengl_render_component_manager* manager;

  void* vertex_buffer_data;
  int32_t vertex_buffer_size;
  uint32_t vertex_buffer_stride;
  bool vertex_buffer_changed;

  void* texture_2d_data;
  enum z11_opengl_texture_2d_format texture_format;
  int32_t texture_width;
  int32_t texture_height;
  int32_t texture_buffer_size;
  bool texture_changed;

  const char* vertex_shader_source;
  const char* fragment_shader_source;
  bool shader_changed;

  struct wl_array vertex_input_attributes;
  bool input_attributes_changed;

  enum z11_opengl_topology topology;
  bool topology_changed;

  struct zazen_opengl_render_component_back_state back_state;
};

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

  render_item->vertex_buffer_changed = false;
  render_item->texture_changed = false;
  render_item->shader_changed = false;
  render_item->input_attributes_changed = false;
  render_item->topology_changed = true;
  render_item->topology = Z11_OPENGL_TOPOLOGY_LINES;  // default value

  wl_array_init(&render_item->vertex_input_attributes);

  wl_list_init(&render_item->back_state.link);

  return render_item;

out:
  return NULL;
}

void zazen_opengl_render_item_destroy(
    struct zazen_opengl_render_item* render_item)
{
  wl_array_release(&render_item->vertex_input_attributes);
  wl_list_remove(&render_item->back_state.link);
  zazen_opengl_render_component_back_state_destroy(&render_item->back_state);
  free(render_item);
}

void zazen_opengl_render_item_set_vertex_buffer(
    struct zazen_opengl_render_item* render_item, void* vertex_buffer_data,
    int32_t vertex_buffer_size, uint32_t vertex_buffer_stride)
{
  render_item->vertex_buffer_data = vertex_buffer_data;
  render_item->vertex_buffer_size = vertex_buffer_size;
  render_item->vertex_buffer_stride = vertex_buffer_stride;
  render_item->vertex_buffer_changed = true;
}

void zazen_opengl_render_item_unset_vertex_buffer(
    struct zazen_opengl_render_item* render_item)
{
  render_item->vertex_buffer_data = NULL;
  render_item->vertex_buffer_changed = true;
}

void zazen_opengl_render_item_set_texture_2d(
    struct zazen_opengl_render_item* render_item, void* texture_data,
    enum z11_opengl_texture_2d_format format, int32_t width, int32_t height,
    int32_t buffer_size)
{
  render_item->texture_2d_data = texture_data;
  render_item->texture_format = format;
  render_item->texture_width = width;
  render_item->texture_height = height;
  render_item->texture_buffer_size = buffer_size;
  render_item->texture_changed = true;
}

void zazen_opengl_render_item_unset_texture_2d(
    struct zazen_opengl_render_item* render_item)
{
  render_item->texture_2d_data = NULL;
  render_item->texture_changed = true;
}

void zazen_opengl_render_item_set_shader(
    struct zazen_opengl_render_item* render_item,
    const char* vertex_shader_source, const char* fragment_shader_source)
{
  render_item->vertex_shader_source = vertex_shader_source;
  render_item->fragment_shader_source = fragment_shader_source;
  render_item->shader_changed = true;
}

void zazen_opengl_render_item_unset_shader(
    struct zazen_opengl_render_item* render_item)
{
  render_item->vertex_shader_source = NULL;
  render_item->fragment_shader_source = NULL;
  render_item->shader_changed = true;
}

void zazen_opengl_render_item_append_vertex_input_attribute(
    struct zazen_opengl_render_item* render_item, uint32_t location,
    enum z11_opengl_vertex_input_attribute_format format, uint32_t offset)
{
  struct zazen_opengl_render_component_back_state_vertex_input_attribute*
      input_attribute;
  input_attribute = wl_array_add(&render_item->vertex_input_attributes,
                                 sizeof *input_attribute);
  input_attribute->location = location;
  input_attribute->format = format;
  input_attribute->offset = offset;
  render_item->input_attributes_changed = true;
}

void zazen_opengl_render_item_clear_vertex_input_attribute(
    struct zazen_opengl_render_item* render_item)
{
  wl_array_release(&render_item->vertex_input_attributes);
  wl_array_init(&render_item->vertex_input_attributes);
  render_item->input_attributes_changed = true;
}

void zazen_opengl_render_item_set_topology(
    struct zazen_opengl_render_item* render_item,
    enum z11_opengl_topology topology)
{
  render_item->topology = topology;
  render_item->topology_changed = true;
}

static bool commit_texture_2d(struct zazen_opengl_render_item* render_item);
static bool commit_shader_program(struct zazen_opengl_render_item* render_item);
static bool commit_vertex_buffer(struct zazen_opengl_render_item* render_item);
static bool commit_topology(struct zazen_opengl_render_item* render_item);
static bool commit_vertex_array(struct zazen_opengl_render_item* render_item);

bool zazen_opengl_render_item_commit(
    struct zazen_opengl_render_item* render_item)
{
  wl_list_remove(&render_item->back_state.link);
  wl_list_init(&render_item->back_state.link);

  commit_texture_2d(render_item);

  if (commit_shader_program(render_item) == false ||
      commit_vertex_buffer(render_item) == false ||
      commit_topology(render_item) == false ||
      commit_vertex_array(render_item) == false) {
    render_item->texture_changed = false;
    render_item->shader_changed = false;
    render_item->vertex_buffer_changed = false;
    render_item->topology_changed = false;
    render_item->input_attributes_changed = false;
    return false;
  }

  wl_list_insert(&render_item->manager->render_component_back_state_list,
                 &render_item->back_state.link);

  render_item->texture_changed = false;
  render_item->shader_changed = false;
  render_item->vertex_buffer_changed = false;
  render_item->topology_changed = false;
  render_item->input_attributes_changed = false;
  return true;
}

static bool commit_texture_2d(struct zazen_opengl_render_item* render_item)
{
  if (render_item->texture_changed == false)
    return render_item->back_state.texture_2d_id != 0;

  zazen_opengl_render_component_back_state_delete_texture_2d(
      &render_item->back_state);

  if (render_item->texture_2d_data == NULL) return false;

  zazen_opengl_render_component_back_state_generate_texture_2d(
      &render_item->back_state, render_item->texture_format,
      render_item->texture_width, render_item->texture_height,
      render_item->texture_2d_data, render_item->texture_buffer_size);

  return true;
}

static bool commit_shader_program(struct zazen_opengl_render_item* render_item)
{
  if (render_item->shader_changed == false)
    return render_item->back_state.shader_program_id != 0;

  zazen_opengl_render_component_back_state_delete_shader_program(
      &render_item->back_state);

  if (render_item->vertex_shader_source == NULL ||
      render_item->fragment_shader_source == NULL)
    return false;

  return zazen_opengl_render_component_back_state_generate_shader_program(
      &render_item->back_state, render_item->vertex_shader_source,
      render_item->fragment_shader_source);

  return true;
}

static bool commit_vertex_buffer(struct zazen_opengl_render_item* render_item)
{
  if (render_item->vertex_buffer_changed == false)
    return render_item->back_state.vertex_buffer_id != 0;

  zazen_opengl_render_component_back_state_delete_vertex_buffer(
      &render_item->back_state);

  if (render_item->vertex_buffer_data == NULL) return false;

  zazen_opengl_render_component_back_state_generate_vertex_buffer(
      &render_item->back_state, render_item->vertex_buffer_size,
      render_item->vertex_buffer_data, render_item->vertex_buffer_stride);

  return true;
}

static bool commit_topology(struct zazen_opengl_render_item* render_item)
{
  if (render_item->topology_changed == false) return true;

  zazen_opengl_render_component_back_state_set_topology_mode(
      &render_item->back_state, render_item->topology);

  return true;
}

static bool commit_vertex_array(struct zazen_opengl_render_item* render_item)
{
  if (render_item->vertex_buffer_changed == false &&
      render_item->input_attributes_changed == false)
    return render_item->back_state.vertex_array_id != 0;

  zazen_opengl_render_component_back_state_delete_vertex_array(
      &render_item->back_state);

  if (render_item->vertex_buffer_data == NULL) return false;

  zazen_opengl_render_component_back_state_generate_vertex_array(
      &render_item->back_state, &render_item->vertex_input_attributes);

  return true;
}
