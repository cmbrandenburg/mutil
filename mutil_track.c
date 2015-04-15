/* vim: tabstop=4:shiftwidth=4:expandtab:tw=80
 */

#include "mutil_track.h"

struct mutil_track {
    gint ref_cnt;
    gchar *filename;
    mutil_audio_type_t audio_type;
    mutil_tag_map_t *tag_map;
};

static gchar *mutil_format_escaped_string(
        gchar const *src_str,
        gboolean opt_flag_escape_newlines,
        gboolean opt_flag_escape_parentheses);

gchar *mutil_format_escaped_string(
        gchar const *src_str,
        gboolean opt_flag_escape_newlines,
        gboolean opt_flag_escape_parentheses)
{
    GString *safe_filename;
    gchar const *src_pos_i;
    gunichar src_ch;

    safe_filename = g_string_new("");

    for (src_pos_i = src_str;
         *src_pos_i != '\0';
         src_pos_i = g_utf8_next_char(src_pos_i)) {

        src_ch = g_utf8_get_char(src_pos_i);

        if (src_ch == '\\' ||
            src_ch == '\"' ||
            src_ch == '\'' ||
            src_ch == '&' ||
            src_ch == '#' ||
            (opt_flag_escape_parentheses && src_ch == '(') ||
            (opt_flag_escape_parentheses && src_ch == ')')) {
            g_string_append_c(safe_filename, '\\');
        }

        if (opt_flag_escape_newlines && src_ch == '\n') {
            /* Add two backslashes: one to replace the raw newline character and
             * another to escape the other backslash. */
            g_string_append(safe_filename, "\\\\n");
        } else {
            g_string_append_unichar(safe_filename, src_ch);
        }
    }

    return g_string_free(safe_filename, FALSE);
} /* mutil_format_escaped_string */

void mutil_track_add_tag(
        mutil_track_t *track,
        mutil_tag_t *tag)
{
    g_assert(track != NULL);
    g_assert(tag != NULL);

    mutil_tag_map_add_tag(track->tag_map, tag);

    return;
} /* mutil_track_add_tag */

void mutil_track_add_tag_list(
        mutil_track_t *track,
        GList *tag_list)
{
    GList *node_i;
    mutil_tag_t *tag_i;

    for (node_i = tag_list;
         node_i != NULL;
         node_i = node_i->next) {
        tag_i = node_i->data;
        mutil_track_add_tag(track, tag_i);
    }

    return;
} /* mutil_track_add_tag_list */

mutil_track_t *mutil_track_alloc(
        gchar const *audio_filename,
        mutil_audio_type_t audio_type)
{
    mutil_track_t *new_track = NULL;

    g_assert(audio_filename != NULL);

    new_track = g_malloc0(sizeof(mutil_track_t));
    new_track->ref_cnt = 1;
    new_track->filename = g_strdup(audio_filename);
    new_track->audio_type = audio_type;
    new_track->tag_map = mutil_tag_map_alloc();

    return new_track;
} /* mutil_track_alloc */

mutil_track_t *mutil_track_copy(
        mutil_track_t *track)
{
    g_assert(track != NULL);

    track->ref_cnt++;

    return track;
} /* mutil_copy_track */

GList *mutil_track_copy_list_of(
        GList *track_list)
{
    GList *new_track_list = NULL;
    GList *node_i;
    mutil_track_t *track_i;
    mutil_track_t *new_track = NULL;

    for (node_i = track_list;
         node_i != NULL;
         node_i = node_i->next) {

        track_i = node_i->data;
        g_assert(new_track == NULL);
        new_track = mutil_track_copy(track_i);
        new_track_list = g_list_append(new_track_list, new_track);
        new_track = NULL;
    }

    return new_track_list;
} /* mutil_track_copy_list_of */

GList *mutil_track_create_tag_list(
        mutil_track_t *track)
{
    g_assert(track != NULL);

    return mutil_tag_map_create_list(track->tag_map);
} /* mutil_track_create_tag_list */

gchar *mutil_track_format_archive_encode_command(
        mutil_track_t *track,
        gchar const *tgt_filename,
        gboolean opt_flag_use_echo_e)
{
    GString *new_cmd = NULL;
    GList *tag_list;
    GList *node_i;
    mutil_tag_t *tag_i;
    gchar const *name_text;
    gchar const *value_text;
    gchar *safe_value_text = NULL;

    g_assert(track != NULL);
    g_assert(tgt_filename != NULL);

    tag_list = mutil_track_create_tag_list(track);

    new_cmd = g_string_new("flac --silent");
    g_string_append_printf(new_cmd, " --output-name=\"%s\"", tgt_filename);
    g_string_append(new_cmd, " --best");

    for (node_i = tag_list;
         node_i != NULL;
         node_i = node_i->next) {

        tag_i = node_i->data;

        name_text = mutil_tag_get_name(tag_i);
        value_text = mutil_tag_get_value(tag_i);

        g_free(safe_value_text);
        safe_value_text = mutil_format_escaped_string(value_text, TRUE, TRUE);

        /* The "echo" command is used to allow for newline characters within a
         * tag value. A raw newline character cannot exist within a valid
         * makefile, nor will make interpret it if escaped. */
        g_string_append_printf(
                new_cmd,
                " --tag=%s=\"$$(echo %s%s)\"",
                name_text,
                opt_flag_use_echo_e ? "-e " : "",
                safe_value_text);
    }

    g_string_append(new_cmd, " -");

    /* Clean up. */

    mutil_tag_free_list_of(tag_list);
    g_free(safe_value_text);

    g_assert(new_cmd != NULL);
    return g_string_free(new_cmd, FALSE);
} /* mutil_track_format_archive_encode_command */

gchar *mutil_track_format_decode_command(
        mutil_track_t *track)
{
    gchar *new_cmd = NULL;

    g_assert(track != NULL);

    switch (track->audio_type) {
        case mutil_audio_type_flac:
            new_cmd = g_strdup_printf(
                    "flac --decode --silent --stdout \"%s\"",
                    track->filename);
            break;
        case mutil_audio_type_native:
            new_cmd = g_strdup_printf("cat \"%s\"", track->filename);
            break;
    }

    g_assert(new_cmd != NULL);
    return new_cmd;
} /* mutil_track_format_decode_command */

gchar *mutil_track_format_ogg_encode_command(
        mutil_track_t *track,
        gchar const *tgt_filename,
        gboolean opt_flag_use_echo_e)
{
    GString *new_cmd = NULL;
    GList *tag_list;
    GList *node_i;
    mutil_tag_t *tag_i;
    gchar const *name_text;
    gchar const *value_text;
    gchar *safe_value_text = NULL;

    g_assert(track != NULL);
    g_assert(tgt_filename != NULL);

    tag_list = mutil_track_create_tag_list(track);

    new_cmd = g_string_new("oggenc --quiet --bitrate=128");
    g_string_append_printf(new_cmd, " --output=\"%s\"", tgt_filename);

    for (node_i = tag_list;
         node_i != NULL;
         node_i = node_i->next) {

        tag_i = node_i->data;

        name_text = mutil_tag_get_name(tag_i);
        value_text = mutil_tag_get_value(tag_i);

        g_free(safe_value_text);
        safe_value_text = mutil_format_escaped_string(value_text, TRUE, TRUE);

        /* The "echo" command is used to allow for newline characters within a
         * tag value. A raw newline character cannot exist within a valid
         * makefile, nor will make interpret it if escaped. */
        g_string_append_printf(
                new_cmd,
                " --comment=%s=\"$$(echo %s%s)\"",
                name_text,
                opt_flag_use_echo_e ? "-e " : "",
                safe_value_text);
    }

    g_string_append(new_cmd, " -");

    mutil_tag_free_list_of(tag_list);
    g_free(safe_value_text);

    g_assert(new_cmd != NULL);
    return g_string_free(new_cmd, FALSE);
} /* mutil_track_format_ogg_encode_command */

void mutil_track_free(
        mutil_track_t *track)
{
    if (track != NULL && --track->ref_cnt == 0) {
        mutil_tag_map_free(track->tag_map);
        g_free(track->filename);
        g_free(track);
    }

    return;
} /* mutil_track_free */

void mutil_track_free_list_of(
        GList *track_list)
{
    while (track_list != NULL) {
        mutil_track_free(track_list->data);
        track_list = g_list_delete_link(track_list, track_list);
    }

    return;
} /* mutil_track_free_list_of */

gchar const *mutil_track_get_filename(
        mutil_track_t const * const track)
{
    g_assert(track != NULL);

    return track->filename;
} /* mutil_track_get_filename */

gchar const *mutil_track_get_first_tag_value_by_name(
        mutil_track_t *track,
        gchar const *tag_name)
{
    GList *tag_bucket;
    mutil_tag_t *tag;
    gchar const *tag_value = NULL;

    g_assert(track != NULL);
    g_assert(tag_name != NULL);

    tag_bucket = mutil_tag_map_look_up(track->tag_map, tag_name);
    if (tag_bucket != NULL) {
        tag = tag_bucket->data;
        tag_value = mutil_tag_get_value(tag);
    }

    return tag_value;
} /* mutil_track_get_first_tag_value_by_name */

gboolean mutil_track_has_duplicate_tags(
        mutil_track_t *track,
        gchar const *tag_name)
{
    GList *tag_bucket;

    g_assert(track != NULL);
    g_assert(tag_name != NULL);

    tag_bucket = mutil_tag_map_look_up(track->tag_map, tag_name);

    return tag_bucket != NULL && tag_bucket->next != NULL ? TRUE : FALSE;
} /* mutil_track_has_duplicate_tags */

gboolean mutil_track_has_tag(
        mutil_track_t *track,
        gchar const *tag_name)
{
    GList *tag_bucket;

    g_assert(track != NULL);
    g_assert(tag_name != NULL);

    tag_bucket = mutil_tag_map_look_up(track->tag_map, tag_name);

    return tag_bucket != NULL ? TRUE : FALSE;
} /* mutil_track_has_tag */

void mutil_track_list_generate_track_number_tags(
        GList *track_list)
{
    GList *node_i;
    mutil_track_t *track_i;
    gint i;
    mutil_tag_t *new_tag = NULL;
    gchar *new_tag_value = NULL;

    for (node_i = track_list, i = 1;
         node_i != NULL;
         node_i = node_i->next, i++) {

        track_i = node_i->data;

        g_free(new_tag_value);
        new_tag_value = g_strdup_printf("%d", i);

        mutil_tag_free(new_tag);
        new_tag = mutil_tag_alloc(mutil_tag_track_no, new_tag_value);

        mutil_track_add_tag(track_i, new_tag);
    }

    /* Clean up. */

    mutil_tag_free(new_tag);
    g_free(new_tag_value);

    return;
} /* mutil_track_list_generate_track_number_tags */

