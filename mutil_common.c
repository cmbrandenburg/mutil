/* vim: tabstop=4:shiftwidth=4:expandtab:tw=80
 */

#include "mutil_common.h"

void mutil_print_warning(
        gboolean enable_flag,
        gchar const *format,
        ...)
{
    gchar *msg;
    va_list varg_list;

    if (enable_flag) {

        va_start(varg_list, format);
        msg = g_strdup_vprintf(format, varg_list);
        va_end(varg_list);

        g_fprintf(stderr, "*** (warning): %s\n", msg);

        g_free(msg);
    }

    return;
} /* mutil_print_warning */

