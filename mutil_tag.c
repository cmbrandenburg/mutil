/* vim: tabstop=4:shiftwidth=4:expandtab:tw=80
 */

#include "mutil_tag.h"

struct mutil_tag {
    gint ref_cnt;
    gchar *name;
    gchar *value;
};

static GList *mutil_tag_copy_list_of(
        GList *tag_list);

static gint mutil_tag_map_cb_compare_names(
        gconstpointer a,
        gconstpointer b,
        gpointer user_data);

static gboolean mutil_tag_map_cb_copy_bucket_into_tree(
        gpointer key,
        gpointer value,
        gpointer data);

static gboolean mutil_tag_map_cb_copy_tag_list_into_list(
        gpointer key,
        gpointer value,
        gpointer data);

static gboolean mutil_tag_map_destruct_value(
        gpointer key,
        gpointer value,
        gpointer data);

mutil_tag_t *mutil_tag_alloc(
        gchar const *name,
        gchar const *value)
{
    mutil_tag_t *new_tag;

    g_assert(name != NULL);
    g_assert(value != NULL);

    new_tag = g_malloc0(sizeof(mutil_tag_t));
    new_tag->ref_cnt = 1;
    new_tag->name = g_strdup(name);
    new_tag->value = g_strdup(value);

    return new_tag;
} /* mutil_tag_alloc */

mutil_tag_t *mutil_tag_copy(
        mutil_tag_t *tag)
{
    g_assert(tag != NULL);

    tag->ref_cnt++;

    return tag;
} /* mutil_tag_copy */

GList *mutil_tag_copy_list_of(
        GList *tag_list)
{
    GList *new_tag_list = NULL;
    GList *node_i;
    mutil_tag_t *new_tag = NULL;

    for (node_i = tag_list;
         node_i != NULL;
         node_i = node_i->next) {

        g_assert(new_tag == NULL);
        new_tag = mutil_tag_copy(node_i->data);
        new_tag_list = g_list_append(new_tag_list, new_tag);
        new_tag = NULL;
    }

    return new_tag_list;
} /* mutil_tag_copy_list_of */

mutil_tag_t *mutil_tag_create_from_simple_assignment(
        gchar const *tag_text,
        gunichar separator,
        GError **o_error)
{
    mutil_tag_t *new_tag = NULL;
    gchar const *sep_pos;
    gchar *name = NULL;

    g_assert(tag_text != NULL);
    g_assert(o_error == NULL || *o_error == NULL);

    sep_pos = g_utf8_strchr(tag_text, -1, separator);
    if (sep_pos == NULL) {
        g_set_error(
                o_error,
                mutil_error_domain,
                mutil_error_code_undefined,
                "no tag separator found in text '%s'", tag_text);
        goto error_handling;
    }

    if (sep_pos[1] == '\0') {
        g_set_error(
                o_error,
                mutil_error_domain,
                mutil_error_code_undefined,
                "no value found in text '%s'", tag_text);
        goto error_handling;
    }

    g_assert(name == NULL);
    name = g_malloc((sep_pos - tag_text + 1) * sizeof(gchar));
    memcpy(name, tag_text, (sep_pos - tag_text) * sizeof(gchar));
    name[sep_pos - tag_text] = '\0';

    g_assert(new_tag == NULL);
    new_tag = mutil_tag_alloc(name, &sep_pos[1]);

    g_assert(o_error == NULL || *o_error == NULL);
    g_assert(new_tag != NULL);
    goto cleanup;

error_handling:

    g_assert(o_error == NULL || *o_error != NULL);

    mutil_tag_free(new_tag);
    new_tag = NULL;

cleanup:

    g_free(name);

    return new_tag;
} /* mutil_tag_create_from_simple_assignment */

void mutil_tag_free(
        mutil_tag_t *tag)
{
    if (tag != NULL && --tag->ref_cnt == 0) {
        g_free(tag->name);
        g_free(tag->value);
        g_free(tag);
    }

    return;
} /* mutil_tag_free */

void mutil_tag_free_list_of(
        GList *tag_list)
{
    while (tag_list != NULL) {
        mutil_tag_free(tag_list->data);
        tag_list = g_list_delete_link(tag_list, tag_list);
    }

    return;
} /* mutil_tag_free_list_of */

gchar const *mutil_tag_get_name(
        mutil_tag_t *tag)
{
    g_assert(tag != NULL);

    return tag->name;
} /* mutil_tag_get_name */

gchar const *mutil_tag_get_value(
        mutil_tag_t *tag)
{
    g_assert(tag != NULL);

    return tag->value;
} /* mutil_tag_get_value */

gboolean mutil_tag_is_name_equal_to(
        mutil_tag_t *tag,
        gchar const *tag_name)
{
    g_assert(tag != NULL);
    g_assert(tag_name != NULL);

    return mutil_tag_map_cb_compare_names(tag->name, tag_name, NULL) == 0 ?
        TRUE :
        FALSE;
} /* mutil_tag_is_name_equal_to */

void mutil_tag_map_add_tag(
        mutil_tag_map_t *tag_map,
        mutil_tag_t *tag)
{
    GList *old_bucket;
    GList *new_bucket;
    gchar *new_key;
    mutil_tag_t *new_tag;

    g_assert(tag_map != NULL);
    g_assert(tag != NULL);

    old_bucket = g_tree_lookup(tag_map, tag->name);
    new_key = g_strdup(tag->name);
    new_tag = mutil_tag_copy(tag);
    new_bucket = g_list_append(old_bucket, new_tag);
    g_tree_replace(tag_map, new_key, new_bucket);
    new_key = NULL;
    new_tag = NULL;

    return;
} /* mutil_tag_map_add_tag */

mutil_tag_map_t *mutil_tag_map_alloc(void)
{
    mutil_tag_map_t *new_map;

    new_map = g_tree_new_full(
            mutil_tag_map_cb_compare_names,
            NULL,
            g_free,
            NULL);

    return new_map;
} /* mutil_tag_map_alloc */

gint mutil_tag_map_cb_compare_names(
        gconstpointer a,
        gconstpointer b,
        gpointer user_data)
{
    gchar *safe_a;
    gchar *safe_b;
    gint cmp_result;

    safe_a = g_utf8_casefold(a, -1);
    safe_b = g_utf8_casefold(b, -1);

    cmp_result = g_utf8_collate(safe_a, safe_b);

    g_free(safe_a);
    g_free(safe_b);

    return cmp_result;
} /* mutil_tag_map_cb_compare_names*/

gboolean mutil_tag_map_cb_copy_bucket_into_tree(
        gpointer key,
        gpointer value,
        gpointer data)
{
    gchar *new_key;
    GList *new_tag_list;

    g_assert(key != NULL);
    g_assert(value != NULL);
    g_assert(data != NULL);

    new_key = g_strdup(key);
    new_tag_list = mutil_tag_copy_list_of(value);

    g_assert(g_tree_lookup(data, key) == NULL);
    g_tree_insert(data, new_key, new_tag_list);

    return FALSE;
} /* mutil_tag_map_cb_copy_bucket_into_tree */

gboolean mutil_tag_map_cb_copy_tag_list_into_list(
        gpointer key,
        gpointer value,
        gpointer data)
{
    mutil_tag_t *new_tag = NULL;
    GList *node_i;
    GList **o_list = data;

    g_assert(key != NULL);
    g_assert(value != NULL);
    g_assert(o_list != NULL);

    for (node_i = value;
         node_i != NULL;
         node_i = node_i->next) {

        g_assert(new_tag == NULL);
        new_tag = mutil_tag_copy(node_i->data);
        *o_list = g_list_append(*o_list, new_tag);
        new_tag = NULL;
    }

    return FALSE;
} /* mutil_tag_map_cb_copy_tag_list_into_list */

gboolean mutil_tag_map_destruct_value(
        gpointer key,
        gpointer value,
        gpointer data)
{
    GList *bucket;

    g_assert(key != NULL);
    g_assert(value != NULL);

    bucket = value;
    while (bucket != NULL) {
        mutil_tag_free(bucket->data);
        bucket = g_list_remove_link(bucket, bucket);
    }

    return FALSE;
} /* mutil_tag_map_destruct_value */

mutil_tag_map_t *mutil_tag_map_copy(
        mutil_tag_map_t *tag_map)
{
    mutil_tag_map_t *new_tag_map;

    g_assert(tag_map != NULL);

    new_tag_map = mutil_tag_map_alloc();
    g_tree_foreach(
            tag_map,
            mutil_tag_map_cb_copy_bucket_into_tree,
            new_tag_map);

    return new_tag_map;
} /* mutil_tag_map_copy */

GList *mutil_tag_map_create_list(
        mutil_tag_map_t *tag_map)
{
    GList *new_tag_list = NULL;

    g_assert(tag_map != NULL);

    g_tree_foreach(
            tag_map,
            mutil_tag_map_cb_copy_tag_list_into_list,
            &new_tag_list);

    return new_tag_list;
} /* mutil_tag_map_create_list */

void mutil_tag_map_free(
        mutil_tag_map_t *tag_map)
{
    if (tag_map != NULL) {
        g_tree_foreach(tag_map, mutil_tag_map_destruct_value, NULL);
        g_tree_destroy(tag_map);
    }

    return;
} /* mutil_tag_map_free */

GList *mutil_tag_map_look_up(
        mutil_tag_map_t *tag_map,
        gchar const *tag_name)
{
    GList *tag_bucket;

    g_assert(tag_map != NULL);
    g_assert(tag_name != NULL);

    tag_bucket = g_tree_lookup(tag_map, tag_name);

    return tag_bucket;
} /* mutil_tag_map_look_up */

