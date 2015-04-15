/* vim: tabstop=4:shiftwidth=4:expandtab:tw=80
 */

#ifndef mutil_track_h
#define mutil_track_h

#include "mutil_audio_file.h"
#include "mutil_common.h"
#include "mutil_tag.h"

struct mutil_track;
typedef struct mutil_track mutil_track_t;

/* track: */

void mutil_track_add_tag(
        mutil_track_t *track,
        mutil_tag_t *tag);

void mutil_track_add_tag_list(
        mutil_track_t *track,
        GList *tag_list);

mutil_track_t *mutil_track_alloc(
        gchar const *audio_filename,
        mutil_audio_type_t audio_type);

mutil_track_t *mutil_track_copy(
        mutil_track_t *track);

GList *mutil_track_copy_list_of(
        GList *track_list);

GList *mutil_track_create_tag_list(
        mutil_track_t *track);

gchar *mutil_track_format_archive_encode_command(
        mutil_track_t *track,
        gchar const *tgt_filename,
        gboolean opt_flag_use_echo_e);

gchar *mutil_track_format_decode_command(
        mutil_track_t *track);

gchar *mutil_track_format_ogg_encode_command(
        mutil_track_t *track,
        gchar const *tgt_filename,
        gboolean opt_flag_use_echo_e);

void mutil_track_free(
        mutil_track_t *track);

void mutil_track_free_list_of(
        GList *track_list);

gchar const *mutil_track_get_filename(
        mutil_track_t const * const track);

gchar const *mutil_track_get_first_tag_value_by_name(
        mutil_track_t *track,
        gchar const *tag_name);

gboolean mutil_track_has_duplicate_tags(
        mutil_track_t *track,
        gchar const *tag_name);

gboolean mutil_track_has_tag(
        mutil_track_t *track,
        gchar const *tag_name);

/* track list: */

void mutil_track_list_generate_track_number_tags(
        GList *track_list);

#endif /* #ifndef mutil_track_h */

