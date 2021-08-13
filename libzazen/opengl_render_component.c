#include "opengl_render_component.h"

#include <GL/glew.h>
#include <libzazen.h>
#include <stdio.h>
#include <wayland-server.h>

#include "opengl_render_component_manager.h"
#include "opengl_shader_program.h"
#include "opengl_texture_2d.h"
#include "opengl_vertex_buffer.h"
#include "util.h"
#include "virtual_object.h"
#include "z11-opengl-server-protocol.h"
#include "z11-server-protocol.h"

static void zazen_opengl_render_component_destroy(struct zazen_opengl_render_component* render_component);
static void zazen_opengl_render_component_commit(struct zazen_opengl_render_component* render_component);

static void zazen_opengl_render_component_handle_destroy(struct wl_resource* resource)
{
  struct zazen_opengl_render_component* render_component = wl_resource_get_user_data(resource);

  zazen_opengl_render_component_destroy(render_component);
}

static void zazen_opengl_render_component_protocol_destroy(struct wl_client* client,
                                                           struct wl_resource* resource)
{
  UNUSED(client);
  wl_resource_destroy(resource);
}

static void vertex_buffer_state_change_listener(struct wl_listener* listener, void* data)
{
  UNUSED(data);
  struct zazen_opengl_render_component* render_component;

  render_component = wl_container_of(listener, render_component, vertex_buffer_state_change_listener);

  render_component->state_changed = true;
}

static void vertex_buffer_destroy_listener(struct wl_listener* listener, void* data)
{
  UNUSED(data);
  struct zazen_opengl_render_component* render_component;

  render_component = wl_container_of(listener, render_component, vertex_buffer_destroy_listener);

  wl_list_remove(&render_component->vertex_buffer_destroy_listener.link);
  wl_list_init(&render_component->vertex_buffer_destroy_listener.link);
  wl_list_remove(&render_component->vertex_buffer_state_change_listener.link);
  wl_list_init(&render_component->vertex_buffer_state_change_listener.link);

  render_component->vertex_buffer = NULL;
}

static void zazen_opengl_render_component_protocol_attach_vertex_buffer(
    struct wl_client* client, struct wl_resource* resource, struct wl_resource* vertex_buffer_resource)
{
  UNUSED(client);
  struct zazen_opengl_render_component* render_component;
  struct zazen_opengl_vertex_buffer* vertex_buffer;

  render_component = wl_resource_get_user_data(resource);
  vertex_buffer = wl_resource_get_user_data(vertex_buffer_resource);

  wl_list_remove(&render_component->vertex_buffer_state_change_listener.link);
  wl_signal_add(&vertex_buffer->state_change_signal, &render_component->vertex_buffer_state_change_listener);

  wl_list_remove(&render_component->vertex_buffer_destroy_listener.link);
  wl_signal_add(&vertex_buffer->destroy_signal, &render_component->vertex_buffer_destroy_listener);

  render_component->vertex_buffer = vertex_buffer;

  render_component->state_changed = true;
}

static void shader_program_state_change_listener(struct wl_listener* listener, void* data)
{
  UNUSED(data);
  struct zazen_opengl_render_component* render_component;

  render_component = wl_container_of(listener, render_component, shader_program_state_change_listener);

  render_component->state_changed = true;
}

static void shader_program_destroy_handler(struct wl_listener* listener, void* data)
{
  UNUSED(data);
  struct zazen_opengl_render_component* render_component;

  render_component = wl_container_of(listener, render_component, shader_program_destroy_listener);

  wl_list_remove(&render_component->shader_program_destroy_listener.link);
  wl_list_init(&render_component->shader_program_destroy_listener.link);
  wl_list_remove(&render_component->shader_program_state_change_listener.link);
  wl_list_init(&render_component->shader_program_state_change_listener.link);

  render_component->shader_program = NULL;
}

static void zazen_opengl_render_component_protocol_attach_shader_program(
    struct wl_client* client, struct wl_resource* resource, struct wl_resource* shader_program_resource)
{
  UNUSED(client);
  struct zazen_opengl_render_component* render_component;
  struct zazen_opengl_shader_program* shader_program;

  render_component = wl_resource_get_user_data(resource);
  shader_program = wl_resource_get_user_data(shader_program_resource);

  wl_list_remove(&render_component->shader_program_state_change_listener.link);
  wl_signal_add(&shader_program->state_change_signal,
                &render_component->shader_program_state_change_listener);

  wl_list_remove(&render_component->shader_program_destroy_listener.link);
  wl_signal_add(&shader_program->destroy_signal, &render_component->shader_program_destroy_listener);

  render_component->shader_program = shader_program;

  render_component->state_changed = true;
}

static void texture_2d_state_change_listener(struct wl_listener* listener, void* data)
{
  UNUSED(data);
  struct zazen_opengl_render_component* render_component;

  render_component = wl_container_of(listener, render_component, texture_2d_state_change_listener);

  render_component->state_changed = true;
}

static void texture_2d_destroy_listener(struct wl_listener* listener, void* data)
{
  UNUSED(data);
  struct zazen_opengl_render_component* render_component;

  render_component = wl_container_of(listener, render_component, texture_2d_destroy_listener);

  wl_list_remove(&render_component->texture_2d_destroy_listener.link);
  wl_list_init(&render_component->texture_2d_destroy_listener.link);
  wl_list_remove(&render_component->texture_2d_state_change_listener.link);
  wl_list_init(&render_component->texture_2d_state_change_listener.link);

  render_component->texture_2d = NULL;
}

static void zazen_opengl_render_component_protocol_attach_texture_2d(struct wl_client* client,
                                                                     struct wl_resource* resource,
                                                                     struct wl_resource* texture_2d_resource)
{
  UNUSED(client);
  struct zazen_opengl_render_component* render_component;
  struct zazen_opengl_texture_2d* texture_2d;

  render_component = wl_resource_get_user_data(resource);
  texture_2d = wl_resource_get_user_data(texture_2d_resource);

  wl_list_remove(&render_component->texture_2d_state_change_listener.link);
  wl_signal_add(&texture_2d->state_change_signal, &render_component->texture_2d_state_change_listener);

  wl_list_remove(&render_component->texture_2d_destroy_listener.link);
  wl_signal_add(&texture_2d->destroy_signal, &render_component->texture_2d_destroy_listener);

  render_component->texture_2d = texture_2d;

  render_component->state_changed = true;
}

static void zazen_opengl_render_component_protocol_append_vertex_input_attribute(
    struct wl_client* client, struct wl_resource* resource, uint32_t location,
    enum z11_opengl_vertex_input_attribute_format format, uint32_t offset)
{
  UNUSED(client);
  struct zazen_opengl_render_component* render_component;

  render_component = wl_resource_get_user_data(resource);

  struct zazen_opengl_vertex_input_attribute* input_attribtue;

  input_attribtue = wl_array_add(&render_component->vertex_input_attributes, sizeof *input_attribtue);

  input_attribtue->location = location;
  input_attribtue->format = format;
  input_attribtue->offset = offset;

  render_component->state_changed = true;
}

static void zazen_opengl_render_component_protocol_clear_vertex_input_attributes(struct wl_client* client,
                                                                                 struct wl_resource* resource)
{
  UNUSED(client);
  struct zazen_opengl_render_component* render_component;

  render_component = wl_resource_get_user_data(resource);

  wl_array_release(&render_component->vertex_input_attributes);
  wl_array_init(&render_component->vertex_input_attributes);

  render_component->state_changed = true;
}

static void zazen_opengl_render_component_protocol_set_topology(struct wl_client* client,
                                                                struct wl_resource* resource,
                                                                enum z11_opengl_topology topology)
{
  UNUSED(client);
  struct zazen_opengl_render_component* render_component;

  render_component = wl_resource_get_user_data(resource);

  render_component->topology = topology;

  render_component->state_changed = true;
}

static const struct z11_opengl_render_component_interface zazen_opengl_render_component_interface = {
    .destroy = zazen_opengl_render_component_protocol_destroy,
    .attach_vertex_buffer = zazen_opengl_render_component_protocol_attach_vertex_buffer,
    .attach_shader_program = zazen_opengl_render_component_protocol_attach_shader_program,
    .attach_texture_2d = zazen_opengl_render_component_protocol_attach_texture_2d,
    .append_vertex_input_attribute = zazen_opengl_render_component_protocol_append_vertex_input_attribute,
    .clear_vertex_input_attributes = zazen_opengl_render_component_protocol_clear_vertex_input_attributes,
    .set_topology = zazen_opengl_render_component_protocol_set_topology,
};

static void virtual_object_destroy_signal_handler(struct wl_listener* listener, void* data)
{
  struct zazen_virtual_object* virtual_object = data;
  struct zazen_opengl_render_component* render_component;

  wl_resource_post_error(virtual_object->resource, Z11_VIRTUAL_OBJECT_ERROR_DEFUNCT_COMPONENT,
                         "Virtual object was destroyed before associated component");

  render_component = wl_container_of(listener, render_component, virtual_object_destroy_signal_listener);
  wl_resource_destroy(render_component->resource);
}

static void virtual_object_commit_signal_handler(struct wl_listener* listener, void* data)
{
  UNUSED(data);
  struct zazen_opengl_render_component* render_component;

  render_component = wl_container_of(listener, render_component, virtual_object_commit_signal_listener);

  if (render_component->state_changed) {
    zazen_opengl_render_component_commit(render_component);
    render_component->state_changed = false;
  }
}

struct zazen_opengl_render_component* zazen_opengl_render_component_create(
    struct wl_client* client, uint32_t id, struct zazen_opengl_render_component_manager* manager,
    struct zazen_virtual_object* virtual_object)
{
  struct zazen_opengl_render_component* render_component;
  struct wl_resource* resource;

  render_component = zalloc(sizeof *render_component);
  if (render_component == NULL) {
    wl_client_post_no_memory(client);
    goto out;
  }

  resource = wl_resource_create(client, &z11_opengl_render_component_interface, 1, id);
  if (resource == NULL) {
    wl_client_post_no_memory(client);
    goto out_component;
  }

  wl_resource_set_implementation(resource, &zazen_opengl_render_component_interface, render_component,
                                 zazen_opengl_render_component_handle_destroy);

  render_component->resource = resource;
  render_component->manager = manager;

  wl_list_init(&render_component->back_state.link);

  render_component->virtual_object_destroy_signal_listener.notify = virtual_object_destroy_signal_handler;
  // Make sure that render_component->virtual_object will not be a dangling pointer
  wl_signal_add(&virtual_object->destroy_signal, &render_component->virtual_object_destroy_signal_listener);

  render_component->virtual_object_commit_signal_listener.notify = virtual_object_commit_signal_handler;
  wl_signal_add(&virtual_object->commit_signal, &render_component->virtual_object_commit_signal_listener);

  render_component->state_changed = true;

  render_component->vertex_buffer = NULL;
  render_component->vertex_buffer_state_change_listener.notify = vertex_buffer_state_change_listener;
  wl_list_init(&render_component->vertex_buffer_state_change_listener.link);
  render_component->vertex_buffer_destroy_listener.notify = vertex_buffer_destroy_listener;
  wl_list_init(&render_component->vertex_buffer_destroy_listener.link);

  render_component->shader_program = NULL;
  render_component->shader_program_state_change_listener.notify = shader_program_state_change_listener;
  wl_list_init(&render_component->shader_program_state_change_listener.link);
  render_component->shader_program_destroy_listener.notify = shader_program_destroy_handler;
  wl_list_init(&render_component->shader_program_destroy_listener.link);

  render_component->texture_2d = NULL;
  render_component->texture_2d_state_change_listener.notify = texture_2d_state_change_listener;
  wl_list_init(&render_component->texture_2d_state_change_listener.link);
  render_component->texture_2d_destroy_listener.notify = texture_2d_destroy_listener;
  wl_list_init(&render_component->texture_2d_destroy_listener.link);

  wl_array_init(&render_component->vertex_input_attributes);

  render_component->topology = Z11_OPENGL_TOPOLOGY_LINES;

  return render_component;

out_component:
  free(render_component);

out:
  return NULL;
}

static void zazen_opengl_render_component_destroy(struct zazen_opengl_render_component* render_component)
{
  wl_array_release(&render_component->vertex_input_attributes);
  wl_list_remove(&render_component->back_state.link);
  wl_list_remove(&render_component->virtual_object_destroy_signal_listener.link);
  wl_list_remove(&render_component->virtual_object_commit_signal_listener.link);
  wl_list_remove(&render_component->vertex_buffer_state_change_listener.link);
  wl_list_remove(&render_component->vertex_buffer_destroy_listener.link);
  wl_list_remove(&render_component->shader_program_state_change_listener.link);
  wl_list_remove(&render_component->shader_program_destroy_listener.link);
  wl_list_remove(&render_component->texture_2d_state_change_listener.link);
  wl_list_remove(&render_component->texture_2d_destroy_listener.link);
  // TODO: clean up back state
  free(render_component);
}

static void commit_texture_2d(struct zazen_opengl_render_component* render_component);
static bool commit_shader_program(struct zazen_opengl_render_component* render_component);
static void commit_vertex_buffer(struct zazen_opengl_render_component* render_component);
static GLuint get_size_from_attribute_format(enum z11_opengl_vertex_input_attribute_format format);
static GLenum get_type_from_attribute_format(enum z11_opengl_vertex_input_attribute_format format);
static GLenum get_topology_mode(enum z11_opengl_topology topology);

static void zazen_opengl_render_component_commit(struct zazen_opengl_render_component* render_component)
{
  struct zazen_opengl_vertex_input_attribute* vertex_input_attribute;

  wl_list_remove(&render_component->back_state.link);
  wl_list_init(&render_component->back_state.link);

  commit_texture_2d(render_component);
  if (commit_shader_program(render_component) == false) {
    // TODO: Error handle
    return;
  }
  commit_vertex_buffer(render_component);

  render_component->back_state.topology_mode = get_topology_mode(render_component->topology);

  glDeleteVertexArrays(1, &render_component->back_state.vertex_array_id);
  render_component->back_state.vertex_array_id = 0;
  if (render_component->back_state.vertex_buffer_id == 0 ||
      render_component->back_state.shader_program_id == 0) {
    return;
  }

  glGenVertexArrays(1, &render_component->back_state.vertex_array_id);

  glBindVertexArray(render_component->back_state.vertex_array_id);
  glBindBuffer(GL_ARRAY_BUFFER, render_component->back_state.vertex_buffer_id);

  wl_array_for_each(vertex_input_attribute, &render_component->vertex_input_attributes)
  {
    GLint size = get_size_from_attribute_format(vertex_input_attribute->format);
    GLenum type = get_type_from_attribute_format(vertex_input_attribute->format);
    glEnableVertexAttribArray(vertex_input_attribute->location);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
    glVertexAttribPointer(vertex_input_attribute->location, size, type, GL_FALSE,
                          render_component->back_state.vertex_stride, (void*)vertex_input_attribute->offset);
#pragma GCC diagnostic pop
  }
  glBindVertexArray(0);

  wl_list_insert(&render_component->manager->render_component_back_state_list,
                 &render_component->back_state.link);
}

static void commit_texture_2d(struct zazen_opengl_render_component* render_component)
{
  struct zazen_opengl_texture_2d_state* state;
  struct wl_shm_raw_buffer* shm_raw_buffer;
  int32_t buffer_size;
  void* data;

  glDeleteTextures(1, &render_component->back_state.texture_2d_id);
  render_component->back_state.texture_2d_id = 0;

  if (render_component->texture_2d == NULL) return;
  state = render_component->texture_2d->state;

  shm_raw_buffer = wl_shm_raw_buffer_get(state->raw_buffer_resource);
  buffer_size = wl_shm_raw_buffer_get_size(shm_raw_buffer);
  data = wl_shm_raw_buffer_get_data(shm_raw_buffer);

  glGenTextures(1, &render_component->back_state.texture_2d_id);
  glBindTexture(GL_TEXTURE_2D, render_component->back_state.texture_2d_id);
  if (state->format == Z11_OPENGL_TEXTURE_2D_FORMAT_ARGB8888 &&
      buffer_size <= state->width * state->height * 4) {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, state->width, state->height, 0, GL_BGRA,
                 GL_UNSIGNED_INT_8_8_8_8_REV, data);
  }
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glBindTexture(GL_TEXTURE_2D, 0);
}

static bool commit_shader_program(struct zazen_opengl_render_component* render_component)
{
  glDeleteProgram(render_component->back_state.shader_program_id);
  render_component->back_state.shader_program_id = 0;

  if (render_component->shader_program == NULL) return true;

  render_component->back_state.shader_program_id = glCreateProgram();

  GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex_shader, 1, (const char**)&render_component->shader_program->vertex_shader_source,
                 NULL);
  glCompileShader(vertex_shader);

  GLint vertex_shader_compiled = GL_FALSE;
  glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &vertex_shader_compiled);
  if (vertex_shader_compiled != GL_TRUE) {
    glDeleteShader(vertex_shader);
    goto out;
  }
  glAttachShader(render_component->back_state.shader_program_id, vertex_shader);
  glDeleteShader(vertex_shader);

  GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment_shader, 1, (const char**)&render_component->shader_program->fragment_shader_source,
                 NULL);
  glCompileShader(fragment_shader);

  GLint fragment_shader_compiled = GL_FALSE;
  glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &fragment_shader_compiled);
  if (fragment_shader_compiled != GL_TRUE) {
    glDeleteShader(fragment_shader);
    goto out;
  }
  glAttachShader(render_component->back_state.shader_program_id, fragment_shader);
  glDeleteShader(fragment_shader);

  glLinkProgram(render_component->back_state.shader_program_id);

  GLint program_success = GL_TRUE;
  glGetProgramiv(render_component->back_state.shader_program_id, GL_LINK_STATUS, &program_success);
  if (program_success != GL_TRUE) {
    goto out;
  }

  glUseProgram(render_component->back_state.shader_program_id);
  glUseProgram(0);

  return true;

out:
  glDeleteProgram(render_component->back_state.shader_program_id);
  render_component->back_state.shader_program_id = 0;
  return false;
}

static void commit_vertex_buffer(struct zazen_opengl_render_component* render_component)
{
  struct wl_shm_raw_buffer* shm_raw_buffer;
  int32_t buffer_size;
  void* data;

  glDeleteBuffers(1, &render_component->back_state.vertex_buffer_id);
  render_component->back_state.vertex_buffer_id = 0;

  if (render_component->vertex_buffer->raw_buffer_resource == NULL) return;

  shm_raw_buffer = wl_shm_raw_buffer_get(render_component->vertex_buffer->raw_buffer_resource);
  data = wl_shm_raw_buffer_get_data(shm_raw_buffer);
  buffer_size = wl_shm_raw_buffer_get_size(shm_raw_buffer);

  glGenBuffers(1, &render_component->back_state.vertex_buffer_id);
  glBindBuffer(GL_ARRAY_BUFFER, render_component->back_state.vertex_buffer_id);
  glBufferData(GL_ARRAY_BUFFER, buffer_size, data, GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  render_component->back_state.vertex_stride = render_component->vertex_buffer->stride;

  render_component->back_state.vertex_buffer_size = buffer_size;
}

static GLuint get_size_from_attribute_format(enum z11_opengl_vertex_input_attribute_format format)
{
  switch (format) {
    case Z11_OPENGL_VERTEX_INPUT_ATTRIBUTE_FORMAT_FLOAT_SCALAR:
      return 1;
    case Z11_OPENGL_VERTEX_INPUT_ATTRIBUTE_FORMAT_FLOAT_VECTOR2:
      return 2;
    case Z11_OPENGL_VERTEX_INPUT_ATTRIBUTE_FORMAT_FLOAT_VECTOR3:
      return 3;
    case Z11_OPENGL_VERTEX_INPUT_ATTRIBUTE_FORMAT_FLOAT_VECTOR4:
      return 4;
    default:
      return 0;
  }
}

static GLenum get_type_from_attribute_format(enum z11_opengl_vertex_input_attribute_format format)
{
  switch (format) {
    case Z11_OPENGL_VERTEX_INPUT_ATTRIBUTE_FORMAT_FLOAT_SCALAR:
    case Z11_OPENGL_VERTEX_INPUT_ATTRIBUTE_FORMAT_FLOAT_VECTOR2:
    case Z11_OPENGL_VERTEX_INPUT_ATTRIBUTE_FORMAT_FLOAT_VECTOR3:
    case Z11_OPENGL_VERTEX_INPUT_ATTRIBUTE_FORMAT_FLOAT_VECTOR4:
      return GL_FLOAT;
    default:
      return GL_FLOAT;
  }
}

static GLenum get_topology_mode(enum z11_opengl_topology topology)
{
  switch (topology) {
    case Z11_OPENGL_TOPOLOGY_POINTS:
      return GL_POINTS;
    case Z11_OPENGL_TOPOLOGY_LINES:
      return GL_LINES;
    case Z11_OPENGL_TOPOLOGY_LINE_STRIP:
      return GL_LINE_STRIP;
    case Z11_OPENGL_TOPOLOGY_TRIANGLES:
      return GL_TRIANGLES;
    case Z11_OPENGL_TOPOLOGY_TRIANGLE_STRIP:
      return GL_TRIANGLE_STRIP;
    case Z11_OPENGL_TOPOLOGY_TRIANGLE_FAN:
      return GL_TRIANGLE_FAN;
    default:
      return GL_LINES;
  }
}
