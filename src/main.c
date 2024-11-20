#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <assert.h>

#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <getopt.h>
#include <pwd.h>


#include <X11/Xlib.h>
#include <X11/X.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>

#include <X11/XKBlib.h>
#include <X11/extensions/XKBrules.h>

#include <xkbcommon/xkbcommon.h>
#include <xkbcommon/xkbcommon-x11.h>

int32_t
xkb_groups_list(void);

int32_t
xkb_groups_lock_prev(void);

int32_t
xkb_groups_lock_next(void);

int32_t 
options_handler(int32_t     argc, 
                const char* argv[]);

// sets in options_handler(), I cant do that varible local
bool debug_flag = false;

#define debug_msg(...)    \
{                         \
  if (debug_flag)         \
  {                       \
    printf(__VA_ARGS__);  \
  }                       \
}

#define debug_msgnl(...)  \
{                         \
  if (debug_flag)         \
  {                       \
    printf(__VA_ARGS__);  \
    printf("\n");         \
  }                       \
}


int32_t 
main( int32_t     argc, 
      const char* argv[])
{
	if (options_handler(argc, argv) != 0)
  {
    debug_msgnl("options_handler() error occurred")
    return -1;
  }

	return 0;
}

/*!
* @brief process program options
*
* This function process program options
* 
* @param argc[in] Count of arguments.
* @param argv[in] pointer to arguments array of strings.
* @return 0 if Ok and -1 if error
*/
int32_t 
options_handler(int32_t     argc, 
                const char* argv[])
{
  assert(argv);

  /* getopt_long stores the option index here. */
  int option_index = 0;

  while (true)
  {
    static struct option long_options[] =
    {
      {"debug",   no_argument,  0,  'd' },
      {"help",    no_argument,  0,  'h' },
      {"next",    no_argument,  0,  'n' },
      {"prev",    no_argument,  0, 	'p' },
      {"list",    no_argument,  0,  'l' },
      {"version", no_argument,  0,  'v' },
      {0, 0, 0, 0}
    };

		int32_t c = getopt_long(argc, 
                            (char* const*)argv, 
                            "dhnplv",
                            long_options, 
                            &option_index);
    /* Detect the end of the options. */
		if (c == -1)
    {      
      // return fro the cicle
      break;
    }
    
    switch (c)
    {
      case 0:
			{
        /* If this option set a flag, do nothing else now. */
        if (long_options[option_index].flag != 0)
        {
          break;
        }
        debug_msg ("option %s", long_options[option_index].name);
        if (optarg)
        {
          debug_msg (" with arg %s", optarg);
        }
        debug_msg ("\n");
      }
			break;

      case 'd':
      {
        debug_flag = true;
      }
      break;

      case 'h':
      {
        debug_msgnl("Usage: openswitcher [options]");
        debug_msgnl("Options:");
        debug_msgnl("  -h, --help            Display this help message.");
        debug_msgnl("  -n, --next            Set next xkb layout");
        debug_msgnl("  -p, --prev            Set previos xkb layout");
        debug_msgnl("  -l, --list            List avalible layous");
        debug_msgnl("  -v, --version         Print program version.");
        debug_msgnl("  -d, --debug           Enable debug mode.");
        debug_msgnl("");
        debug_msgnl("See also:");
        debug_msgnl("man wxkb_switch");
        debug_msgnl("sudo actkbd --help");
        debug_msgnl("man xkb-switch");
        debug_msgnl("man openswitcher");
      }
      break;

      // next group
      case 'n':
      {
        if (xkb_groups_lock_next() != 0)
        {
          debug_msgnl("xkb_groups_lock_next() error occurred")
          return -1;
        }
      }
      break;

      // previos group
      case 'p':
      {
        if (xkb_groups_lock_prev() != 0)
        {
          debug_msgnl("xkb_groups_lock_prev() error occurred")
          return -1;
        }
      }
      break;

      // list layouts
      case 'l':
      {
        if (xkb_groups_list() != 0)
        {
          debug_msgnl("xkb_groups_list() error occurred")
          return -1;
        }
      }
      break;
      
      case 'v':
      {
        puts("1.0.0");
        // debug_msg("%s\n", VERSION);
      }
      break;
      
      case '?':
      {
        /* getopt_long already printed an error message. */
      }
      break;

      default:
      {
        abort ();
      }
    }
  }

  // if no arguments
  if (argc == 1)
  {
    if (xkb_groups_lock_next() != 0)
    {
      debug_msgnl("xkb_groups_lock_next() error occurred")
      return -1;
    }
  }
  

	/* Print any remaining command line arguments (not options). */
	if (option_index < argc-1)
  {
    debug_msg ("non-option ARGV-elements: ");
    while (option_index < argc)
    {
      debug_msg ("%s ", argv[option_index++]);
    }
    debug_msgnl("");
  }
  debug_msgnl("argc: %d ", argc);

	return 0;
}

int32_t
xkb_groups_lock_next(void)
{
  char* displayName = strdup(""); // allocates memory for string!
  int eventCode;
  int errorReturn;
  int major = XkbMajorVersion;
  int minor = XkbMinorVersion;
  int reasonReturn;


  Display* display = XkbOpenDisplay(displayName, 
                                    &eventCode, 
                                    &errorReturn, 
                                    &major, 
                                    &minor, 
                                    &reasonReturn);
  if (display == NULL)
  {
    debug_msgnl("display ERROR");
  }

	struct xkb_context* context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
	if (context == NULL)
	{
    debug_msgnl("No context for libxkbcommon");
	}

	struct xkb_keymap* keymap = xkb_keymap_new_from_names(context, 
                                                        NULL, 
                                                        XKB_KEYMAP_COMPILE_NO_FLAGS);
	if (keymap == NULL)
	{
    debug_msgnl("No keymap for libxkbcommon");
	}
		
	xkb_layout_index_t num_layouts = xkb_keymap_num_layouts(keymap);

	debug_msg("num_layouts: %d\n", num_layouts);

	xcb_connection_t* connection = xcb_connect(NULL, NULL);
	if (connection == NULL)
	{
    debug_msgnl("No connection for xcb");
	}

	uint16_t major_xkb_version_out;
	uint16_t minor_xkb_version_out;
	uint8_t base_event_out;
	uint8_t base_error_out;

	int ret = xkb_x11_setup_xkb_extension(connection,
                                        XKB_X11_MIN_MAJOR_XKB_VERSION,
                                        XKB_X11_MIN_MINOR_XKB_VERSION,
                                        XKB_X11_SETUP_XKB_EXTENSION_NO_FLAGS,
                                        &major_xkb_version_out,
                                        &minor_xkb_version_out,
                                        &base_event_out,
                                        &base_error_out);

	int32_t core_keyboard_device_id = xkb_x11_get_core_keyboard_device_id(connection);

	struct xkb_keymap* x11_keymap 
  = xkb_x11_keymap_new_from_device( context, 
                                    connection, 
                                    core_keyboard_device_id, 
                                    XKB_KEYMAP_COMPILE_NO_FLAGS);
  if (x11_keymap == NULL)
	{
    debug_msgnl("No x11_keymap for libxkbcommon-x11");
	}
	
	xkb_layout_index_t num_layouts_x11 = xkb_keymap_num_layouts(x11_keymap);
	debug_msg("num_layouts_x11: %d\n", num_layouts_x11);
  debug_msgnl("")

	for (xkb_layout_index_t i = 0; i < num_layouts_x11; i++)
	{
		debug_msg("i: %d layout: %s\n", i, xkb_keymap_layout_get_name(x11_keymap, i)); 
	}
  debug_msgnl("")

  XkbStateRec state_return;
  if (XkbGetState(display, XkbUseCoreKbd, &state_return)
  != Success)
  {
    debug_msgnl("XkbGetState() error occurred")
    return -1;
  }

  debug_msgnl("Before switch to next group:")
  debug_msg("state_return->group:          %d\n",  state_return.group);
  debug_msg("state_return->base_group:     %d\n",  state_return.base_group);
  debug_msg("state_return->locked_group:   %d\n",  state_return.locked_group);
  debug_msg("state_return->latched_group:  %d\n",  state_return.latched_group);
  debug_msgnl("")

  if (state_return.group >= num_layouts_x11)
  {
    if (XkbLockGroup(display, XkbUseCoreKbd, 0) 
    !=  True)
    {
      debug_msgnl("XkbLockGroup() error occurred")
      return -1;
    }
  }
  else
  {
	  if (XkbLockGroup(display, XkbUseCoreKbd, state_return.group+1) 
    !=  True)
    {
      debug_msgnl("XkbLockGroup() error occurred")
      return -1;
    }
  }	

  if (XkbGetState(display, XkbUseCoreKbd, &state_return)
  != Success)
  {
    debug_msgnl("XkbGetState() error occurred")
    return -1;
  }

  debug_msgnl("After switch to next group:")
  debug_msg("state_return->group:          %d\n",  state_return.group);
  debug_msg("state_return->base_group:     %d\n",  state_return.base_group);
  debug_msg("state_return->locked_group:   %d\n",  state_return.locked_group);
  debug_msg("state_return->latched_group:  %d\n",  state_return.latched_group);
  debug_msgnl("")

  XCloseDisplay(display);

  return 0;
}

int32_t
xkb_groups_lock_prev(void)
{
  char* displayName = strdup(""); // allocates memory for string!
  int eventCode;
  int errorReturn;
  int major = XkbMajorVersion;
  int minor = XkbMinorVersion;
  int reasonReturn;


  Display* display = XkbOpenDisplay(displayName, 
                                    &eventCode, 
                                    &errorReturn, 
                                    &major, 
                                    &minor, 
                                    &reasonReturn);
  if (display == NULL)
  {
    debug_msgnl("display ERROR");
  }

	struct xkb_context* context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
	if (context == NULL)
	{
    debug_msgnl("No context for libxkbcommon");
	}

	struct xkb_keymap* keymap = xkb_keymap_new_from_names(context, 
                                                        NULL, 
                                                        XKB_KEYMAP_COMPILE_NO_FLAGS);
	if (keymap == NULL)
	{
    debug_msgnl("No keymap for libxkbcommon");
	}
		
	xkb_layout_index_t num_layouts = xkb_keymap_num_layouts(keymap);

	debug_msg("num_layouts: %d\n", num_layouts);

	xcb_connection_t* connection = xcb_connect(NULL, NULL);
	if (connection == NULL)
	{
    debug_msgnl("No connection for xcb");
	}

	uint16_t major_xkb_version_out;
	uint16_t minor_xkb_version_out;
	uint8_t base_event_out;
	uint8_t base_error_out;

	int ret = xkb_x11_setup_xkb_extension(connection,
                                        XKB_X11_MIN_MAJOR_XKB_VERSION,
                                        XKB_X11_MIN_MINOR_XKB_VERSION,
                                        XKB_X11_SETUP_XKB_EXTENSION_NO_FLAGS,
                                        &major_xkb_version_out,
                                        &minor_xkb_version_out,
                                        &base_event_out,
                                        &base_error_out);

	int32_t core_keyboard_device_id = xkb_x11_get_core_keyboard_device_id(connection);

	struct xkb_keymap* x11_keymap 
  = xkb_x11_keymap_new_from_device( context, 
                                    connection, 
                                    core_keyboard_device_id, 
                                    XKB_KEYMAP_COMPILE_NO_FLAGS);
  if (x11_keymap == NULL)
	{
    debug_msgnl("No x11_keymap for libxkbcommon-x11");
	}
	
	xkb_layout_index_t num_layouts_x11 = xkb_keymap_num_layouts(x11_keymap);
	debug_msg("num_layouts_x11: %d\n", num_layouts_x11);
  debug_msgnl("")

	for (xkb_layout_index_t i = 0; i < num_layouts_x11; i++)
	{
		debug_msg("i: %d layout: %s\n", i, xkb_keymap_layout_get_name(x11_keymap, i)); 
	}
  debug_msgnl("")

  XkbStateRec state_return;
  if (XkbGetState(display, XkbUseCoreKbd, &state_return)
  != Success)
  {
    debug_msgnl("XkbGetState() error occurred")
    return -1;
  }

  debug_msgnl("Before switch to next group:")
  debug_msg("state_return->group:          %d\n",  state_return.group);
  debug_msg("state_return->base_group:     %d\n",  state_return.base_group);
  debug_msg("state_return->locked_group:   %d\n",  state_return.locked_group);
  debug_msg("state_return->latched_group:  %d\n",  state_return.latched_group);
  debug_msgnl("")

  if (state_return.group <= 0)
  {
    if (XkbLockGroup(display, XkbUseCoreKbd, num_layouts_x11-1) 
    !=  True)
    {
      debug_msgnl("XkbLockGroup() error occurred")
      return -1;
    }
  }
  else
  {
	  if (XkbLockGroup(display, XkbUseCoreKbd, 0) 
    !=  True)
    {
      debug_msgnl("XkbLockGroup() error occurred")
      return -1;
    }
  }	

  if (XkbGetState(display, XkbUseCoreKbd, &state_return)
  != Success)
  {
    debug_msgnl("XkbGetState() error occurred")
    return -1;
  }

  debug_msgnl("After switch to next group:")
  debug_msg("state_return->group:          %d\n",  state_return.group);
  debug_msg("state_return->base_group:     %d\n",  state_return.base_group);
  debug_msg("state_return->locked_group:   %d\n",  state_return.locked_group);
  debug_msg("state_return->latched_group:  %d\n",  state_return.latched_group);
  debug_msgnl("")

  XCloseDisplay(display);

  return 0;
}

int32_t
xkb_groups_list(void)
{
  char* displayName = strdup(""); // allocates memory for string!
  int eventCode;
  int errorReturn;
  int major = XkbMajorVersion;
  int minor = XkbMinorVersion;
  int reasonReturn;


  Display* display = XkbOpenDisplay(displayName, 
                                    &eventCode, 
                                    &errorReturn, 
                                    &major, 
                                    &minor, 
                                    &reasonReturn);
  if (display == NULL)
  {
    debug_msgnl("display ERROR");
  }

	struct xkb_context* context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
	if (context == NULL)
	{
    debug_msgnl("No context for libxkbcommon");
	}

	struct xkb_keymap* keymap = xkb_keymap_new_from_names(context, 
                                                        NULL, 
                                                        XKB_KEYMAP_COMPILE_NO_FLAGS);
	if (keymap == NULL)
	{
    debug_msgnl("No keymap for libxkbcommon");
	}
		
	xkb_layout_index_t num_layouts = xkb_keymap_num_layouts(keymap);

	debug_msg("num_layouts: %d\n", num_layouts);

	xcb_connection_t* connection = xcb_connect(NULL, NULL);
	if (connection == NULL)
	{
    debug_msgnl("No connection for xcb");
	}

	uint16_t major_xkb_version_out;
	uint16_t minor_xkb_version_out;
	uint8_t base_event_out;
	uint8_t base_error_out;

	int ret = xkb_x11_setup_xkb_extension(connection,
                                        XKB_X11_MIN_MAJOR_XKB_VERSION,
                                        XKB_X11_MIN_MINOR_XKB_VERSION,
                                        XKB_X11_SETUP_XKB_EXTENSION_NO_FLAGS,
                                        &major_xkb_version_out,
                                        &minor_xkb_version_out,
                                        &base_event_out,
                                        &base_error_out);

	int32_t core_keyboard_device_id = xkb_x11_get_core_keyboard_device_id(connection);

	struct xkb_keymap* x11_keymap 
  = xkb_x11_keymap_new_from_device( context, 
                                    connection, 
                                    core_keyboard_device_id, 
                                    XKB_KEYMAP_COMPILE_NO_FLAGS);
  if (x11_keymap == NULL)
	{
    debug_msgnl("No x11_keymap for libxkbcommon-x11");
	}
	
	xkb_layout_index_t num_layouts_x11 = xkb_keymap_num_layouts(x11_keymap);
	debug_msg("num_layouts_x11: %d\n", num_layouts_x11);
  debug_msgnl("")


	for (xkb_layout_index_t i = 0; i < num_layouts_x11; i++)
	{
		printf("index: %d layout name: %s\n", i, xkb_keymap_layout_get_name(x11_keymap, i)); 
	}

  XCloseDisplay(display);

  return 0;
}