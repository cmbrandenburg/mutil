/* vim: tabstop=4:shiftwidth=4:expandtab:tw=80
 */

#include "mutil_album.h"
#include "mutil_track.h"
#include <math.h>

/* ALL      : not any tag whose presence generates a warning
 * EXPECTED : any tag whose absence generates a warning
 * REQUIRED : any tag whose absence generates an error
 * SINGLE   : any tag whose duplication generates a warning
 */

static gchar const *mutil_tags_all[] = {
    mutil_tag_album,
    mutil_tag_artist,
    mutil_tag_contact,
    mutil_tag_copyright,
    mutil_tag_date,
    mutil_tag_description,
    mutil_tag_genre,
    mutil_tag_isrc,
    mutil_tag_license,
    mutil_tag_location,
    mutil_tag_organization,
    mutil_tag_performer,
    mutil_tag_title,
    mutil_tag_track_no,
    mutil_tag_version,
    NULL
};

static gchar const *mutil_tags_expected[] = {
    mutil_tag_contact,
    mutil_tag_copyright,
    mutil_tag_date,
    mutil_tag_description,
    mutil_tag_genre,
    mutil_tag_license,
    mutil_tag_location,
    mutil_tag_organization,
    mutil_tag_track_no,
    NULL
};

static gchar const *mutil_tags_required[] = {
    mutil_tag_album,
    mutil_tag_artist,
    mutil_tag_title,
    NULL
};

static gchar const *mutil_tags_single[] = {
    mutil_tag_album,
    mutil_tag_artist,
    mutil_tag_title,
    mutil_tag_track_no,
    NULL
};

struct mutil_album_key;
typedef struct mutil_album_key mutil_album_key_t;

struct mutil_album {
    gchar *name;
    GList *tracks;
};

struct mutil_album_key {
    gchar *artist_text;
    gchar *album_text;
    gchar *performer_text;
};

static mutil_album_t *mutil_album_alloc(
        gboolean opt_flag_simple_album,
        gchar const *artist_text,
        gchar const *album_text,
        gchar const *performer_text);

static void mutil_album_free(
        mutil_album_t *album);

static mutil_album_key_t *mutil_album_key_alloc(
        gchar const *artist_text,
        gchar const *album_text,
        gchar const *performer_text);

static void mutil_album_key_cb_destruct(
        gpointer data);

static void mutil_album_key_free(
        mutil_album_key_t *album_key);

static gint mutil_album_key_cb_compare(
        gconstpointer a,
        gconstpointer b,
        gpointer user_data);

static gint mutil_album_sanity_check(
        mutil_album_t *album,
        gboolean opt_flag_enable_sanity_warnings,
        GError **o_error);

static gint mutil_sanity_check_track(
        mutil_track_t *track,
        gboolean opt_flag_enable_sanity_warnings,
        GError **o_error);

static void mutil_convert_filename(
        gchar **o_filename,
        gboolean opt_flag_strict,
        gboolean opt_flag_no_uppercase,
        gboolean opt_flag_no_space);

mutil_album_t *mutil_album_alloc(
        gboolean opt_flag_simple_album,
        gchar const *artist_text,
        gchar const *album_text,
        gchar const *performer_text)
{
    mutil_album_t *new_album;
    GString *album_name = NULL;

    g_assert(artist_text != NULL);
    g_assert(album_text != NULL);
    /* performer_text may be NULL */

    /* Format the album name, which is:
     * [<artist> - ]<album>[ - <performer>] */

    g_assert(album_name == NULL);
    album_name = g_string_new("");
    if (!opt_flag_simple_album) {
        g_string_append_printf(album_name, "%s - ", artist_text);
    }
    g_string_append_printf(album_name, "%s", album_text);
    if (!opt_flag_simple_album && performer_text != NULL) {
        g_string_append_printf(album_name, " - %s", performer_text);
    }

    new_album = g_malloc0(sizeof(mutil_album_t));
    new_album->name = g_string_free(album_name, FALSE);

    return new_album;
} /* mutil_album_alloc */

GList *mutil_album_create_track_list(
        mutil_album_t *album)
{
    return mutil_track_copy_list_of(album->tracks);
} /* mutil_album_create_track_list */

void mutil_album_free(
        mutil_album_t *album)
{
    if (album != NULL) {
        mutil_track_free_list_of(album->tracks);
        g_free(album->name);
        g_free(album);
    }

    return;
} /* mutil_album_free */

mutil_album_key_t *mutil_album_key_alloc(
        gchar const *artist_text,
        gchar const *album_text,
        gchar const *performer_text)
{
    mutil_album_key_t *new_album_key;

    /* artist_text may be NULL */
    g_assert(album_text != NULL);
    /* performer_text may be NULL */

    new_album_key = g_malloc0(sizeof(mutil_album_key_t));
    new_album_key->artist_text = g_strdup(artist_text);
    new_album_key->album_text = g_strdup(album_text);
    new_album_key->performer_text = g_strdup(performer_text);

    return new_album_key;
} /* mutil_album_key_alloc */

gint mutil_album_key_cb_compare(
        gconstpointer a,
        gconstpointer b,
        gpointer user_data)
{
    mutil_album_key_t const *album_key_a = a;
    mutil_album_key_t const *album_key_b = b;
    gint cmp_result;

    g_assert(album_key_a != NULL);
    g_assert(album_key_b != NULL);

    cmp_result = g_strcmp0(album_key_a->artist_text, album_key_b->artist_text);
    if (cmp_result == 0) {
        cmp_result = g_strcmp0(
                album_key_a->album_text,
                album_key_b->album_text);
    }
    if (cmp_result == 0) {
        cmp_result = g_strcmp0(
                album_key_a->performer_text,
                album_key_b->performer_text);
    }

    return cmp_result;
} /* mutil_album_key_cb_compare */

void mutil_album_key_cb_destruct(
        gpointer data)
{
    mutil_album_key_free(data);

    return;
} /* mutil_album_key_cb_destruct */

void mutil_album_key_free(
        mutil_album_key_t *album_key)
{
    if (album_key != NULL) {
        g_free(album_key->artist_text);
        g_free(album_key->album_text);
        g_free(album_key->performer_text);
        g_free(album_key);
    }

    return;
} /* mutil_album_key_free */

gint mutil_album_list_create_from_track_list(
        GList **o_album_list,
        GList *track_list,
        gboolean opt_flag_simple_album,
        gboolean opt_flag_enable_sanity_warnings,
        gboolean opt_flag_auto_track_no_tags,
        GError **o_error)
{
    gint ret_value;
    GList *new_album_list = NULL;
    GTree *album_tree = NULL;
    GList *node_i;
    mutil_track_t *track_i;
    mutil_album_t *album_i;
    gint status;
    mutil_album_key_t *album_key = NULL;
    mutil_track_t *track_cp = NULL;
    mutil_album_t *album;
    gchar const *artist_text;
    gchar const *album_text;
    gchar const *performer_text;

    g_assert(o_album_list != NULL);
    g_assert(o_error == NULL || *o_error == NULL);

    /* Separate the tracks into albums based on their metadata. */

    album_tree = g_tree_new_full(
            mutil_album_key_cb_compare,
            NULL,
            mutil_album_key_cb_destruct,
            NULL);

    for (node_i = track_list;
         node_i != NULL;
         node_i = node_i->next) {

        track_i = node_i->data;

        status = mutil_sanity_check_track(
                track_i,
                opt_flag_enable_sanity_warnings,
                o_error);
        if (status == -1) {
            goto error_handling;
        }

        artist_text = mutil_track_get_first_tag_value_by_name(
                track_i,
                mutil_tag_artist);
        album_text = mutil_track_get_first_tag_value_by_name(
                track_i,
                mutil_tag_album);
        performer_text = mutil_track_get_first_tag_value_by_name(
                track_i,
                mutil_tag_performer);

        mutil_album_key_free(album_key);
        album_key = mutil_album_key_alloc(
                opt_flag_simple_album ? NULL : artist_text,
                album_text,
                opt_flag_simple_album ? NULL : performer_text);

        album = g_tree_lookup(album_tree, album_key);
        if (album == NULL) {
            album = mutil_album_alloc(
                    opt_flag_simple_album,
                    artist_text,
                    album_text,
                    performer_text);
            g_tree_insert(album_tree, album_key, album);
            new_album_list = g_list_append(new_album_list, album);
            album_key = NULL;
        }

        g_assert(track_cp == NULL);
        track_cp = mutil_track_copy(track_i);
        album->tracks = g_list_append(album->tracks, track_cp);
        track_cp = NULL;
    }

    if (opt_flag_auto_track_no_tags) {
        for (node_i = new_album_list;
             node_i != NULL;
             node_i = node_i->next) {
            album_i = node_i->data;
            mutil_track_list_generate_track_number_tags(album_i->tracks);
        }
    }

    /* Sanity check each album. */
    for (node_i = new_album_list;
         node_i != NULL;
         node_i = node_i->next) {

        album_i = node_i->data;

        status = mutil_album_sanity_check(
                album_i,
                opt_flag_enable_sanity_warnings,
                o_error);
        if (status == -1) {
            goto error_handling;
        }
    }

    g_assert(o_error == NULL || *o_error == NULL);
    *o_album_list = new_album_list;
    new_album_list = NULL;
    ret_value = 0;
    goto cleanup;

error_handling:

    g_assert(o_error == NULL || *o_error != NULL);

    ret_value = -1;

cleanup:

    mutil_album_key_free(album_key);
    mutil_track_free(track_cp);
    g_tree_destroy(album_tree);
    mutil_album_list_free(new_album_list);

    return ret_value;
} /* mutil_album_list_create_from_track_list */

GList *mutil_album_list_create_track_list(
        GList *album_list)
{
    GList *new_track_list = NULL;
    GList *new_sub_list = NULL;
    GList *node_i;
    mutil_album_t *album_i;

    for (node_i = album_list;
         node_i != NULL;
         node_i = node_i->next) {

        album_i = node_i->data;
    
        g_assert(new_sub_list == NULL);
        new_sub_list = mutil_track_copy_list_of(album_i->tracks);
        new_track_list = g_list_concat(new_track_list, new_sub_list);
        new_sub_list = NULL;
    }

    g_assert(new_sub_list == NULL);

    return new_track_list;
} /* mutil_album_list_create_track_list */

void mutil_album_list_free(
        GList *album_list)
{
    while (album_list != NULL) {
        mutil_album_free(album_list->data);
        album_list = g_list_delete_link(album_list, album_list);
    }

    return;
} /* mutil_album_list_free */

mutil_makefile_t *mutil_album_list_generate_archive_makefile(
        GList *album_list,
        gboolean opt_flag_verbose_makefile,
        gboolean opt_flag_use_echo_e)
{
    static gchar const *default_target = "all";

    mutil_makefile_t *new_makefile = NULL;
    GList *node_i;
    mutil_album_t *album_i;
    GList *node_j;
    mutil_track_t *track_j;
    gint j;
    gchar *tmp_str = NULL;
    mutil_make_rule_t *new_rule = NULL;
    mutil_make_rule_t *default_rule = NULL;
    mutil_make_rule_t *replay_gain_rule = NULL;
    gchar *replay_gain_filename = NULL;
    GString *replay_gain_command = NULL;
    gchar *formatter = NULL;
    gint track_cnt;
    gchar const *title_text;
    gchar const *track_filename;
    gchar *target_filename = NULL;
    gchar *encode_command = NULL;
    gchar *decode_command = NULL;

    g_assert(new_makefile == NULL);
    new_makefile = mutil_makefile_alloc();

    /* default target: */

    g_assert(new_rule == NULL);
    new_rule = mutil_make_rule_alloc();
    mutil_make_rule_append_target(new_rule, mutil_makefile_phony);
    mutil_make_rule_append_prereq(new_rule, default_target);
    mutil_makefile_append_rule(new_makefile, new_rule);
    new_rule = NULL;

    g_assert(default_rule == NULL);
    default_rule = mutil_make_rule_alloc();
    mutil_make_rule_append_target(default_rule, default_target);
    
    /* Create rules for each album. */
    for (node_i = album_list;
         node_i != NULL;
         node_i = node_i->next) {

        album_i = node_i->data;
        track_cnt = g_list_length(album_i->tracks);

        /* replay gain (begin): */
        g_assert(replay_gain_rule == NULL);
        replay_gain_rule = mutil_make_rule_alloc();
        g_free(replay_gain_filename);
        replay_gain_filename = g_strdup_printf("%s.replay_gain", album_i->name);
        mutil_convert_filename(&replay_gain_filename, TRUE, TRUE, TRUE);
        mutil_make_rule_append_target(replay_gain_rule, replay_gain_filename);
        mutil_make_rule_append_prereq(default_rule, replay_gain_filename);
        g_assert(replay_gain_command == NULL);
        replay_gain_command = g_string_new("");
        g_string_append_printf(
                replay_gain_command,
                "%smetaflac --add-replay-gain",
                opt_flag_verbose_makefile ? "" : "@");

        /* Create rules for each archive target. */
        for (node_j = album_i->tracks, j = 1;
             node_j != NULL;
             node_j = node_j->next, j++) {

            track_j = node_j->data;
            track_filename = mutil_track_get_filename(track_j);

            g_assert(new_rule == NULL);
            new_rule = mutil_make_rule_alloc();

            /* target: */
            g_free(formatter);
            formatter = g_strdup_printf(
                    "%%s - %%0%dd - %%s.flac",
                    (gint) log10(track_cnt) + 1);

            title_text = mutil_track_get_first_tag_value_by_name(
                        track_j, 
                        mutil_tag_title);
            g_assert(title_text != NULL);

            g_free(target_filename);
            target_filename = g_strdup_printf(
                    formatter,
                    album_i->name,
                    j,
                    title_text);
            mutil_convert_filename(&target_filename, TRUE, TRUE, TRUE);

            mutil_make_rule_append_target(new_rule, target_filename);
            mutil_make_rule_append_prereq(replay_gain_rule, target_filename);
            mutil_make_rule_append_prereq(default_rule, target_filename);
            g_string_append_printf(replay_gain_command, " %s", target_filename);

            /* prereq: */
            g_free(tmp_str);
            tmp_str = g_strdup(track_filename);

            mutil_make_rule_append_prereq(new_rule, tmp_str);

            /* command: */
            if (!opt_flag_verbose_makefile) {

                g_free(tmp_str);
                tmp_str = g_strdup_printf(
                        "@echo %s",
                        target_filename);
                mutil_make_rule_append_command(new_rule, tmp_str);
            }

            /* command: */
            g_free(decode_command);
            decode_command = mutil_track_format_decode_command(track_j);

            g_free(encode_command);
            encode_command = mutil_track_format_archive_encode_command(
                    track_j,
                    target_filename,
                    opt_flag_use_echo_e);

            g_free(tmp_str);
            tmp_str = g_strdup_printf(
                    "%s%s | %s",
                    opt_flag_verbose_makefile ? "" : "@",
                    decode_command,
                    encode_command);

            mutil_make_rule_append_command(new_rule, tmp_str);

            mutil_makefile_append_rule(new_makefile, new_rule);
            new_rule = NULL;
        }

        /* replay gain (end): */
        g_free(tmp_str);
        tmp_str = g_strdup_printf("@echo %s", replay_gain_filename);
        mutil_make_rule_append_command(replay_gain_rule, tmp_str);

        g_free(tmp_str);
        tmp_str = g_string_free(replay_gain_command, FALSE);
        replay_gain_command = NULL;
        mutil_make_rule_append_command(replay_gain_rule, tmp_str);
        mutil_makefile_append_rule(new_makefile, replay_gain_rule);
        replay_gain_rule = NULL;

        g_assert(new_rule == NULL);
        new_rule = mutil_make_rule_alloc();
        mutil_make_rule_append_target(new_rule, mutil_makefile_phony);
        mutil_make_rule_append_prereq(new_rule, replay_gain_filename);
        mutil_makefile_append_rule(new_makefile, new_rule);
        new_rule = NULL;
    }

    mutil_makefile_prepend_rule(new_makefile, default_rule);
    default_rule = NULL;

    /* Clean up. */

    mutil_make_rule_free(new_rule);
    mutil_make_rule_free(default_rule);
    mutil_make_rule_free(replay_gain_rule);
    g_free(tmp_str);
    g_free(replay_gain_filename);
    g_assert(replay_gain_command == NULL);
    g_free(formatter);
    g_free(target_filename);
    g_free(encode_command);
    g_free(decode_command);

    g_assert(new_makefile != NULL);
    return new_makefile;
} /* mutil_album_list_generate_archive_makefile */

mutil_makefile_t *mutil_album_list_generate_oggify_makefile(
        GList *album_list,
        gboolean opt_flag_verbose_makefile,
        gboolean opt_flag_use_echo_e)
{
    static gchar const *default_target = "all";

    mutil_makefile_t *new_makefile = NULL;
    GList *node_i;
    mutil_album_t *album_i;
    GList *node_j;
    mutil_track_t *track_j;
    gint j;
    gint track_cnt;
    mutil_make_rule_t *new_rule = NULL;
    mutil_make_rule_t *default_rule = NULL;
    gchar const *title_text;
    gchar *tmp_str = NULL;
    gchar *dir_name = NULL;
    gchar *formatter = NULL;
    gchar *target_filename = NULL;
    gchar *target_basename = NULL;
    gchar *decode_command = NULL;
    gchar *encode_command = NULL;

    g_assert(new_makefile == NULL);
    new_makefile = mutil_makefile_alloc();

    /* default target: */

    g_assert(new_rule == NULL);
    new_rule = mutil_make_rule_alloc();
    mutil_make_rule_append_target(new_rule, mutil_makefile_phony);
    mutil_make_rule_append_prereq(new_rule, default_target);
    mutil_makefile_append_rule(new_makefile, new_rule);
    new_rule = NULL;

    g_assert(default_rule == NULL);
    default_rule = mutil_make_rule_alloc();
    mutil_make_rule_append_target(default_rule, default_target);
    
    /* Create rules for each album. */
    for (node_i = album_list;
         node_i != NULL;
         node_i = node_i->next) {

        album_i = node_i->data;
        track_cnt = g_list_length(album_i->tracks);

        /* directory creation: */
        g_assert(new_rule == NULL);
        new_rule = mutil_make_rule_alloc();
        g_free(dir_name);
        dir_name = g_strdup(album_i->name);
        mutil_convert_filename(&dir_name, TRUE, FALSE, FALSE);
        mutil_make_rule_append_target(new_rule, dir_name);
        g_free(tmp_str);
        tmp_str = g_strdup_printf(
                "%smkdir -p \"$@\"",
                opt_flag_verbose_makefile ? "" : "@");
        mutil_make_rule_append_command(new_rule, tmp_str);
        mutil_makefile_append_rule(new_makefile, new_rule);
        new_rule = NULL;

        mutil_make_rule_append_prereq(default_rule, dir_name);

        /* Create rules for each ogg target. */
        for (node_j = album_i->tracks, j = 1;
             node_j != NULL;
             node_j = node_j->next, j++) {

            track_j = node_j->data;

            g_assert(new_rule == NULL);
            new_rule = mutil_make_rule_alloc();

            /* target: */

            title_text = mutil_track_get_first_tag_value_by_name(
                    track_j,
                    mutil_tag_title);

            g_free(formatter);
            formatter = g_strdup_printf(
                    "%%0%dd - %%s.ogg",
                    (gint) log10(track_cnt) + 1);

            g_free(target_basename);
            target_basename = g_strdup_printf(
                    formatter,
                    j,
                    title_text);
            mutil_convert_filename(&target_basename, TRUE, FALSE, FALSE);

            g_free(target_filename);
            target_filename = g_strdup_printf(
                    "%s/%s",
                    dir_name,
                    target_basename);

            mutil_make_rule_append_target(new_rule, target_filename);
            mutil_make_rule_append_prereq(default_rule, target_filename);

            /* prereq: */

            mutil_make_rule_append_prereq(
                    new_rule,
                    mutil_track_get_filename(track_j));

            /* command: */

            if (!opt_flag_verbose_makefile) {
                g_free(tmp_str);
                tmp_str = g_strdup("@echo \"$@\"");
                mutil_make_rule_append_command(new_rule, tmp_str);
            }

            g_free(decode_command);
            decode_command = mutil_track_format_decode_command(track_j);

            g_free(encode_command);
            encode_command = mutil_track_format_ogg_encode_command(
                    track_j,
                    target_filename,
                    opt_flag_use_echo_e);

            g_free(tmp_str);
            tmp_str = g_strdup_printf(
                    "%s%s | %s",
                    opt_flag_verbose_makefile ? "" : "@",
                    decode_command,
                    encode_command);

            mutil_make_rule_append_command(new_rule, tmp_str);

            mutil_makefile_append_rule(new_makefile, new_rule);
            new_rule = NULL;
        }
    }

    /* TODO */

    mutil_makefile_prepend_rule(new_makefile, default_rule);
    default_rule = NULL;

    /* Clean up. */

    mutil_make_rule_free(new_rule);
    mutil_make_rule_free(default_rule);
    g_free(tmp_str);
    g_free(dir_name);
    g_free(formatter);
    g_free(target_filename);
    g_free(target_basename);
    g_free(decode_command);
    g_free(encode_command);

    g_assert(new_makefile != NULL);
    return new_makefile;
} /* mutil_album_list_generate_oggify_makefile */

gint mutil_album_sanity_check(
        mutil_album_t *album,
        gboolean opt_flag_enable_sanity_warnings,
        GError **o_error)
{
    gint ret_value;
    GList const *node_i;
    mutil_track_t *track_i;
    gchar const *track_no_text;
    gint i;
    gint64 track_no_from_text;
    gchar *conv_ptr;
    gboolean validity_flag = FALSE;

    g_assert(album != NULL);
    g_assert(o_error == NULL || *o_error == NULL);

    for (node_i = album->tracks, i = 1;
         node_i != NULL;
         node_i = node_i->next, i++) {

        track_i = node_i->data;

        track_no_text = mutil_track_get_first_tag_value_by_name(
                track_i,
                mutil_tag_track_no);
        if (track_no_text != NULL) {

            track_no_from_text = g_ascii_strtoll(
                    track_no_text,
                    &conv_ptr,
                    10);
            if (conv_ptr == track_no_text) {
                mutil_print_warning(
                        opt_flag_enable_sanity_warnings,
                        "track '%s' contains invalid track number",
                        mutil_track_get_filename(track_i));
            } else if (track_no_from_text != i) {
                mutil_print_warning(
                        opt_flag_enable_sanity_warnings,
                        "track '%s' contains out-of-order track number ("
                        "got %d, expected %d)",
                        mutil_track_get_filename(track_i),
                        (gint) track_no_from_text,
                        i);
            } else {
                validity_flag = TRUE;
            }
        }
    }

    g_assert(o_error == NULL || *o_error == NULL);
    ret_value = validity_flag ? 0 : -1;
    goto cleanup;

cleanup:

    return ret_value;
} /* mutil_album_sanity_check_list_of */

void mutil_convert_filename(
        gchar **o_filename,
        gboolean opt_flag_strict,
        gboolean opt_flag_no_uppercase,
        gboolean opt_flag_no_space)
{
    GString *new_filename = NULL;
    gchar *src_pos;
    gunichar src_ch;
    gchar *dot_pos;
    gint dot_cnt;
    gint dot_sum;

    g_assert(o_filename != NULL);
    g_assert(*o_filename != NULL);

    new_filename = g_string_new("");

    /* Count '.' characters so that the final '.' character won't be replaced in
     * the conversion. This protects file extensions. */
    if (opt_flag_strict) {
        dot_cnt = 0;
        for (dot_pos = g_utf8_strchr(*o_filename, -1, '.');
             dot_pos != NULL;
             dot_pos = g_utf8_strchr(dot_pos + 1, -1, '.'), dot_cnt++) {
        }
    }

    dot_sum = dot_cnt;
    dot_cnt = 0;
    for (src_pos = *o_filename;
         *src_pos != '\0';
         src_pos = g_utf8_next_char(src_pos)) {

        src_ch = g_utf8_get_char(src_pos);

        dot_cnt += src_ch == '.' ? 1 : 0;
        g_assert(dot_cnt <= dot_sum);

        src_ch = opt_flag_no_uppercase ? g_unichar_tolower(src_ch) : src_ch;
        src_ch = opt_flag_no_space && g_unichar_isspace(src_ch) ?  '_' : src_ch;
        src_ch = opt_flag_strict && g_unichar_ispunct(src_ch) &&
            src_ch != '-' && (src_ch != '.' || dot_cnt < dot_sum) ?
            '_' : src_ch;
        src_ch = opt_flag_strict && g_unichar_isalpha(src_ch) &&
            (g_unichar_tolower(src_ch) < 'a' ||
             g_unichar_tolower(src_ch) > 'z') ? '_' : src_ch;

        g_string_append_unichar(new_filename, src_ch);
    }

    g_free(*o_filename);
    *o_filename = g_string_free(new_filename, FALSE);

    return;
} /* mutil_convert_filename */

gint mutil_sanity_check_track(
        mutil_track_t *track,
        gboolean opt_flag_enable_sanity_warnings,
        GError **o_error)
{
    gint ret_value;
    gint i;
    gint j;
    GList *tag_list = NULL;
    GList *node_i;
    mutil_tag_t *tag_i;
    gchar const *track_filename;
    gchar const *tag_name_text;
    gchar const *tag_value_text;
    gunichar value_ch;
    gchar *tmp_str = NULL;
    gchar const *str_iter;

    g_assert(track != NULL);
    g_assert(o_error == NULL || *o_error == NULL);

    track_filename = mutil_track_get_filename(track);

    g_assert(tag_list == NULL);
    tag_list = mutil_track_create_tag_list(track);

    /* Check for tags which are required. */
    for (i = 0; mutil_tags_required[i] != NULL; i++) {
        if (!mutil_track_has_tag(track, mutil_tags_required[i])) {
            g_set_error(
                    o_error,
                    mutil_error_domain,
                    mutil_error_code_undefined,
                    "track '%s' is missing tag '%s'",
                    track_filename,
                    mutil_tags_required[i]);
            goto error_handling;
        }
    }
    
    /* Check for tags which are expected to be singletons. */
    for (i = 0; mutil_tags_single[i] != NULL; i++) {
        if (mutil_track_has_duplicate_tags(track, mutil_tags_single[i])) {
            mutil_print_warning(
                    opt_flag_enable_sanity_warnings,
                    "track '%s' contains muliple tags '%s'",
                    track_filename,
                    mutil_tags_single[i]);
        }
    }
    
    /* Check for tags which are expected. */
    for (i = 0; mutil_tags_expected[i] != NULL; i++) { 
        if (!mutil_track_has_tag(track, mutil_tags_expected[i])) {
            mutil_print_warning(
                    opt_flag_enable_sanity_warnings,
                    "track '%s' is missing tag '%s'",
                    track_filename,
                    mutil_tags_expected[i]);
        }
    }

    /* Check for tags which are not expected. */
    for (node_i = tag_list;
         node_i != NULL;
         node_i = node_i->next) {

        tag_i = node_i->data;
        tag_name_text = mutil_tag_get_name(tag_i);

        j = 0;
        while (mutil_tags_all[j] != NULL &&
               !mutil_tag_is_name_equal_to(tag_i, mutil_tags_all[j])) {
            j++;
        }
        if (mutil_tags_all[j] == NULL) {
            mutil_print_warning(
                    opt_flag_enable_sanity_warnings,
                    "track '%s' contains unexpected tag '%s'",
                    track_filename,
                    tag_name_text);
        }
    }

    /* Check leading or trailing whitespace in the tag value text. */
    for (node_i = tag_list;
         node_i != NULL;
         node_i = node_i->next) {

        tag_i = node_i->data;
        tag_name_text = mutil_tag_get_name(tag_i);
        tag_value_text = mutil_tag_get_value(tag_i);

        value_ch = g_utf8_get_char(tag_value_text);
        if (g_unichar_isspace(value_ch)) {
            mutil_print_warning(
                    opt_flag_enable_sanity_warnings,
                    "track '%s' has tag '%s' with leading whitespace",
                    track_filename,
                    tag_name_text);
        }

        g_free(tmp_str);
        tmp_str = g_utf8_strreverse(tag_value_text, -1);
        value_ch = g_utf8_get_char(tmp_str);
        if (g_unichar_isspace(value_ch)) {
            mutil_print_warning(
                    opt_flag_enable_sanity_warnings,
                    "track '%s' has tag '%s' with trailing whitespace",
                    track_filename,
                    tag_name_text);
        }
    }

    /* Check for newlines in the tag value text. */
    for (node_i = tag_list;
         node_i != NULL;
         node_i = node_i->next) {

        tag_i = node_i->data;
        tag_name_text = mutil_tag_get_name(tag_i);
        tag_value_text = mutil_tag_get_value(tag_i);

        for (str_iter = tag_value_text;
             *str_iter != '\0';
             str_iter = g_utf8_next_char(str_iter)) {

            value_ch = g_utf8_get_char(str_iter);
            if (value_ch == '\n') {
                mutil_print_warning(
                        opt_flag_enable_sanity_warnings,
                        "track '%s' has tag '%s' with line breaks",
                        track_filename,
                        tag_name_text);
            }
        }
    }

    g_assert(o_error == NULL || *o_error == NULL);
    ret_value = 0;
    goto cleanup;

error_handling:

    g_assert(o_error == NULL || *o_error != NULL);

    ret_value = -1;

cleanup:

    mutil_tag_free_list_of(tag_list);
    g_free(tmp_str);

    return ret_value;
} /* mutil_sanity_check_track */

