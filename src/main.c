#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <getopt.h>
#include <assert.h>

#include <X11/Xlib.h>
#include <X11/XKBlib.h>

#include <xkbcommon/xkbcommon-x11.h>

/* ── Shared XKB session context ─────────────────────────────────────── */

/** @brief XKB session state.
 *
 * Holds all resources required for keyboard layout operations:
 * the X11 display connection, xkbcommon context and keymaps,
 * the XCB connection needed to query the live XKB configuration,
 * and the number of layouts discovered on this session.
 */
typedef struct xkb_session
{
  Display*           display;      /**< X11 display connection. */
  struct xkb_context* context;     /**< xkbcommon context for keymap compilation. */
  struct xkb_keymap* keymap;       /**< Default system keymap (built from names). */
  struct xkb_keymap* x11_keymap;   /**< Live keymap captured from the X11 device. */
  xcb_connection_t*  connection;   /**< XCB connection for extension setup / device ID. */
  xkb_layout_index_t num_layouts;  /**< Number of layouts in @p x11_keymap. */
} xkb_session_t;

/* ── Forward declarations ───────────────────────────────────────────── */

static int xkb_session_init(xkb_session_t* session);
static void xkb_session_free(xkb_session_t* session);

static int xkb_groups_list(void);

/* ── Enum for lock direction (used only by xkb_groups_lock) ───────── */

/** @brief Direction of layout switch.
 *
 * Passed to xkb_groups_lock() to indicate whether the next or previous
 * keyboard group should be selected. Uses simple wrap-around semantics:
 * NEXT advances and wraps to 0; PREV decrements and wraps to the last index.
 */
typedef enum xkb_lock_direction
{
  XKB_LOCK_NEXT,   /**< Advance to the next layout (wraps to first). */
  XKB_LOCK_PREV    /**< Go back to the previous layout (wraps to last). */
} xkb_lock_direction_t;

static int xkb_groups_lock(xkb_lock_direction_t dir);

static int options_handler(int argc,
                           const char* argv[]);

/* ── Global flags (set by CLI options) ──────────────────────────────── */

static bool g_debug_flag     = false;
static bool g_gnome_override = false;

/* ── Debug infrastructure — msg→stdout, err→stderr ─────────────────── */

/*! * @brief Core debug logging helper.
 *
 * Formats a timestamped, structured log line. Messages go to stdout;
 * errors (is_error == true) additionally append errno details and source
 * location, and are written to stderr.
 *
 * @param[in] component  Logical subsystem tag (e.g. "init", "switch").
 * @param[in] action     Action/event descriptor.
 * @param[in] fmt        printf-style format string (may be empty).
 * @param[in] ap         va_list of optional arguments for fmt.
 * @param[in] is_error   If true, append errno and source location.
 * @param[in] file       Source file name (__FILE__).
 * @param[in] func       Function name (__func__).
 * @param[in] line       Line number (__LINE__).
 */
static void debug_msgnl_helper(const char* component,
                               const char* action,
                               const char* fmt,
                               va_list     ap,
                               bool        is_error,
                               const char* file,
                               const char* func,
                               int         line)
{
  if (!g_debug_flag)
  {
    return;
  }

  time_t now = time(NULL);
  struct tm* tm_info = localtime(&now);
  if (tm_info == NULL)
  {
    return;
  }

  char ts[26];
  strftime(ts, sizeof(ts), "%Y-%m-%dT%H:%M:%S", tm_info);

  FILE* stream = is_error ? stderr : stdout;

  fprintf(stream,
          "wxkb-switch dbg ts=%s component=%s action=%s",
          ts, component, action);

  if (fmt != NULL && fmt[0] != '\0')
  {
    fprintf(stream, ",");
    vfprintf(stream, fmt, ap);
  }

  if (is_error)
  {
    fprintf(stderr,
            " errno=%d errno_msg=\"%s\" file=%s func=%s line=%d",
            errno, strerror(errno), file, func, line);
  }

  fputc('\n', stream);
}

#define debug_msg(...) \
    do { \
      if (g_debug_flag) { \
        fprintf(stdout, __VA_ARGS__); \
      } \
    } while (0)

/* ── Thin inline wrappers — handle va_list for the helper ─────────── */

/*! * @brief Internal variadic wrapper for debug logging.
 *
 * Called by the debug_msgnl macro. Does not include error metadata.
 */
static inline void _debug_log(const char* component,
                              const char* action,
                              const char* fmt,
                              ...)
{
  va_list ap;
  va_start(ap, fmt);
  debug_msgnl_helper(component, action, fmt, ap, false, NULL, NULL, 0);
  va_end(ap);
}

/*! * @brief Internal variadic wrapper for error logging.
 *
 * Called by the debug_err macro with source location info captured at
 * the call site (via __FILE__, __func__, __LINE__ in the macro).
 */
static inline void _debug_log_err(const char* component,
                                  const char* action,
                                  const char* fmt,
                                  const char* file,
                                  const char* func,
                                  int         line,
                                  ...)
{
  va_list ap;
  va_start(ap, line);
  debug_msgnl_helper(component, action, fmt, ap, true,
                     file, func, line);
  va_end(ap);
}

/* ── Public macros — capture source location at call site ─────────── */

/*! * @brief Log a structured debug message.
 *
 * Usage: debug_msgnl("component", "action", "fmt", ...);
 * The fmt argument may be an empty string if no extra data is needed.
 */
#define debug_msgnl(component, action, fmt, ...) \
    _debug_log((component), (action), (fmt), ##__VA_ARGS__)

/*! * @brief Log a structured error message with errno and source location.
 *
 * Automatically captures __FILE__, __func__, __LINE__ at the call site
 * and the current errno value. Use this at every error site for maximum
 * traceability.
 */
#define debug_err(component, action, fmt, ...) \
    _debug_log_err((component), (action), (fmt), \
                   __FILE__, __func__, (int)__LINE__, ##__VA_ARGS__)


/* ── GNOME detection via getenv (no fork) ───────────────────────────── */

/** @brief Detect whether the current session is running under GNOME.
 *
 * Reads the XDG_SESSION_DESKTOP environment variable and checks for a
 * case-sensitive substring "gnome". This lightweight check avoids forking
 * to external commands such as loginctl or dbus calls.
 *
 * @return true if GNOME appears to be active, false otherwise.
 */
static bool detect_gnome(void)
{
  const char* desktop = getenv("XDG_SESSION_DESKTOP");
  return (desktop != NULL && strstr(desktop, "gnome") != NULL);
}

/* ── XKB session initialization ─────────────────────────────────────── */

/** @brief Initialise the full XKB session.
 *
 * Performs the following steps in order:
 * 1. Opens the default X11 display via XOpenDisplay().
 * 2. Queries the XKB extension version and capability.
 * 3. Creates an xkbcommon context and builds a default keymap from names.
 * 4. Opens an XCB connection and sets up the XKB extension on it.
 * 5. Retrieves the core keyboard device ID.
 * 6. Builds a live keymap from that X11 device (this is the authoritative
 *    source of layout information).
 *
 * On any failure the partially initialised session is cleaned up via
 * xkb_session_free() before returning EXIT_FAILURE.
 *
 * @param[out] session  Populated session structure (must not be NULL).
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on error.
 */
static int xkb_session_init(xkb_session_t* session)
{
  assert(session != NULL);
  memset(session, 0, sizeof(*session));

  /* Open primary X11 display */
  char* display_name = "";
  session->display = XOpenDisplay(display_name);
  if (session->display == NULL)
  {
    debug_err("init", "display_open_failed", "");
    return EXIT_FAILURE;
  }
  debug_msgnl("init", "display_opened", "");

  /* Query XKB extension */
  int32_t opcode_rtrn, event_rtrn, error_rtrn;
  int32_t major = XkbMajorVersion;
  int32_t minor = XkbMinorVersion;

  if (XkbQueryExtension(session->display,
                        &opcode_rtrn,
                        &event_rtrn,
                        &error_rtrn,
                        &major,
                        &minor) != True)
  {
    debug_err("init", "xkb_query_failed", "");
    xkb_session_free(session);
    return EXIT_FAILURE;
  }
  debug_msgnl("init", "xkb_queried", "opcode=%d event=%d error=%d",
              opcode_rtrn, event_rtrn, error_rtrn);

  /* Create xkbcommon context */
  session->context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
  if (session->context == NULL)
  {
    debug_err("init", "context_create_failed", "");
    xkb_session_free(session);
    return EXIT_FAILURE;
  }

  /* Build default keymap */
  session->keymap = xkb_keymap_new_from_names(session->context,
                                              NULL,
                                              XKB_KEYMAP_COMPILE_NO_FLAGS);
  if (session->keymap == NULL)
  {
    debug_err("init", "keymap_create_failed", "");
    xkb_session_free(session);
    return EXIT_FAILURE;
  }

  xkb_layout_index_t num_layouts = xkb_keymap_num_layouts(session->keymap);
  debug_msgnl("init", "default_keymap_layouts", "count=%d", num_layouts);

  /* Connect via XCB */
  session->connection = xcb_connect(NULL, NULL);
  if (session->connection == NULL)
  {
    debug_err("init", "xcb_connect_failed", "");
    xkb_session_free(session);
    return EXIT_FAILURE;
  }
  debug_msgnl("init", "xcb_connected", "");

  /* Setup XKB extension on XCB */
  uint16_t major_out, minor_out;
  uint8_t  base_event, base_error;

  int32_t ret = xkb_x11_setup_xkb_extension(session->connection,
                                            XKB_X11_MIN_MAJOR_XKB_VERSION,
                                            XKB_X11_MIN_MINOR_XKB_VERSION,
                                            XKB_X11_SETUP_XKB_EXTENSION_NO_FLAGS,
                                            &major_out,
                                            &minor_out,
                                            &base_event,
                                            &base_error);
  if (ret != 1)
  {
    debug_msgnl("init", "xkb_setup_failed", "ret=%d", ret);
    xkb_session_free(session);
    return EXIT_FAILURE;
  }
  debug_msgnl("init", "xkb_setup_done",
              "version=%u.%u event_base=%u error_base=%u",
              major_out, minor_out, base_event, base_error);

  /* Get core keyboard device ID */
  int32_t device_id = xkb_x11_get_core_keyboard_device_id(session->connection);
  if (device_id == -1)
  {
    debug_err("init", "core_kbd_device_failed", "");
    xkb_session_free(session);
    return EXIT_FAILURE;
  }
  debug_msgnl("init", "core_kbd_device", "id=%d", device_id);

  /* Build keymap from X11 device */
  session->x11_keymap = xkb_x11_keymap_new_from_device(session->context,
                                                       session->connection,
                                                       device_id,
                                                       XKB_KEYMAP_COMPILE_NO_FLAGS);
  if (session->x11_keymap == NULL)
  {
    debug_err("init", "x11_keymap_failed", "");
    xkb_session_free(session);
    return EXIT_FAILURE;
  }

  session->num_layouts = xkb_keymap_num_layouts(session->x11_keymap);
  debug_msgnl("init", "x11_keymap_layouts", "count=%d", session->num_layouts);

  /* Log all layouts */
  for (xkb_layout_index_t i = 0; i < session->num_layouts; i++)
  {
    debug_msgnl("init", "layout",
                "index=%d name=%s",
                i,
                xkb_keymap_layout_get_name(session->x11_keymap, i));
  }

  return EXIT_SUCCESS;
}

/* ── XKB session cleanup ────────────────────────────────────────────── */

/** @brief Release all resources held by an XKB session.
 *
 * Unrefs keymaps and context, disconnects the XCB connection, and closes
 * the X11 display. Safe to call on a partially initialised or fully zeroed
 * session — every pointer is checked before use.
 *
 * @param[in,out] session  Session to release (must not be NULL).
 */
static void xkb_session_free(xkb_session_t* session)
{
  assert(session != NULL);

  if (session->x11_keymap != NULL)
  {
    xkb_keymap_unref(session->x11_keymap);
  }
  if (session->keymap != NULL)
  {
    xkb_keymap_unref(session->keymap);
  }
  if (session->context != NULL)
  {
    xkb_context_unref(session->context);
  }
  if (session->connection != NULL)
  {
    xcb_disconnect(session->connection);
  }
  if (session->display != NULL)
  {
    XCloseDisplay(session->display);
  }
}

/* ── main ───────────────────────────────────────────────────────────── */

/** @brief Program entry point.
 *
 * Delegates all argument parsing and action dispatch to options_handler().
 * If the handler returns an error code, logs a debug message before exiting.
 *
 * @param[in]  argc Argument count.
 * @param[in]  argv Argument vector.
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on error.
 */
int main(int argc,
         const char* argv[])
{
  if (options_handler(argc, argv) != EXIT_SUCCESS)
  {
    debug_err("main", "handler_error", "");
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

/* ── Options handler ────────────────────────────────────────────────── */

/*! * @brief Process program options and invoke corresponding actions.
 *
 * Uses getopt_long for argument parsing. Sets g_debug_flag when -d/--debug
 * is provided. With no arguments, defaults to switching to the next layout.
 *
 * @param[in]  argc Argument count.
 * @param[in]  argv Argument vector.
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on error.
 */
static int options_handler(int argc,
                           const char* argv[])
{
  assert(argv != NULL);

  /* ── Pre-scan for --gnome/-g to allow override before blocking ─ */

  for (int i = 1; i < argc; i++)
  {
    if (strcmp(argv[i], "-g") == 0 || strcmp(argv[i], "--gnome") == 0)
    {
      g_gnome_override = true;
      break;
    }
  }

  /* ── Early GNOME detection — before any X11 init or action ───── */

  if (!g_gnome_override && detect_gnome())
  {
    fprintf(stderr,
            "wxkb-switch: GNOME detected. Layout switching is not "
            "supported under GNOME because its settings-daemon manages "
            "layouts independently and may override external changes.\n"
            "Use -g/--gnome to force execution anyway.\n");
    return EXIT_FAILURE;
  }

  debug_msgnl("cli", "args_count", "argc=%d", argc);

  static struct option long_options[] =
  {
    { "debug",   no_argument, 0, 'd' },
    { "gnome",   no_argument, 0, 'g' },
    { "help",    no_argument, 0, 'h' },
    { "next",    no_argument, 0, 'n' },
    { "prev",    no_argument, 0, 'p' },
    { "list",    no_argument, 0, 'l' },
    { "version", no_argument, 0, 'v' },
    { 0, 0, 0, 0 }
  };

  int32_t option_index = 0;

  while (true)
  {
    int32_t c = getopt_long(argc,
                            (char* const*)argv,
                            "dghnplv",
                            long_options,
                            &option_index);

    if (c == -1)
    {
      break;
    }

    switch (c)
    {
      case 0:
      {
        if (long_options[option_index].flag != 0)
        {
          break;
        }
        debug_msg("option %s", long_options[option_index].name);
        if (optarg != NULL)
        {
          debug_msg(" with arg %s", optarg);
        }
      }
      break;

      case 'd':
      {
        g_debug_flag = true;
        debug_msgnl("cli", "debug_enabled", "");
      }
      break;

      case 'g':
      {
        g_gnome_override = true;
        debug_msgnl("cli", "gnome_override_enabled", "");
      }
      break;

      case 'h':
      {
        puts("Usage: wxkb-switch [options]");
        puts("Options:");
        puts("  -h, --help            Display this help message.");
        puts("  -g, --gnome           Force execution under GNOME.");
        puts("  -n, --next            Set next xkb layout.");
        puts("  -p, --prev            Set previous xkb layout.");
        puts("  -l, --list            List available layouts.");
        puts("  -v, --version         Print program version.");
        puts("  -d, --debug           Enable debug mode.");
        puts("");
        puts("See also:");
        puts("man wxkb-switch");
        puts("sudo actkbd --help");
        puts("man xkb-switch");
      }
      break;

      case 'n':
      {
        if (xkb_groups_lock(XKB_LOCK_NEXT) != EXIT_SUCCESS)
        {
          debug_err("cli", "next_failed", "");
          return EXIT_FAILURE;
        }
      }
      break;

      case 'p':
      {
        if (xkb_groups_lock(XKB_LOCK_PREV) != EXIT_SUCCESS)
        {
          debug_err("cli", "prev_failed", "");
          return EXIT_FAILURE;
        }
      }
      break;

      case 'l':
      {
        if (xkb_groups_list() != EXIT_SUCCESS)
        {
          debug_err("cli", "list_failed", "");
          return EXIT_FAILURE;
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
        /* getopt_long already printed an error message */
        return EXIT_FAILURE;
      }

      default:
      {
        abort();
      }
    }
  }

  /* Default action: switch to next layout when no options given */
  if (argc == 1)
  {
    debug_msgnl("cli", "default_action", "action=next");
    if (xkb_groups_lock(XKB_LOCK_NEXT) != EXIT_SUCCESS)
    {
      debug_err("cli", "next_failed", "");
      return EXIT_FAILURE;
    }
  }

  return EXIT_SUCCESS;
}

/* ── Unified lock helper ───────────────────────────────────────────── */

/*! * @brief Lock keyboard group in the given direction.
 *
 * Shared implementation for next/prev layout switching. Calculates the
 * target group with wrap-around semantics, locks it, and verifies.
 *
 * @param[in] dir    Direction: XKB_LOCK_NEXT or XKB_LOCK_PREV.
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on error.
 */
static int xkb_groups_lock(xkb_lock_direction_t dir)
{
  const char* tag = (dir == XKB_LOCK_NEXT) ? "next" : "prev";

  debug_msgnl("switch", "%s_start", tag);

  xkb_session_t session;
  if (xkb_session_init(&session) != EXIT_SUCCESS)
  {
    return EXIT_FAILURE;
  }

  XkbStateRec state;
  if (XkbGetState(session.display, XkbUseCoreKbd, &state) != Success)
  {
    debug_err("switch", "get_state_failed", "");
    xkb_session_free(&session);
    return EXIT_FAILURE;
  }

  debug_msgnl("switch", tag,
               "group=%d base_group=%d locked_group=%d latched_group=%d",
               state.group, state.base_group,
               state.locked_group, state.latched_group);

  /* Calculate target group with wrap-around */
  xkb_layout_index_t target;

  switch (dir)
  {
    case XKB_LOCK_NEXT:
    {
      target = state.locked_group + 1;
      if (target >= session.num_layouts)
      {
        target = 0;
        debug_msgnl("switch", "wrap_to_first", "");
      }
    }
    break;

    case XKB_LOCK_PREV:
    {
      if (state.locked_group == 0)
      {
        target = session.num_layouts - 1;
        debug_msgnl("switch", "wrap_to_last", "group=%d", target);
      }
      else
      {
        target = state.locked_group - 1;
      }
    }
    break;
  }

  if (XkbLockGroup(session.display, XkbUseCoreKbd, target) != True)
  {
    debug_err("switch", "lock_group_failed", "target=%d", target);
    xkb_session_free(&session);
    return EXIT_FAILURE;
  }
  debug_msgnl("switch", "locked", "group=%d", target);

  /* Verify new state */
  if (XkbGetState(session.display, XkbUseCoreKbd, &state) == Success)
  {
    debug_msgnl("switch", tag,
                "group=%d base_group=%d locked_group=%d latched_group=%d",
                state.group, state.base_group,
                state.locked_group, state.latched_group);
  }

  debug_msgnl("switch", "%s_done", tag);

  xkb_session_free(&session);
  return EXIT_SUCCESS;
}

/* ── List layouts ───────────────────────────────────────────────────── */

/*! * @brief Print all available layouts, marking the current one.
 *
 * Output format: "index: name" with " <---current layout" appended to
 * the active layout line.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on error.
 */
static int xkb_groups_list(void)
{
  debug_msgnl("list", "start", "");

  xkb_session_t session;
  if (xkb_session_init(&session) != EXIT_SUCCESS)
  {
    return EXIT_FAILURE;
  }

  XkbStateRec state;
  if (XkbGetState(session.display, XkbUseCoreKbd, &state) != Success)
  {
    debug_err("list", "get_state_failed", "");
    xkb_session_free(&session);
    return EXIT_FAILURE;
  }

  debug_msgnl("list", "current_group", "group=%d", state.locked_group);

  for (xkb_layout_index_t i = 0; i < session.num_layouts; i++)
  {
    const char* name = xkb_keymap_layout_get_name(session.x11_keymap, i);
    if (i == state.locked_group)
    {
      printf("%d: %s <--- current layout\n", i, name);
    }
    else
    {
      printf("%d: %s\n", i, name);
    }
  }

  debug_msgnl("list", "done", "");
  xkb_session_free(&session);
  return EXIT_SUCCESS;
}
