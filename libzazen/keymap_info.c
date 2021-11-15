#include "keymap_info.h"

#include <string.h>
#include <unistd.h>
#include <xkbcommon/xkbcommon.h>

#include "util.h"

struct zazen_keymap_info *zazen_keymap_info_create()
{
  struct zazen_keymap_info *info;
  struct xkb_context *context;
  struct xkb_keymap *keymap;
  char *keymap_string;

  info = zalloc(sizeof *info);
  if (info == NULL) return NULL;

  context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
  if (context == NULL) {
    zazen_log("Failed to create XKB context\n");
    goto out_info;
  }

  keymap =
      xkb_keymap_new_from_names(context, NULL, XKB_KEYMAP_COMPILE_NO_FLAGS);
  if (keymap == NULL) {
    zazen_log("Failed to create XKB keymap\n");
    goto out_context;
  }

  keymap_string = xkb_keymap_get_as_string(keymap, XKB_KEYMAP_FORMAT_TEXT_V1);
  if (keymap_string == NULL) {
    zazen_log("Failed to get string version of keymap\n");
    goto out_keymap;
  }

  info->size = strlen(keymap_string) + 1;
  info->fd = create_shared_file(info->size, keymap_string);
  if (info->fd < 0) {
    zazen_log("Failed to create file\n");
    goto out_keymap_string;
  }
  info->format = Z11_KEYBOARD_KEYMAP_FORMAT_XKB_V1;

  free(keymap_string);

  xkb_keymap_unref(keymap);

  xkb_context_unref(context);

  return info;

out_keymap_string:
  free(keymap_string);

out_keymap:
  xkb_keymap_unref(keymap);

out_context:
  xkb_context_unref(context);

out_info:
  free(info);

  return NULL;
}

void zazen_keymap_info_destroy(struct zazen_keymap_info *info)
{
  close(info->fd);
  free(info);
}
