/* vim: tabstop=4:shiftwidth=4:expandtab:tw=80
 */

#include "mutil_audio_file.h"
#include "mutil_track.h"
#include <FLAC/all.h>

static mutil_track_t *mutil_create_track_from_audio_file(
        gchar const *audio_filename,
        GError **o_error);

static mutil_track_t *mutil_create_track_from_audio_file__flac(
        gchar const *audio_filename,
        GError **o_error);

mutil_track_t *mutil_create_track_from_audio_file(
        gchar const *audio_filename,
        GError **o_error)
{
    mutil_track_t *new_track = NULL;

    g_assert(audio_filename != NULL);
    g_assert(o_error == NULL || *o_error == NULL);

    /* Try to extract meta-information from the audio file treating the file as
     * supported audio file types until one works. */

    /* FLAC: */
    if (new_track == NULL) {
        new_track = mutil_create_track_from_audio_file__flac(
                audio_filename,
                NULL);
    }

    /* If none of the supported audio file types worked then the file is assumed
     * to be a "native" type and no tags are created for it. (This is the case
     * that is used for WAV files.) */

    if (new_track == NULL) {
        new_track = mutil_track_alloc(audio_filename, mutil_audio_type_native);
    }

    g_assert(o_error == NULL || *o_error == NULL);
    g_assert(new_track != NULL);
    return new_track;
} /* mutil_create_track_from_audio_file */

mutil_track_t *mutil_create_track_from_audio_file__flac(
        gchar const *audio_filename,
        GError **o_error)
{
    mutil_track_t *new_track = NULL;
    FLAC__Metadata_SimpleIterator *metadata_iter = NULL;
    FLAC__bool flac_status;
    FLAC__bool iter_done;
    FLAC__MetadataType metadata_type;
    FLAC__StreamMetadata *metadata_block = NULL;
    gint comment_i;
    mutil_tag_t *new_tag = NULL;

    g_assert(audio_filename != NULL);
    g_assert(o_error == NULL || *o_error == NULL);

    /* Open the file by attaching an metadata iterator to it. */

    g_assert(metadata_iter == NULL);
    metadata_iter = FLAC__metadata_simple_iterator_new();
    if (metadata_iter == NULL) {
        g_set_error(
                o_error,
                mutil_error_domain,
                mutil_error_code_undefined,
                "failed to create FLAC metadata iterator");
        goto error_handling;
    }
    
    flac_status = FLAC__metadata_simple_iterator_init(
            metadata_iter,
            audio_filename,
            true,
            false);
    if (!flac_status) {
        g_set_error(
                o_error,
                mutil_error_domain,
                mutil_error_code_undefined,
                "failed to initialize FLAC metadata iterator to file '%s'--"
                "most likely the file cannot be opened or else is not a valid "
                "FLAC file",
                audio_filename);
        goto error_handling;
    }

    new_track = mutil_track_alloc(audio_filename, mutil_audio_type_flac);

    /* Traverse all metadata blocks and retrieve ones that are Vorbis comments.
     * */

    do {

        metadata_type = FLAC__metadata_simple_iterator_get_block_type(
                metadata_iter);

        if (metadata_type == FLAC__METADATA_TYPE_VORBIS_COMMENT) {

            if (metadata_block != NULL) {
                FLAC__metadata_object_delete(metadata_block);
            }
            metadata_block = FLAC__metadata_simple_iterator_get_block(
                    metadata_iter);
            if (metadata_block == NULL) {
                g_set_error(
                        o_error,
                        mutil_error_domain,
                        mutil_error_code_undefined,
                        "failed to get metadata block from file '%s'",
                        audio_filename);
                goto error_handling;
            }

            for (comment_i = 0;
                 comment_i < metadata_block->data.vorbis_comment.num_comments;
                 comment_i++) {

                g_assert(new_tag == NULL);
                new_tag = mutil_tag_create_from_simple_assignment(
                        (gchar const *) metadata_block->data.vorbis_comment.comments[comment_i].entry,
                        '=',
                        NULL);
                if (new_tag != NULL) {
                    mutil_track_add_tag(new_track, new_tag);
                    new_tag = NULL;
                }
            }
        }

        iter_done = !FLAC__metadata_simple_iterator_next(metadata_iter);

    } while (!iter_done);

    g_assert(o_error == NULL || *o_error == NULL);
    g_assert(new_track != NULL);
    goto cleanup;

error_handling:

    g_assert(o_error == NULL || *o_error != NULL);

    mutil_track_free(new_track);
    new_track = NULL;

cleanup:

    if (metadata_iter != NULL) {
        FLAC__metadata_simple_iterator_delete(metadata_iter);
    }

    if (metadata_block != NULL) {
        FLAC__metadata_object_delete(metadata_block);
    }

    mutil_tag_free(new_tag);

    return new_track;
} /* mutil_create_track_from_audio_file__flac */

gint mutil_create_track_list_from_audio_files(
        gchar const * const *audio_filenames,
        gint audio_filename_cnt,
        GList **o_track_list,
        GError **o_error)
{
    gint ret_value;
    GList *new_track_list = NULL;
    gint audio_filename_i;
    mutil_track_t *new_track = NULL;

    g_assert(o_track_list != NULL);
    g_assert(o_error == NULL || *o_error == NULL);

    for (audio_filename_i = 0;
         audio_filename_i < audio_filename_cnt;
         audio_filename_i++) {

        mutil_track_free(new_track);
        new_track = mutil_create_track_from_audio_file(
                audio_filenames[audio_filename_i],
                o_error);
        if (new_track == NULL) {
            goto error_handling;
        }

        new_track_list = g_list_append(new_track_list, new_track);
        new_track = NULL;
    }

    g_assert(o_error == NULL || *o_error == NULL);
    *o_track_list = new_track_list;
    new_track_list = NULL;
    ret_value = 0;
    goto cleanup;

error_handling:

    g_assert(o_error == NULL || *o_error != NULL);

    ret_value = -1;

cleanup:

    mutil_track_free(new_track);
    mutil_track_free_list_of(new_track_list);

    return ret_value;
} /* mutil_create_track_list_from_audio_files */

gint mutil_determine_file_audio_type(
        gchar const *audio_filename,
        mutil_audio_type_t *o_audio_type,
        GError **o_error)
{
    gint ret_value;
    mutil_track_t *new_track = NULL;
    mutil_audio_type_t audio_type;

    g_assert(audio_filename != NULL);
    g_assert(o_error == NULL || *o_error == NULL);

    if (new_track == NULL) {
        new_track = mutil_create_track_from_audio_file__flac(
                audio_filename,
                NULL);
        if (new_track != NULL) {
            audio_type = mutil_audio_type_flac;
        }
    }

    if (new_track == NULL) {
        audio_type = mutil_audio_type_native;
    }

    g_assert(o_error == NULL || *o_error == NULL);
    *o_audio_type = audio_type;
    ret_value = 0;
    goto cleanup;

cleanup:

    mutil_track_free(new_track);

    return ret_value;
} /* mutil_determine_file_audio_type */

