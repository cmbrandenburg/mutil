/* vim: tabstop=4:shiftwidth=4:expandtab:tw=80
 */

#ifndef mutil_common_h
#define mutil_common_h

#include <glib.h>
#include <glib/gprintf.h>
#include <libxml/tree.h>
#include <stdlib.h>
#include <string.h>

#define mutil_error_domain g_quark_from_static_string("mutil")

enum {
    mutil_error_code_ok,
    mutil_error_code_undefined
};

void mutil_print_warning(
        gboolean enable_flag,
        gchar const *format,
        ...);

#endif /* #ifndef mutil_common_h */

