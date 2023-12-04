#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <linux/input-event-codes.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <wayland-client.h>
#include <wayland-util.h>
#include <xkbcommon/xkbcommon.h>
#include <shm.h>
#include <xdg-shell-protocol.h>

static struct wxkb_switch_state
{
	struct wl_display *display;
	struct wl_registry *registry;
	struct wl_seat *seat;
	struct wl_keyboard *keyboard;
	struct xkb_context *context;
	struct xkb_keymap *keymap;
	struct xkb_state *state;
};

static int verbose_flag = 1;
static int debug_flag = 1;

/*!
* @brief print debug message
*
* This function prints message for debug mode
* 
* @param text[in] pointer to the text
* @return 0 if Ok and -1 if error
*/
int debug(const char *text)
{
	if (debug_flag)
	perror(text);
	
	return 0;
}

/*!
* @brief print verbose message
*
* This function prints message for verbose mode
* 
* @param text[in] pointer to the text
* @return 0 if Ok and -1 if error
*/
int message(const char *text)
{
	if (verbose_flag)
	{
		int ret = puts(text);
		if (ret == EOF)
		return -1;
	}
	
	return 0;
}

static int list_layouts(struct xkb_keymap *keymap)
{
	if (keymap == NULL)
	{
		puts("keymap is NULL");
		return -1;
	}
	
	xkb_layout_index_t num = xkb_keymap_num_layouts(keymap);
	printf("num: %d\n", num);

	for (xkb_layout_index_t i = 0; i < num; i++)
	{
		const char *name = xkb_keymap_layout_get_name(keymap, i);
		if (name == NULL)
		{
			debug("xkb_keymap_layout_get_name() error");
			return -1;
		}
		if (verbose_flag)
		printf("%s\n", name);
	}
	
	return 0;
}

static void wl_keyboard_keymap(void *data, struct wl_keyboard *wl_keyboard, uint32_t format, int32_t fd, uint32_t size) 
{
	// puts("wl_keyboard_keymap() !!!");
	struct wxkb_switch_state *state = data;

	char *map_shm = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (map_shm == MAP_FAILED)
	{
		close(fd);
		fprintf(stderr, "Unable to mmap keymap: %s", strerror(errno));
		return;
	}
	// printf("map_shm:\n%s", map_shm);

	if (format != WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1) {
		munmap(map_shm, size);
		close(fd);
		return;
	}

	state->keymap = xkb_keymap_new_from_string(state->context, map_shm, XKB_KEYMAP_FORMAT_TEXT_V1, XKB_KEYMAP_COMPILE_NO_FLAGS);
	state->state = xkb_state_new(state->keymap);
	munmap(map_shm, size);
	close(fd);

	// list ru and us keyboard layouts
	puts("It prints keyboard layout via xkbcommon and wayland");
	list_layouts(state->keymap);


	// xkb_mod_mask_t depressed_mods = xkb_state_serialize_mods(state, XKB_STATE_MODS_DEPRESSED);
	// xkb_mod_mask_t latched_mods = xkb_state_serialize_mods(state, XKB_STATE_MODS_LATCHED);
	// xkb_mod_mask_t locked_mods = xkb_state_serialize_mods(state, XKB_STATE_MODS_LOCKED);

	// xkb_layout_index_t depressed_layout = xkb_state_serialize_layout(state, XKB_STATE_LAYOUT_DEPRESSED);
	// xkb_layout_index_t latched_layout = xkb_state_serialize_layout(state, XKB_STATE_LAYOUT_LATCHED);
	// xkb_layout_index_t locked_layout = xkb_state_serialize_layout(state, XKB_STATE_LAYOUT_LOCKED);

	// xkb_state_update_mask(state, depressed_mods, latched_mods, locked_mods, 1, latched_layout, locked_layout);


}

static void wl_keyboard_enter(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial, struct wl_surface *surface, struct wl_array *keys)
{

}

static void wl_keyboard_leave(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial, struct wl_surface *surface) 
{

}

static void wl_keyboard_key(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state) 
{

}


static void wl_keyboard_modifiers(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked, uint32_t group)
{

}

static void wl_keyboard_repeat_info(void *data, struct wl_keyboard *wl_keyboard, int32_t rate, int32_t delay) 
{

}


static const struct wl_keyboard_listener keyboard_listener = {
	.keymap = wl_keyboard_keymap,
	.enter = wl_keyboard_enter,
	.leave = wl_keyboard_leave,
	.key = wl_keyboard_key,
	.modifiers = wl_keyboard_modifiers,
	.repeat_info = wl_keyboard_repeat_info,
};

static void wl_seat_capabilities(void *data, struct wl_seat *wl_seat, uint32_t capabilities)
{
	struct wxkb_switch_state *state = data;

	if ((capabilities & WL_SEAT_CAPABILITY_KEYBOARD)) 
	{
		struct wl_keyboard *keyboard = wl_seat_get_keyboard(wl_seat);
		wl_keyboard_add_listener(keyboard, &keyboard_listener, data);
	}
}

static void wl_seat_name(void *data, struct wl_seat *seat, const char *name) {
	struct wev_state *state = data;
}

static const struct wl_seat_listener seat_listener = {
	.capabilities = wl_seat_capabilities,
	.name = wl_seat_name,
};



static void registry_global(void *data, struct wl_registry *wl_registry, uint32_t name, const char *interface, uint32_t version)
{
	struct wxkb_switch_state *state = data;

	
	
	if (strcmp(interface, "wl_seat") == 0)
	{
		// printf("name: %d\ninterface: %s\nversion: %d\n", name, interface, version);
		state->seat = wl_registry_bind(state->registry, name, &wl_seat_interface, version);
	}
	
}

static void registry_global_remove(
		void *data, struct wl_registry *wl_registry, uint32_t name) {
	/* Who cares */
}

static const struct wl_registry_listener registry_listener = {
	.global = registry_global,
	.global_remove = registry_global_remove,
};





int main(int argc, char const *argv[])
{
	struct wxkb_switch_state state;

	

	state.context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);

	state.keymap = xkb_keymap_new_from_names(state.context, NULL, XKB_KEYMAP_COMPILE_NO_FLAGS);
	if (state.keymap == NULL)
	{
		debug("xkb_keymap_new_from_names");
		return -1;
	}
	puts("It prints keyboard layout only via xkbcommon");
	list_layouts(state.keymap);


	state.display = wl_display_connect(NULL);
	if (!state.display) {
		fprintf(stderr, "Failed to connect to Wayland display\n");
		return 1;
	}
	state.registry = wl_display_get_registry(state.display);
	if (!state.registry) {
		fprintf(stderr, "Failed to obtain Wayland registry\n");
		return 1;
	}

	wl_registry_add_listener(state.registry, &registry_listener, &state);
	wl_display_roundtrip(state.display);

	if (state.seat == NULL)
	{
		perror("seat is NULL");
	}
	state.keyboard = wl_seat_get_keyboard(state.seat);
	if (state.keyboard == NULL)
	{
		perror("keyboard is NULL");
	}

	wl_keyboard_add_listener(state.keyboard, &keyboard_listener, &state);
	wl_display_roundtrip(state.display);
	

	// wl_display_dispatch(state.display);

	return 0;
}
