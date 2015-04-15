/* vim: tabstop=4:shiftwidth=4:expandtab:tw=80
 */

#ifndef mutil_tag_h
#define mutil_tag_h

#include "mutil_common.h"

#define mutil_tag_album "album"
#define mutil_tag_artist "artist"
#define mutil_tag_contact "contact"
#define mutil_tag_copyright "copyright"
#define mutil_tag_date "date"
#define mutil_tag_description "description"
#define mutil_tag_genre "genre"
#define mutil_tag_isrc "isrc"
#define mutil_tag_license "license"
#define mutil_tag_location "location"
#define mutil_tag_organization "organization"
#define mutil_tag_performer "performer"
#define mutil_tag_title "title"
#define mutil_tag_track_no "tracknumber"
#define mutil_tag_version "version"

struct mutil_tag;
typedef struct mutil_tag mutil_tag_t;

typedef GTree mutil_tag_map_t;

/* tag: */

mutil_tag_t *mutil_tag_alloc(
        gchar const *name,
        gchar const *value);

mutil_tag_t *mutil_tag_copy(
        mutil_tag_t *tag);

mutil_tag_t *mutil_tag_create_from_simple_assignment(
        gchar const *tag_text,
        gunichar separator,
        GError **o_error);

void mutil_tag_free(
        mutil_tag_t *tag);

void mutil_tag_free_list_of(
        GList *tag_list);

gchar const *mutil_tag_get_name(
        mutil_tag_t *tag);

gchar const *mutil_tag_get_value(
        mutil_tag_t *tag);

gboolean mutil_tag_is_name_equal_to(
        mutil_tag_t *tag,
        gchar const *tag_name);

/* tag map: */

void mutil_tag_map_add_tag(
        mutil_tag_map_t *tag_map,
        mutil_tag_t *tag);

mutil_tag_map_t *mutil_tag_map_alloc(void);

mutil_tag_map_t *mutil_tag_map_copy(
        mutil_tag_map_t *tag_map);

GList *mutil_tag_map_create_list(
        mutil_tag_map_t *tag_map);

void mutil_tag_map_free(
        mutil_tag_map_t *tag_map);

GList *mutil_tag_map_look_up(
        mutil_tag_map_t *tag_map,
        gchar const *tag_name);

#endif /* #ifndef mutil_tag_h */

