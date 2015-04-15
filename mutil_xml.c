/* vim: tabstop=4:shiftwidth=4:expandtab:tw=80
 */

#include "mutil_tag.h"
#include "mutil_track.h"
#include "mutil_xml.h"
#include <libxml/xmlsave.h>

#define mutil_xml_attr_name_filename "filename"

#define mutil_xml_tag_global "global"
#define mutil_xml_tag_tag_list "tag_list"
#define mutil_xml_tag_track "track"
#define mutil_xml_tag_track_list "track_list"

static gboolean mutil_is_xml_string_all_whitespace(
        xmlChar const *str);

static gint mutil_parse_xml_element_as_tag(
        mutil_tag_t **o_tag,
        xmlNode *root_node,
        GError **o_error);

static gint mutil_parse_xml_element_global(
        GList **o_tag_list,
        xmlNode *root_node,
        GError **o_error);

static gint mutil_parse_xml_element_tag_list(
        GList **o_tag_list,
        xmlNode *root_node,
        GError **o_error);

static gint mutil_parse_xml_element_track(
        mutil_track_t **o_track,
        xmlNode *root_node,
        GError **o_error);

static gint mutil_parse_xml_element_track_list(
        GList **o_track_list,
        xmlNode *root_node,
        GError **o_error);

static xmlChar *mutil_xmlstrdup(
        xmlChar **o_str,
        gchar const *i_str);

gint mutil_create_track_list_from_xml_doc(
        xmlDoc *xml_doc,
        GList **o_track_list,
        GError **o_error)
{
    gint ret_value;
    GList *new_track_list = NULL;
    xmlNode *root_node;
    xmlChar *tmp_xml_str_1 = NULL;
    gint status;

    g_assert(xml_doc != NULL);
    g_assert(o_track_list != NULL);
    g_assert(o_error == NULL || *o_error == NULL);

    root_node = xmlDocGetRootElement(xml_doc);
    if (root_node == NULL ||
        root_node->type != XML_ELEMENT_NODE) {
        g_set_error(
                o_error,
                mutil_error_domain,
                mutil_error_code_undefined,
                "missing root element '%s'",
                mutil_xml_tag_track_list);
        goto error_handling;
    }

    mutil_xmlstrdup(&tmp_xml_str_1, mutil_xml_tag_track_list);
    if (!xmlStrEqual(root_node->name, tmp_xml_str_1)) {
        g_set_error(
                o_error,
                mutil_error_domain,
                mutil_error_code_undefined,
                "root element is '%s', expected '%s'",
                (gchar const *) root_node->name,
                mutil_xml_tag_track_list);
        goto error_handling;
    }

    status = mutil_parse_xml_element_track_list(
            &new_track_list,
            root_node,
            o_error);
    if (status == -1) {
        goto error_handling;
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

    mutil_xmlstrdup(&tmp_xml_str_1, NULL);
    mutil_track_free_list_of(new_track_list);

    return ret_value;
} /* mutil_create_track_list_from_xml_doc */

xmlDoc *mutil_create_xml_doc_from_track_list(
        GList *track_list,
        gboolean opt_flag_create_global_section)
{
    xmlDoc *new_xml_doc = NULL;
    xmlNode *new_node = NULL;
    xmlNode *track_list_node;
    xmlNode *track_node;
    xmlNode *tag_list_node;
    xmlNode *global_node;
    xmlChar *tmp_xml_str_1 = NULL;
    xmlChar *tmp_xml_str_2 = NULL;
    xmlChar *tmp_xml_str_3 = NULL;
    GList *node_i;
    mutil_track_t *track_i;
    GList *tag_list = NULL;
    GList *node_j;
    mutil_tag_t *tag_j;

    g_assert(new_xml_doc == NULL);
    mutil_xmlstrdup(&tmp_xml_str_1, "1.0");
    new_xml_doc = xmlNewDoc(tmp_xml_str_1);

    mutil_xmlstrdup(&tmp_xml_str_1, mutil_xml_tag_track_list);

    g_assert(new_node == NULL);
    new_node = xmlNewDocNode(
            new_xml_doc,
            NULL,
            tmp_xml_str_1,
            NULL);
    xmlDocSetRootElement(new_xml_doc, new_node);
    track_list_node = new_node;
    new_node = NULL;

    /* Create global section if specified to do so. */
    if (opt_flag_create_global_section) {

        mutil_xmlstrdup(&tmp_xml_str_1, mutil_xml_tag_global);
        g_assert(new_node == NULL);
        new_node = xmlNewChild(track_list_node, NULL, tmp_xml_str_1, NULL);
        global_node = new_node;
        new_node = NULL;

        mutil_xmlstrdup(&tmp_xml_str_1, mutil_xml_tag_tag_list);
        g_assert(new_node == NULL);
        new_node = xmlNewChild(global_node, NULL, tmp_xml_str_1, NULL);
        new_node = NULL;
    }

    for (node_i = track_list;
         node_i != NULL;
         node_i = node_i->next) {

        track_i = node_i->data;

        mutil_xmlstrdup(&tmp_xml_str_1, mutil_xml_tag_track);
        mutil_xmlstrdup(&tmp_xml_str_2, mutil_xml_attr_name_filename);
        mutil_xmlstrdup(&tmp_xml_str_3, mutil_track_get_filename(track_i));

        track_node = xmlNewChild(track_list_node, NULL, tmp_xml_str_1, NULL);
        xmlNewProp(track_node, tmp_xml_str_2, tmp_xml_str_3);

        mutil_xmlstrdup(&tmp_xml_str_1, mutil_xml_tag_tag_list);
        tag_list_node = xmlNewChild(track_node, NULL, tmp_xml_str_1, NULL);

        mutil_tag_free_list_of(tag_list);
        tag_list = mutil_track_create_tag_list(track_i);
        for (node_j = tag_list;
             node_j != NULL;
             node_j = node_j->next) {

            tag_j = node_j->data;

            mutil_xmlstrdup(&tmp_xml_str_1, mutil_tag_get_name(tag_j));
            mutil_xmlstrdup(&tmp_xml_str_2, mutil_tag_get_value(tag_j));
            g_assert(new_node == NULL);
            xmlNewChild(tag_list_node, NULL, tmp_xml_str_1, tmp_xml_str_2);
        }
    }

    /* Clean up. */

    mutil_xmlstrdup(&tmp_xml_str_1, NULL);
    mutil_xmlstrdup(&tmp_xml_str_2, NULL);
    mutil_xmlstrdup(&tmp_xml_str_3, NULL);
    mutil_tag_free_list_of(tag_list);

    g_assert(new_node == NULL);
    g_assert(new_xml_doc != NULL);
    return new_xml_doc;
} /* mutil_create_xml_doc_from_track_list */

gboolean mutil_is_xml_string_all_whitespace(
        xmlChar const *str)
{
    gboolean found_non_whitespace;
    gchar const *text_pos_i;
    gunichar text_char_i;

    found_non_whitespace = FALSE;
    text_pos_i = (gchar const *) str;
    text_char_i = g_utf8_get_char(text_pos_i);
    while (!found_non_whitespace && 
           text_char_i != '\0') {

        if (!g_unichar_isspace(text_char_i)) {
            found_non_whitespace = TRUE;
        } else {
            text_pos_i = g_utf8_next_char(text_pos_i);
            text_char_i = g_utf8_get_char(text_pos_i);
        }
    }

    return found_non_whitespace ? FALSE : TRUE;
} /* mutil_is_xml_node_all_whitespace */

gint mutil_parse_xml_element_as_tag(
        mutil_tag_t **o_tag,
        xmlNode *root_node,
        GError **o_error)
{
    gint ret_value;
    mutil_tag_t *new_tag = NULL;
    xmlNode *node_i;
    GString *value_text = NULL;

    g_assert(o_tag != NULL);
    g_assert(*o_tag == NULL);
    g_assert(root_node != NULL);
    g_assert(root_node->type == XML_ELEMENT_NODE);

    if (root_node->properties != NULL) {
        g_set_error(
                o_error,
                mutil_error_domain,
                mutil_error_code_undefined,
                "XML element '%s' contains invalid property '%s'",
                (gchar const *) root_node->name,
                (gchar const *) root_node->properties->name);
        goto error_handling;
    }

    value_text = g_string_new("");

    for (node_i = root_node->children;
         node_i != NULL;
         node_i = node_i->next) {

        if (node_i->type == XML_TEXT_NODE) {
            g_string_append(value_text, (gchar const *) node_i->content);
        } else if (node_i->type == XML_ELEMENT_NODE) {
            g_set_error(
                    o_error,
                    mutil_error_domain,
                    mutil_error_code_undefined,
                    "XML element '%s' contains invalid element '%s'",
                    (gchar const *) root_node->name,
                    (gchar const *) node_i->name);
            goto error_handling;
        } else {
            /* Ignore all other node types. */
        }
    }

    g_assert(new_tag == NULL);
    new_tag = mutil_tag_alloc(
            (gchar const *) root_node->name,
            value_text->str);

    *o_tag = new_tag;
    new_tag = NULL;
    g_assert(*o_tag != NULL);
    ret_value = 0;
    goto cleanup;

error_handling:

    ret_value = -1;
    g_assert(*o_tag == NULL);

cleanup:

    mutil_tag_free(new_tag);
    g_string_free(value_text, TRUE);

    return ret_value;
} /* mutil_parse_xml_element_as_tag */

gint mutil_parse_xml_element_global(
        GList **o_tag_list,
        xmlNode *root_node,
        GError **o_error)
{
    gint ret_value;
    xmlChar *tmp_xml_str_1 = NULL;
    xmlNode *node_i;
    GList *new_tag_list = NULL;
    gint status;

    g_assert(o_tag_list != NULL);
    g_assert(*o_tag_list == NULL);
    g_assert(root_node != NULL);
    g_assert(root_node->type == XML_ELEMENT_NODE);
    g_assert(xmlStrEqual(
                mutil_xmlstrdup(&tmp_xml_str_1, mutil_xml_tag_global),
                root_node->name));

    if (root_node->properties != NULL) {
        g_set_error(
                o_error,
                mutil_error_domain,
                mutil_error_code_undefined,
                "XML element '%s' contains invalid property '%s'",
                (gchar const *) root_node->name,
                (gchar const *) root_node->properties->name);
        goto error_handling;
    }

    for (node_i = root_node->children;
         node_i != NULL;
         node_i = node_i->next) {

        if (node_i->type == XML_TEXT_NODE &&
            !mutil_is_xml_string_all_whitespace(node_i->content)) {
            g_set_error(
                    o_error,
                    mutil_error_domain,
                    mutil_error_code_undefined,
                    "XML element '%s' contains non-whitespace text",
                    (gchar const *) root_node->name);
            goto error_handling;
        } else if (node_i->type == XML_TEXT_NODE) {
            /* Ignore all whitespace. */
        } else if (node_i->type == XML_ELEMENT_NODE &&
                   xmlStrEqual(
                       mutil_xmlstrdup(&tmp_xml_str_1, mutil_xml_tag_tag_list),
                       node_i->name)) {
            if (new_tag_list != NULL) {
                g_set_error(
                        o_error,
                        mutil_error_domain,
                        mutil_error_code_undefined,
                        "XML element '%s' contains multiple elements '%s'",
                        (gchar const *) root_node->name,
                        mutil_xml_tag_global);
                goto error_handling;
            }
            status = mutil_parse_xml_element_tag_list(
                    &new_tag_list,
                    node_i,
                    o_error);
            if (status == -1) {
                goto error_handling;
            }
        } else if (node_i->type == XML_ELEMENT_NODE) {
            g_set_error(
                    o_error,
                    mutil_error_domain,
                    mutil_error_code_undefined,
                    "XML element '%s' contains invalid element '%s'",
                    (gchar const *) root_node->name,
                    (gchar const *) node_i->name);
            goto error_handling;
        } else {
            /* Ignore all other node types. */
        }
    }

    *o_tag_list = new_tag_list;
    new_tag_list = NULL;
    ret_value = 0;
    goto cleanup;

error_handling:

    ret_value = -1;
    g_assert(*o_tag_list == NULL);

cleanup:

    mutil_xmlstrdup(&tmp_xml_str_1, NULL);
    mutil_tag_free_list_of(new_tag_list);

    return ret_value;
} /* mutil_parse_xml_element_global */

gint mutil_parse_xml_element_tag_list(
        GList **o_tag_list,
        xmlNode *root_node,
        GError **o_error)
{
    gint ret_value;
    xmlChar *tmp_xml_str_1 = NULL;
    xmlNode *node_i;
    gint status;
    GList *new_tag_list = NULL;
    mutil_tag_t *new_tag = NULL;

    g_assert(o_tag_list != NULL);
    g_assert(*o_tag_list == NULL);
    g_assert(root_node != NULL);
    g_assert(root_node->type == XML_ELEMENT_NODE);
    g_assert(xmlStrEqual(
                mutil_xmlstrdup(&tmp_xml_str_1, mutil_xml_tag_tag_list),
                root_node->name));

    if (root_node->properties != NULL) {
        g_set_error(
                o_error,
                mutil_error_domain,
                mutil_error_code_undefined,
                "XML element '%s' contains invalid property '%s'",
                (gchar const *) root_node->name,
                (gchar const *) root_node->properties->name);
        goto error_handling;
    }

    for (node_i = root_node->children;
         node_i != NULL;
         node_i = node_i->next) {

        if (node_i->type == XML_TEXT_NODE &&
            !mutil_is_xml_string_all_whitespace(node_i->content)) {
            g_set_error(
                    o_error,
                    mutil_error_domain,
                    mutil_error_code_undefined,
                    "XML element '%s' contains non-whitespace text",
                    (gchar const *) root_node->name);
            goto error_handling;
        } else if (node_i->type == XML_TEXT_NODE) {
            /* Ignore all whitespace. */
        } else if (node_i->type == XML_ELEMENT_NODE) {

            status = mutil_parse_xml_element_as_tag(&new_tag, node_i, o_error);
            if (status == -1) {
                goto error_handling;
            }

            new_tag_list = g_list_append(new_tag_list, new_tag);
            new_tag = NULL;

        } else {
            /* Ignore all other node types. */
        }
    }

    *o_tag_list = new_tag_list;
    new_tag_list = NULL;
    ret_value = 0;
    goto cleanup;

error_handling:

    ret_value = -1;
    g_assert(*o_tag_list == NULL);

cleanup:

    mutil_xmlstrdup(&tmp_xml_str_1, NULL);
    mutil_tag_free_list_of(new_tag_list);
    mutil_tag_free(new_tag);

    return ret_value;
} /* mutil_parse_xml_element_tag_list */

gint mutil_parse_xml_element_track(
        mutil_track_t **o_track,
        xmlNode *root_node,
        GError **o_error)
{
    gint ret_value;
    xmlChar *tmp_xml_str_1 = NULL;
    xmlAttr *attr_i;
    xmlNode *node_i;
    mutil_track_t *new_track = NULL;
    GList *new_tag_list = NULL;
    gint status;
    gchar const *track_filename = NULL;
    mutil_audio_type_t track_audio_type;

    g_assert(o_track != NULL);
    g_assert(*o_track == NULL);
    g_assert(root_node != NULL);
    g_assert(root_node->type == XML_ELEMENT_NODE);
    g_assert(xmlStrEqual(
                mutil_xmlstrdup(&tmp_xml_str_1, mutil_xml_tag_track),
                root_node->name));

    for (attr_i = root_node->properties;
         attr_i != NULL;
         attr_i = attr_i->next) {

        mutil_xmlstrdup(&tmp_xml_str_1, mutil_xml_attr_name_filename);
        if (!xmlStrEqual(tmp_xml_str_1, attr_i->name)) {
            g_set_error(
                    o_error,
                    mutil_error_domain,
                    mutil_error_code_undefined,
                    "XML element '%s' contains invalid attributes '%s'",
                    (gchar const *) root_node->name,
                    (gchar const *) attr_i->name);
            goto error_handling;
        }

        if (track_filename != NULL) {
            g_set_error(
                    o_error,
                    mutil_error_domain,
                    mutil_error_code_undefined,
                    "XML element '%s' contains multiple attributes '%s'",
                    (gchar const *) root_node->name,
                    (gchar const *) attr_i->name);
            goto error_handling;
        }

        track_filename = (gchar const *) attr_i->children->content;
    }

    for (node_i = root_node->children;
         node_i != NULL;
         node_i = node_i->next) {

        if (node_i->type == XML_TEXT_NODE &&
            !mutil_is_xml_string_all_whitespace(node_i->content)) {
            g_set_error(
                    o_error,
                    mutil_error_domain,
                    mutil_error_code_undefined,
                    "XML element '%s' contains non-whitespace text",
                    (gchar const *) root_node->name);
            goto error_handling;
        } else if (node_i->type == XML_TEXT_NODE) {
            /* Ignore all whitespace. */
        } else if (node_i->type == XML_ELEMENT_NODE &&
                   xmlStrEqual(
                       mutil_xmlstrdup(&tmp_xml_str_1, mutil_xml_tag_tag_list),
                       node_i->name)) {
            if (new_tag_list != NULL) {
                g_set_error(
                        o_error,
                        mutil_error_domain,
                        mutil_error_code_undefined,
                        "XML element '%s' contains multiple elements '%s'",
                        (gchar const *) root_node->name,
                        mutil_xml_tag_global);
                goto error_handling;
            }
            status = mutil_parse_xml_element_tag_list(
                    &new_tag_list,
                    node_i,
                    o_error);
            if (status == -1) {
                goto error_handling;
            }
        } else if (node_i->type == XML_ELEMENT_NODE) {
            g_set_error(
                    o_error,
                    mutil_error_domain,
                    mutil_error_code_undefined,
                    "XML element '%s' contains invalid element '%s'",
                    (gchar const *) root_node->name,
                    (gchar const *) node_i->name);
            goto error_handling;
        } else {
            /* Ignore all other node types. */
        }
    }

    status = mutil_determine_file_audio_type(
            track_filename,
            &track_audio_type,
            o_error);
    if (status == -1) {
        goto error_handling;
    }

    g_assert(new_track == NULL);
    new_track = mutil_track_alloc(track_filename, track_audio_type);
    mutil_track_add_tag_list(new_track, new_tag_list);

    *o_track = new_track;
    new_track = NULL;
    g_assert(*o_track != NULL);
    ret_value = 0;
    goto cleanup;

error_handling:

    ret_value = -1;
    g_assert(*o_track == NULL);

cleanup:

    mutil_xmlstrdup(&tmp_xml_str_1, NULL);
    mutil_tag_free_list_of(new_tag_list);
    mutil_track_free(new_track);

    return ret_value;
} /* mutil_parse_xml_element_track */

gint mutil_parse_xml_element_track_list(
        GList **o_track_list,
        xmlNode *root_node,
        GError **o_error)
{
    gint ret_value;
    xmlChar *tmp_xml_str_1 = NULL;
    xmlNode *xml_node_i;
    gint status;
    GList *new_track_list = NULL;
    GList *new_tag_list = NULL;
    mutil_track_t *new_track = NULL;
    GList *list_node_i;
    mutil_track_t *track_i;

    g_assert(o_track_list != NULL);
    g_assert(*o_track_list == NULL);
    g_assert(root_node != NULL);
    g_assert(root_node->type == XML_ELEMENT_NODE);
    g_assert(xmlStrEqual(
                mutil_xmlstrdup(&tmp_xml_str_1, mutil_xml_tag_track_list),
                root_node->name));

    if (root_node->properties != NULL) {
        g_set_error(
                o_error,
                mutil_error_domain,
                mutil_error_code_undefined,
                "XML element '%s' contains invalid property '%s'",
                (gchar const *) root_node->name,
                (gchar const *) root_node->properties->name);
        goto error_handling;
    }

    for (xml_node_i = root_node->children;
         xml_node_i != NULL;
         xml_node_i = xml_node_i->next) {

        if (xml_node_i->type == XML_TEXT_NODE &&
            !mutil_is_xml_string_all_whitespace(xml_node_i->content)) {
            g_set_error(
                    o_error,
                    mutil_error_domain,
                    mutil_error_code_undefined,
                    "XML element '%s' contains non-whitespace text",
                    (gchar const *) root_node->name);
            goto error_handling;
        } else if (xml_node_i->type == XML_TEXT_NODE) {
            /* Ignore all whitespace. */
        } else if (xml_node_i->type == XML_ELEMENT_NODE &&
                   xmlStrEqual(
                       mutil_xmlstrdup(&tmp_xml_str_1, mutil_xml_tag_global),
                       xml_node_i->name)) {
            if (new_tag_list != NULL) {
                g_set_error(
                        o_error,
                        mutil_error_domain,
                        mutil_error_code_undefined,
                        "XML element '%s' contains multiple elements '%s'",
                        (gchar const *) root_node->name,
                        mutil_xml_tag_global);
                goto error_handling;
            }
            status = mutil_parse_xml_element_global(
                    &new_tag_list,
                    xml_node_i,
                    o_error);
            if (status == -1) {
                goto error_handling;
            }
        } else if (xml_node_i->type == XML_ELEMENT_NODE &&
                   xmlStrEqual(
                       mutil_xmlstrdup(&tmp_xml_str_1, mutil_xml_tag_track),
                       xml_node_i->name)) {

            g_assert(new_track == NULL);
            status = mutil_parse_xml_element_track(
                    &new_track,
                    xml_node_i,
                    o_error);
            if (status == -1) {
                goto error_handling;
            }

            new_track_list = g_list_append(new_track_list, new_track);
            new_track = NULL;
            
        } else if (xml_node_i->type == XML_ELEMENT_NODE) {
            g_set_error(
                    o_error,
                    mutil_error_domain,
                    mutil_error_code_undefined,
                    "XML element '%s' contains invalid element '%s'",
                    (gchar const *) root_node->name,
                    (gchar const *) xml_node_i->name);
            goto error_handling;
        } else {
            /* Ignore all other node types. */
        }
    }

    /* Add global tags to all tracks. */
    for (list_node_i = new_track_list;
         list_node_i != NULL;
         list_node_i = list_node_i->next) {
        track_i = list_node_i->data;
        mutil_track_add_tag_list(track_i, new_tag_list);
    }

    *o_track_list = new_track_list;
    new_track_list = NULL;
    ret_value = 0;
    goto cleanup;

error_handling:

    ret_value = -1;
    g_assert(*o_track_list == NULL);

cleanup:

    mutil_xmlstrdup(&tmp_xml_str_1, NULL);
    mutil_track_free_list_of(new_track_list);
    mutil_tag_free_list_of(new_tag_list);
    mutil_track_free(new_track);

    return ret_value;
} /* mutil_parse_xml_element_track_list */

int mutil_print_xml_doc_to_stdout(
        xmlDoc *xml_doc,
        GError **o_error)
{
    int ret_value;
    xmlSaveCtxt *xml_save_ctx = NULL;
    long save_status;

    g_assert(xml_doc != NULL);
    g_assert(o_error == NULL || *o_error == NULL);

    xml_save_ctx = xmlSaveToFd(
            1,
            NULL,
            XML_SAVE_FORMAT | XML_SAVE_NO_EMPTY);
    if (xml_save_ctx == NULL) {
        g_set_error(
                o_error,
                mutil_error_domain,
                mutil_error_code_undefined,
                "failed to create XML save context");
        goto error_handling;
    }

    save_status = xmlSaveDoc(xml_save_ctx, xml_doc);
    if (save_status == -1) {
        g_set_error(
                o_error,
                mutil_error_domain,
                mutil_error_code_undefined,
                "failed to save XML document");
        goto error_handling;
    }

    save_status = xmlSaveFlush(xml_save_ctx);
    if (save_status == -1) {
        g_set_error(
                o_error,
                mutil_error_domain,
                mutil_error_code_undefined,
                "failed to flush XML document");
        goto error_handling;
    }

    g_assert(o_error == NULL || *o_error == NULL);

    ret_value = 0;
    goto cleanup;

error_handling:

    g_assert(o_error == NULL || *o_error != NULL);

    ret_value = -1;

cleanup:

    xmlSaveClose(xml_save_ctx);

    return ret_value;
} /* mutil_print_xml_doc_to_stdout */

xmlChar *mutil_xmlstrdup(
        xmlChar **o_str,
        gchar const *i_str)
{
    gint i_str_len;

    g_assert(o_str != NULL);

    if (*o_str != NULL) {
        free(*o_str);
        *o_str = NULL;
    }

    if (i_str != NULL) {
        i_str_len = strlen(i_str);
        *o_str = xmlCharStrndup(i_str, i_str_len);
        if (*o_str == NULL) {
            abort();
        }
    }

    return *o_str;
} /* mutil_xmlstrdup */

