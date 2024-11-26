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
* This function process program options and calls handlers
* Implicit uses debug_flag
* @param argc[in] Count of arguments.
* @param argv[in] pointer to arguments array of strings.
* @return 0 if Ok and -1 if error
*/
int32_t 
options_handler(int32_t     argc, 
                const char* argv[])
{
  assert(argv);

  debug_msgnl("argc: %d", argc);
  debug_msgnl("");

  /* getopt_long stores the option index here. */
  int32_t option_index = 0;

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
        puts("Usage: wxkb_switch [options]");
        puts("Options:");
        puts("  -h, --help            Display this help message.");
        puts("  -n, --next            Set next xkb layout.");
        puts("  -p, --prev            Set previos xkb layout.");
        puts("  -l, --list            List avalible layous.");
        puts("  -v, --version         Print program version.");
        puts("  -d, --debug           Enable debug mode.");
        puts("");
        puts("See also:");
        puts("man wxkb_switch");
        puts("sudo actkbd --help");
        puts("man xkb-switch");
        puts("man openswitcher");
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
        puts(WXKBSWITCH_VERSION);
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

	return 0;
}

/*!
* @brief set layout to next
*
* This function sets keyboard group to next
* if next is more then max group index
* then group sets to first group index i.e. 0
* Implicit uses debug_flag
* @return 0 if Ok and -1 if error
*/
int32_t
xkb_groups_lock_next(void)
{
  char* displayName = "";
  int32_t eventCode;
  int32_t errorReturn;
  int32_t major = XkbMajorVersion;
  int32_t minor = XkbMinorVersion;
  int32_t reasonReturn;

  Display* x11_display = XOpenDisplay(displayName);
  if (x11_display == NULL)
  {
    debug_msgnl("xkb_display ERROR");
    return -1;
  }
  
  int32_t opcode_rtrn;
  int32_t event_rtrn;
  int32_t error_rtrn;
  if (XkbQueryExtension(x11_display, 
                        &opcode_rtrn, 
                        &event_rtrn, 
                        &error_rtrn, 
                        &major, 
                        &minor)
  != True)
  {
    debug_msgnl("XkbQueryExtension() error occurred")
    XCloseDisplay(x11_display);
    return -1;
  }
  


  Display* xkb_display = XkbOpenDisplay(displayName, 
                                        &eventCode, 
                                        &errorReturn, 
                                        &major, 
                                        &minor, 
                                        &reasonReturn);
  if (xkb_display == NULL)
  {
    debug_msgnl("Xkb xkb_display ERROR");
    XCloseDisplay(x11_display);
    return -1;
  }

	struct xkb_context* context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
	if (context == NULL)
	{
    debug_msgnl("No context for libxkbcommon");
    XCloseDisplay(xkb_display);
    XCloseDisplay(x11_display);
    return -1;
	}

	struct xkb_keymap* keymap = xkb_keymap_new_from_names(context, 
                                                        NULL, 
                                                        XKB_KEYMAP_COMPILE_NO_FLAGS);
	if (keymap == NULL)
	{
    debug_msgnl("No keymap for libxkbcommon");
    XCloseDisplay(xkb_display);
    XCloseDisplay(x11_display);
    return -1;
	}
		
	xkb_layout_index_t num_layouts = xkb_keymap_num_layouts(keymap);

	debug_msg("num_layouts: %d\n", num_layouts);

	xcb_connection_t* connection = xcb_connect(NULL, NULL);
	if (connection == NULL)
	{
    debug_msgnl("No connection for xcb");
    XCloseDisplay(xkb_display);
    XCloseDisplay(x11_display);
    return -1;
	}

	uint16_t major_xkb_version_out;
	uint16_t minor_xkb_version_out;
	uint8_t base_event_out;
	uint8_t base_error_out;

	int32_t ret 
  = xkb_x11_setup_xkb_extension(connection,
                                XKB_X11_MIN_MAJOR_XKB_VERSION,
                                XKB_X11_MIN_MINOR_XKB_VERSION,
                                XKB_X11_SETUP_XKB_EXTENSION_NO_FLAGS,
                                &major_xkb_version_out,
                                &minor_xkb_version_out,
                                &base_event_out,
                                &base_error_out);
  if (ret != 1)
  {
    debug_msgnl("xkb_x11_setup_xkb_extension() error occurred")
    goto error;
  }

	int32_t core_keyboard_device_id = xkb_x11_get_core_keyboard_device_id(connection);
  if (core_keyboard_device_id == -1)
  {
    debug_msgnl("xkb_x11_get_core_keyboard_device_id() error occurred")
    goto error;
  }
  

	struct xkb_keymap* x11_keymap 
  = xkb_x11_keymap_new_from_device( context, 
                                    connection, 
                                    core_keyboard_device_id, 
                                    XKB_KEYMAP_COMPILE_NO_FLAGS);
  if (x11_keymap == NULL)
	{
    debug_msgnl("No x11_keymap for libxkbcommon-x11");
    goto error;
	}
	
	xkb_layout_index_t num_layouts_x11 = xkb_keymap_num_layouts(x11_keymap);
	debug_msg("num_layouts_x11: %d\n", num_layouts_x11);
  debug_msgnl("")

  // only for Gnome decrease num_layouts_x11, Gnome little buggy :)
  if (system("env | grep XDG_SESSION_DESKTOP | grep gnome > /dev/null")
  ==  0)
  {
    num_layouts_x11 -= 1;
  }

	for (xkb_layout_index_t i = 0; i < num_layouts_x11; i++)
	{
		debug_msg("i: %d layout: %s\n", i, xkb_keymap_layout_get_name(x11_keymap, i)); 
	}
  debug_msgnl("")

  XkbStateRec state_return;
  if (XkbGetState(xkb_display, XkbUseCoreKbd, &state_return)
  != Success)
  {
    debug_msgnl("XkbGetState() error occurred")
    goto error;
  }

  debug_msgnl("Before switch to next group:")
  debug_msg("state_return->group:          %d\n",  state_return.group);
  debug_msg("state_return->base_group:     %d\n",  state_return.base_group);
  debug_msg("state_return->locked_group:   %d\n",  state_return.locked_group);
  debug_msg("state_return->latched_group:  %d\n",  state_return.latched_group);
  debug_msgnl("")

  // if next layout index more then MAX layout index 
  if (state_return.locked_group+1 > (num_layouts_x11-1))
  {
    // sets first layout
    if (XkbLockGroup(xkb_display, XkbUseCoreKbd, 0)
    !=  True)
    {
      debug_msgnl("XkbLockGroup() error occurred")
      goto error;
    }
  }
  else
  {
    // sets next layout
	  if (XkbLockGroup(xkb_display, XkbUseCoreKbd, state_return.locked_group+1) 
    !=  True)
    {
      debug_msgnl("XkbLockGroup() error occurred")
      goto error;
    }
  }	

  if (XkbGetState(xkb_display, XkbUseCoreKbd, &state_return)
  != Success)
  {
    debug_msgnl("XkbGetState() error occurred")
    goto error;
  }

  debug_msgnl("After switch to next group:")
  debug_msg("state_return->group:          %d\n",  state_return.group);
  debug_msg("state_return->base_group:     %d\n",  state_return.base_group);
  debug_msg("state_return->locked_group:   %d\n",  state_return.locked_group);
  debug_msg("state_return->latched_group:  %d\n",  state_return.latched_group);
  debug_msgnl("")

  return 0;

error:
  xcb_disconnect(connection);
  XCloseDisplay(xkb_display);
  XCloseDisplay(x11_display);
  return -1;
}

/*!
* @brief set layout to next
*
* This function sets keyboard group to previos
* if previos is less then min group index
* then group sets to last group index i.e. 0
* Implicit uses debug_flag
* @return 0 if Ok and -1 if error
*/
int32_t
xkb_groups_lock_prev(void)
{
  char* displayName = "";
  int32_t eventCode;
  int32_t errorReturn;
  int32_t major = XkbMajorVersion;
  int32_t minor = XkbMinorVersion;
  int32_t reasonReturn;

  Display* x11_display = XOpenDisplay(displayName);
  if (x11_display == NULL)
  {
    debug_msgnl("xkb_display ERROR");
    return -1;
  }
  
  int32_t opcode_rtrn;
  int32_t event_rtrn;
  int32_t error_rtrn;
  if (XkbQueryExtension(x11_display, 
                        &opcode_rtrn, 
                        &event_rtrn, 
                        &error_rtrn, 
                        &major, 
                        &minor)
  != True)
  {
    debug_msgnl("XkbQueryExtension() error occurred")
    XCloseDisplay(x11_display);
    return -1;
  }
  


  Display* xkb_display = XkbOpenDisplay(displayName, 
                                        &eventCode, 
                                        &errorReturn, 
                                        &major, 
                                        &minor, 
                                        &reasonReturn);
  if (xkb_display == NULL)
  {
    debug_msgnl("Xkb xkb_display ERROR");
    XCloseDisplay(x11_display);
    return -1;
  }

	struct xkb_context* context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
	if (context == NULL)
	{
    debug_msgnl("No context for libxkbcommon");
    XCloseDisplay(xkb_display);
    XCloseDisplay(x11_display);
    return -1;
	}

	struct xkb_keymap* keymap = xkb_keymap_new_from_names(context, 
                                                        NULL, 
                                                        XKB_KEYMAP_COMPILE_NO_FLAGS);
	if (keymap == NULL)
	{
    debug_msgnl("No keymap for libxkbcommon");
    XCloseDisplay(xkb_display);
    XCloseDisplay(x11_display);
    return -1;
	}
		
	xkb_layout_index_t num_layouts = xkb_keymap_num_layouts(keymap);

	debug_msg("num_layouts: %d\n", num_layouts);

	xcb_connection_t* connection = xcb_connect(NULL, NULL);
	if (connection == NULL)
	{
    debug_msgnl("No connection for xcb");
    XCloseDisplay(xkb_display);
    XCloseDisplay(x11_display);
    return -1;
	}

	uint16_t major_xkb_version_out;
	uint16_t minor_xkb_version_out;
	uint8_t base_event_out;
	uint8_t base_error_out;

	int32_t ret 
  = xkb_x11_setup_xkb_extension(connection,
                                XKB_X11_MIN_MAJOR_XKB_VERSION,
                                XKB_X11_MIN_MINOR_XKB_VERSION,
                                XKB_X11_SETUP_XKB_EXTENSION_NO_FLAGS,
                                &major_xkb_version_out,
                                &minor_xkb_version_out,
                                &base_event_out,
                                &base_error_out);
  if (ret != 1)
  {
    debug_msgnl("xkb_x11_setup_xkb_extension() error occurred")
    goto error;
  }

	int32_t core_keyboard_device_id = xkb_x11_get_core_keyboard_device_id(connection);
  if (core_keyboard_device_id == -1)
  {
    debug_msgnl("xkb_x11_get_core_keyboard_device_id() error occurred")
    goto error;
  }
  

	struct xkb_keymap* x11_keymap 
  = xkb_x11_keymap_new_from_device( context, 
                                    connection, 
                                    core_keyboard_device_id, 
                                    XKB_KEYMAP_COMPILE_NO_FLAGS);
  if (x11_keymap == NULL)
	{
    debug_msgnl("No x11_keymap for libxkbcommon-x11");
    goto error;
	}
	
	xkb_layout_index_t num_layouts_x11 = xkb_keymap_num_layouts(x11_keymap);
	debug_msg("num_layouts_x11: %d\n", num_layouts_x11);
  debug_msgnl("")

  // only for Gnome decrease num_layouts_x11, Gnome little buggy :)
  if (system("env | grep XDG_SESSION_DESKTOP | grep gnome > /dev/null")
  ==  0)
  {
    num_layouts_x11 -= 1;
  }

	for (xkb_layout_index_t i = 0; i < num_layouts_x11; i++)
	{
		debug_msg("i: %d layout: %s\n", i, xkb_keymap_layout_get_name(x11_keymap, i)); 
	}
  debug_msgnl("")

  XkbStateRec state_return;
  if (XkbGetState(xkb_display, XkbUseCoreKbd, &state_return)
  != Success)
  {
    debug_msgnl("XkbGetState() error occurred")
    goto error;
  }

  debug_msgnl("Before switch to next group:")
  debug_msg("state_return->group:          %d\n",  state_return.group);
  debug_msg("state_return->base_group:     %d\n",  state_return.base_group);
  debug_msg("state_return->locked_group:   %d\n",  state_return.locked_group);
  debug_msg("state_return->latched_group:  %d\n",  state_return.latched_group);
  debug_msgnl("")

  // if prev layout index less then MIN layout index (0)
  if (state_return.locked_group-1 < 0)
  {
    // sets last layout
    if (XkbLockGroup(xkb_display, XkbUseCoreKbd, num_layouts_x11-1) 
    !=  True)
    {
      debug_msgnl("XkbLockGroup() error occurred")
      return -1;
    }
  }
  else
  {
    // sets previos layout
	  if (XkbLockGroup(xkb_display, XkbUseCoreKbd, 0) 
    !=  True)
    {
      debug_msgnl("XkbLockGroup() error occurred")
      return -1;
    }
  }		

  if (XkbGetState(xkb_display, XkbUseCoreKbd, &state_return)
  != Success)
  {
    debug_msgnl("XkbGetState() error occurred")
    goto error;
  }

  debug_msgnl("After switch to next group:")
  debug_msg("state_return->group:          %d\n",  state_return.group);
  debug_msg("state_return->base_group:     %d\n",  state_return.base_group);
  debug_msg("state_return->locked_group:   %d\n",  state_return.locked_group);
  debug_msg("state_return->latched_group:  %d\n",  state_return.latched_group);
  debug_msgnl("")

  return 0;

error:
  xcb_disconnect(connection);
  XCloseDisplay(xkb_display);
  XCloseDisplay(x11_display);
  return -1;
}

/*!
* @brief list layouts
*
* This function lists avalible and current layouts
* Implicit uses debug_flag
* @return 0 if Ok and -1 if error
*/
int32_t
xkb_groups_list(void)
{
  char* displayName = "";
  int32_t eventCode;
  int32_t errorReturn;
  int32_t major = XkbMajorVersion;
  int32_t minor = XkbMinorVersion;
  int32_t reasonReturn;

  Display* x11_display = XOpenDisplay(displayName);
  if (x11_display == NULL)
  {
    debug_msgnl("xkb_display ERROR");
    return -1;
  }
  
  int32_t opcode_rtrn;
  int32_t event_rtrn;
  int32_t error_rtrn;
  if (XkbQueryExtension(x11_display, 
                        &opcode_rtrn, 
                        &event_rtrn, 
                        &error_rtrn, 
                        &major, 
                        &minor)
  != True)
  {
    debug_msgnl("XkbQueryExtension() error occurred")
    XCloseDisplay(x11_display);
    return -1;
  }
  


  Display* xkb_display = XkbOpenDisplay(displayName, 
                                        &eventCode, 
                                        &errorReturn, 
                                        &major, 
                                        &minor, 
                                        &reasonReturn);
  if (xkb_display == NULL)
  {
    debug_msgnl("Xkb xkb_display ERROR");
    XCloseDisplay(x11_display);
    return -1;
  }

	struct xkb_context* context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
	if (context == NULL)
	{
    debug_msgnl("No context for libxkbcommon");
    XCloseDisplay(xkb_display);
    XCloseDisplay(x11_display);
    return -1;
	}

	struct xkb_keymap* keymap = xkb_keymap_new_from_names(context, 
                                                        NULL, 
                                                        XKB_KEYMAP_COMPILE_NO_FLAGS);
	if (keymap == NULL)
	{
    debug_msgnl("No keymap for libxkbcommon");
    XCloseDisplay(xkb_display);
    XCloseDisplay(x11_display);
    return -1;
	}
		
	xkb_layout_index_t num_layouts = xkb_keymap_num_layouts(keymap);

	debug_msg("num_layouts: %d\n", num_layouts);

	xcb_connection_t* connection = xcb_connect(NULL, NULL);
	if (connection == NULL)
	{
    debug_msgnl("No connection for xcb");
    XCloseDisplay(xkb_display);
    XCloseDisplay(x11_display);
    return -1;
	}

	uint16_t major_xkb_version_out;
	uint16_t minor_xkb_version_out;
	uint8_t base_event_out;
	uint8_t base_error_out;

	int32_t ret 
  = xkb_x11_setup_xkb_extension(connection,
                                XKB_X11_MIN_MAJOR_XKB_VERSION,
                                XKB_X11_MIN_MINOR_XKB_VERSION,
                                XKB_X11_SETUP_XKB_EXTENSION_NO_FLAGS,
                                &major_xkb_version_out,
                                &minor_xkb_version_out,
                                &base_event_out,
                                &base_error_out);
  if (ret != 1)
  {
    debug_msgnl("xkb_x11_setup_xkb_extension() error occurred")
    goto error;
  }

	int32_t core_keyboard_device_id = xkb_x11_get_core_keyboard_device_id(connection);
  if (core_keyboard_device_id == -1)
  {
    debug_msgnl("xkb_x11_get_core_keyboard_device_id() error occurred")
    goto error;
  }
  

	struct xkb_keymap* x11_keymap 
  = xkb_x11_keymap_new_from_device( context, 
                                    connection, 
                                    core_keyboard_device_id, 
                                    XKB_KEYMAP_COMPILE_NO_FLAGS);
  if (x11_keymap == NULL)
	{
    debug_msgnl("No x11_keymap for libxkbcommon-x11");
    goto error;
	}
	
	xkb_layout_index_t num_layouts_x11 = xkb_keymap_num_layouts(x11_keymap);
	debug_msg("num_layouts_x11: %d\n", num_layouts_x11);
  debug_msgnl("")

  // only for Gnome decrease num_layouts_x11, Gnome little buggy :)
  if (system("env | grep XDG_SESSION_DESKTOP | grep gnome > /dev/null")
  ==  0)
  {
    num_layouts_x11 -= 1;
  }

  XkbStateRec state_return;
  if (XkbGetState(xkb_display, XkbUseCoreKbd, &state_return)
  != Success)
  {
    debug_msgnl("XkbGetState() error occurred")
    goto error;
  }

  debug_msg("state_return->group:          %d\n",  state_return.group);
  debug_msg("state_return->base_group:     %d\n",  state_return.base_group);
  debug_msg("state_return->locked_group:   %d\n",  state_return.locked_group);
  debug_msg("state_return->latched_group:  %d\n",  state_return.latched_group);
  debug_msgnl("")

  for (xkb_layout_index_t i = 0; i < num_layouts_x11; i++)
	{
		printf("index: %d layout name: %s", i, xkb_keymap_layout_get_name(x11_keymap, i)); 
    if (i == state_return.locked_group)
    {
      printf(" <---current layout\n");
    }
    else
    {
      printf("\n");
    }
	}

  return 0;

error:
  xcb_disconnect(connection);
  XCloseDisplay(xkb_display);
  XCloseDisplay(x11_display);
  return -1;
}
