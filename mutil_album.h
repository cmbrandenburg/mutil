/* vim: tabstop=4:shiftwidth=4:expandtab:tw=80
 */

#ifndef mutil_album_h
#define mutil_album_h

#include "mutil_common.h"
#include "mutil_makefile.h"

struct mutil_album;
typedef struct mutil_album mutil_album_t;

/* album: */

GList *mutil_album_create_track_list(
        mutil_album_t *album);

/* album list: */

gint mutil_album_list_create_from_track_list(
        GList **o_album_list,
        GList *track_list,
        gboolean opt_flag_simple_album,
        gboolean opt_flag_enable_sanity_warnings,
        gboolean opt_flag_auto_track_no_tags,
        GError **o_error);

GList *mutil_album_list_create_track_list(
        GList *album_list);

void mutil_album_list_free(
        GList *album_list);

mutil_makefile_t *mutil_album_list_generate_archive_makefile(
        GList *album_list,
        gboolean opt_flag_verbose_makefile,
        gboolean opt_flag_use_echo_e);

mutil_makefile_t *mutil_album_list_generate_oggify_makefile(
        GList *album_list,
        gboolean opt_flag_verbose_makefile,
        gboolean opt_flag_use_echo_e);

#endif /* #ifndef mutil_album_h */

