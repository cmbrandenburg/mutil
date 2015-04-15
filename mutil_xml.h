/* vim: tabstop=4:shiftwidth=4:expandtab:tw=80
 */

#ifndef mutil_xml_h
#define mutil_xml_h

#include "mutil_common.h"

gint mutil_create_track_list_from_xml_doc(
        xmlDoc *xml_doc,
        GList **o_track_list,
        GError **o_error);

xmlDoc *mutil_create_xml_doc_from_track_list(
        GList *track_list,
        gboolean opt_flag_create_global_section);

int mutil_print_xml_doc_to_stdout(
        xmlDoc *xml_doc,
        GError **o_error);

#endif /* #ifndef mutil_xml_h */

