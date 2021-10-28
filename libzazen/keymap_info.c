#include "keymap_info.h"

#include <string.h>
#include <unistd.h>
#include <xkbcommon/xkbcommon.h>

#include "util.h"

struct zazen_keymap_info *zazen_keymap_info_create()
{
  struct zazen_keymap_info *info;
  char *keymap_string;
  size_t keymap_size;

  info = zalloc(sizeof *info);
  if (info == NULL) return NULL;

  info->context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
  if (info->context == NULL) {
    zazen_log("Failed to create XKB context\n");
    goto out;
  }

  info->names = NULL;

  info->keymap = xkb_keymap_new_from_names(info->context, info->names,
                                           XKB_KEYMAP_COMPILE_NO_FLAGS);
  if (info->keymap == NULL) {
    zazen_log("Failed to create XKB keymap\n");
    goto out;
  }

  keymap_string =
      xkb_keymap_get_as_string(info->keymap, XKB_KEYMAP_FORMAT_TEXT_V1);
  if (keymap_string == NULL) {
    zazen_log("Failed to get string version of keymap\n");
    goto out;
  }

  keymap_size = strlen(keymap_string) + 1;

  info->size = keymap_size;
  info->fd = create_shared_file(info->size, keymap_string);
  if (info->fd < 0) {
    zazen_log("Failed to create file\n");
    free(keymap_string);
    goto out;
  }
  info->format = Z11_KEYBOARD_KEYMAP_FORMAT_XKB_V1;

  free(keymap_string);

  return info;

out:
  free(info);

  return NULL;
}

void zazen_keymap_info_destroy(struct zazen_keymap_info *info)
{
  xkb_keymap_unref(info->keymap);
  xkb_context_unref(info->context);
  close(info->fd);
  free(info);
}
