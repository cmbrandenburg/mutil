/* vim: tabstop=4:shiftwidth=4:expandtab:tw=80
 */

#ifndef mutil_audio_file_h
#define mutil_audio_file_h

#include "mutil_common.h"

/* New audio types require implementation support added to:
 *   - mutil_create_track_from_audio_file
 *   - mutil_determine_file_audio_type
 */
typedef enum {
    mutil_audio_type_native,
    mutil_audio_type_flac
} mutil_audio_type_t;

/* Creates a list of mutil_track_t objects, one object for each audio file.
 *
 * Returns: NULL on error.
 */ 
gint mutil_create_track_list_from_audio_files(
        gchar const * const *audio_filenames,
        gint audio_filename_cnt,
        GList **o_track_list,
        GError **o_error);

gint mutil_determine_file_audio_type(
        gchar const *audio_filename,
        mutil_audio_type_t *o_audio_type,
        GError **o_error);

#endif /* #ifndef mutil_audio_file_h */

