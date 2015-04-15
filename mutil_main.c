/* vim: tabstop=4:shiftwidth=4:expandtab:tw=80
 */

#include "mutil_album.h"
#include "mutil_main.h"
#include "mutil_makefile.h"
#include "mutil_track.h"
#include "mutil_xml.h"

struct mutil_cl_info;
typedef struct mutil_cl_info mutil_cl_info_t;

struct mutil_cl_info {
    gboolean cmd_flag_archive;
    gboolean opt_flag_auto_track_no_tags;
    gboolean opt_flag_create_global_section;
    gboolean cmd_flag_generate_xml;
    gboolean cmd_flag_oggify;
    gboolean opt_flag_simple_album;
    gboolean opt_flag_use_echo_e;
    gboolean opt_flag_verbose_makefile;
    gchar const *xml_spec_filename;
    gint arg_list_sz;
    gchar const **arg_list;
};

static gint mutil_parse_command_line(
        mutil_cl_info_t *o_cl_info,
        gint *o_argc,
        gchar ***o_argv,
        GError **o_error);

static gint mutil_run_command_archive(
        gchar const *xml_spec_filename,
        gboolean opt_flag_verbose_makefile,
        gboolean opt_flag_simple_album,
        gboolean opt_flag_use_echo_e,
        GError **o_error);

static gint mutil_run_command_generate_xml(
        gchar const * const *audio_filenames,
        gint audio_filename_cnt,
        gboolean opt_flag_auto_track_no_tags,
        gboolean opt_flag_simple_album,
        gboolean opt_flag_create_global_section,
        GError **o_error);

static gint mutil_run_command_oggify(
        gchar const * const *audio_filenames,
        gint audio_filename_cnt,
        gboolean opt_flag_verbose_makefile,
        gboolean opt_flag_simple_album,
        gboolean opt_flag_use_echo_e,
        GError **o_error);

gint main(
        gint argc,
        char **argv)
{
    gint ret_value;
    GError *local_error = NULL;
    gint status;
    mutil_cl_info_t cl_info;

    /* Parse the command line. */
    status = mutil_parse_command_line(&cl_info, &argc, &argv, &local_error);
    if (status == -1) {
        goto error_handling;
    }

    /* Dispatch command. */
    if (cl_info.cmd_flag_archive) {
        status = mutil_run_command_archive(
                cl_info.xml_spec_filename,
                cl_info.opt_flag_verbose_makefile,
                cl_info.opt_flag_simple_album,
                cl_info.opt_flag_use_echo_e,
                &local_error);
        if (status == -1) {
            goto error_handling;
        }
    } else if (cl_info.cmd_flag_generate_xml) {
        status = mutil_run_command_generate_xml(
                cl_info.arg_list,
                cl_info.arg_list_sz,
                cl_info.opt_flag_auto_track_no_tags,
                cl_info.opt_flag_simple_album,
                cl_info.opt_flag_create_global_section,
                &local_error);
        if (status == -1) {
            goto error_handling;
        }
    } else if (cl_info.cmd_flag_oggify) {
        status = mutil_run_command_oggify(
                cl_info.arg_list,
                cl_info.arg_list_sz,
                cl_info.opt_flag_verbose_makefile,
                cl_info.opt_flag_simple_album,
                cl_info.opt_flag_use_echo_e,
                &local_error);
        if (status == -1) {
            goto error_handling;
        }
    } else {
        g_assert(FALSE);
    }

    g_assert(local_error == NULL);
    ret_value = EXIT_SUCCESS;
    goto cleanup;

error_handling:

    g_assert(local_error != NULL);
    g_fprintf(
            stderr,
            "*** a fatal error occurred: %s:%d:%s\n",
            g_quark_to_string(local_error->domain),
            local_error->code,
            local_error->message);
    g_clear_error(&local_error);

    ret_value = EXIT_FAILURE;

cleanup:

    g_assert(local_error == NULL);

    g_free(cl_info.arg_list);

    return ret_value;
} /* main */

gint mutil_parse_command_line(
        mutil_cl_info_t *o_cl_info,
        gint *o_argc,
        gchar ***o_argv,
        GError **o_error)
{
    GOptionEntry const opt_ctx_main_entries[] = {
        {"archive", 0, 0, G_OPTION_ARG_NONE, &o_cl_info->cmd_flag_archive,
            "To FLAC using XML file argument", NULL},
        {"auto-track-no", 0, 0, G_OPTION_ARG_NONE,
            &o_cl_info->opt_flag_auto_track_no_tags,
            "Auto-generate track-number tags", NULL},
        {"create-global", 0, 0, G_OPTION_ARG_NONE,
            &o_cl_info->opt_flag_create_global_section,
            "Create an empty global section in XML", NULL},
        {"generate-xml", 0, 0, G_OPTION_ARG_NONE,
            &o_cl_info->cmd_flag_generate_xml,
            "Write XML output using audio file arguments", NULL},
        /* use-echo-e:
         *
         * Make executes commands like 'ls $$(echo .)' using '/bin/sh'. On some
         * systems, '/bin/sh' links to a shell that uses a built-in 'echo' that
         * doesn't support the '-e' argument (e.g., 'dash' does this). On some
         * systems, '/bin/sh' links to a sell whose 'echo' does support '-e'. If
         * '-e' is used and the built-in doesn't support it, then tag info will
         * be corrupted with extraneous '-e' prefixes. */
        {"use-echo-e", 0, 0, G_OPTION_ARG_NONE, &o_cl_info->opt_flag_use_echo_e,
            "Never use '-e' argument in 'echo'", NULL},
        {"oggify", 0, 0, G_OPTION_ARG_NONE, &o_cl_info->cmd_flag_oggify,
            "To OGG using audio file arguments", NULL},
        {"simple-album", 0, 0, G_OPTION_ARG_NONE,
            &o_cl_info->opt_flag_simple_album,
            "Group tracks using ALBUM tag only", NULL},
        {"verbose-makefile", 0, 0, G_OPTION_ARG_NONE,
            &o_cl_info->opt_flag_verbose_makefile, "Generate verbose makefile",
            NULL},
        {NULL}
    };

    gint ret_value;
    GOptionContext *opt_ctx = NULL;
    gboolean parse_result;
    gint cmd_cnt;

    g_assert(o_cl_info != NULL);
    g_assert(o_argc != NULL);
    g_assert(o_argv != NULL);
    g_assert(*o_argv != NULL);
    g_assert(o_error == NULL || *o_error == NULL);

    /* Parse the command line. */
    opt_ctx = g_option_context_new("[ARGUMENT(S)]");
    g_option_context_set_summary(
            opt_ctx,
            "The command line must specify exactly one of the following "
            "command options:\n"
            "  --archive\n"
            "  --generate-xml\n"
            "  --oggify");
    g_option_context_add_main_entries(opt_ctx, opt_ctx_main_entries, NULL);
    memset(o_cl_info, 0, sizeof(mutil_cl_info_t));
    parse_result = g_option_context_parse(opt_ctx, o_argc, o_argv, o_error);
    if (!parse_result) {
        goto error_handling;
    }

    /* Check that exactly one command is specified. */
    cmd_cnt =
        (o_cl_info->cmd_flag_archive ? 1 : 0) +
        (o_cl_info->cmd_flag_generate_xml ? 1 : 0) +
        (o_cl_info->cmd_flag_oggify ? 1 : 0);

    if (cmd_cnt < 1) {
        g_set_error(
                o_error,
                mutil_error_domain,
                mutil_error_code_undefined,
                "command line does not specify command to run");
        goto error_handling;
    }

    if (cmd_cnt > 1) {
        g_set_error(
                o_error,
                mutil_error_domain,
                mutil_error_code_undefined,
                "command line specifies multiple commands to run");
        goto error_handling;
    }

    /* Verify that exactly one argument is specified if the 'archive' command is
     * specified. */
    if (o_cl_info->cmd_flag_archive && *o_argc < 2) {
        g_set_error(
                o_error,
                mutil_error_domain,
                mutil_error_code_undefined,
                "command line does not specify XML spec file");
        goto error_handling;
    }

    if (o_cl_info->cmd_flag_archive && *o_argc > 2) {
        g_set_error(
                o_error,
                mutil_error_domain,
                mutil_error_code_undefined,
                "command line specifies multiple XML spec files");
        goto error_handling;
    }

    if (o_cl_info->cmd_flag_archive) {
        o_cl_info->xml_spec_filename = (*o_argv)[1];
    }

    /* Allocate argument lits if the 'generate-xml' or 'oggify' commands are
     * specified. */
    if (o_cl_info->cmd_flag_generate_xml ||
        o_cl_info->cmd_flag_oggify) {
        o_cl_info->arg_list = g_malloc0((*o_argc - 1) * sizeof(gchar const *));
        memcpy(
                o_cl_info->arg_list,
                &(*o_argv)[1],
                (*o_argc - 1) * sizeof(gchar const *));
        o_cl_info->arg_list_sz = *o_argc - 1;
    }

    g_assert(o_error == NULL || *o_error == NULL);
    ret_value = 0;
    goto cleanup;

error_handling:

    g_assert(o_error == NULL || *o_error != NULL);
    g_prefix_error(
            o_error,
            "failed to parse command line: ");

    ret_value = -1;

cleanup:

    if (opt_ctx != NULL) {
        g_option_context_free(opt_ctx);
    }

    return ret_value;
} /* mutil_parse_command_line */

gint mutil_run_command_archive(
        gchar const *xml_spec_filename,
        gboolean opt_flag_verbose_makefile,
        gboolean opt_flag_simple_album,
        gboolean opt_flag_use_echo_e,
        GError **o_error)
{

    gint ret_value;
    xmlDoc *xml_doc = NULL;
    GList *track_list = NULL;
    gint status;
    GList *album_list = NULL;
    mutil_makefile_t *makefile = NULL;
    gchar *makefile_text = NULL;

    g_assert(xml_spec_filename != NULL);
    g_assert(o_error == NULL || *o_error == NULL);

    /* Create the track list from the XML specification. */

    xml_doc = xmlParseFile(xml_spec_filename);
    if (xml_doc == NULL) {
        g_set_error(
                o_error,
                mutil_error_domain,
                mutil_error_code_undefined,
                "failed to parse XML file '%s'",
                xml_spec_filename);
        goto error_handling;
    }

    g_assert(track_list == NULL);
    status = mutil_create_track_list_from_xml_doc(
            xml_doc,
            &track_list,
            o_error);
    if (status == -1) {
        goto error_handling;
    }

    /* Separate the tracks into albums based on their metadata. Do some sanity
     * checking. */

    g_assert(album_list == NULL);
    status = mutil_album_list_create_from_track_list(
            &album_list,
            track_list,
            opt_flag_simple_album,
            TRUE, /* enable sanity warnings */
            FALSE, /* don't auto-generate track-number tags */
            o_error);
    if (status == -1) {
        goto error_handling;
    }

    /* Generate and print the makefile. */
    g_assert(makefile == NULL);
    makefile = mutil_album_list_generate_archive_makefile(
            album_list,
            opt_flag_verbose_makefile,
            opt_flag_use_echo_e);
    makefile_text = mutil_makefile_to_string(makefile);
    g_printf("%s", makefile_text);

    /* Output the makefile. */

    g_assert(o_error == NULL || *o_error == NULL);
    ret_value = 0;
    goto cleanup;

error_handling:

    g_assert(o_error == NULL || *o_error != NULL);

    ret_value = -1;

cleanup:

    xmlFreeDoc(xml_doc);
    mutil_track_free_list_of(track_list);
    mutil_album_list_free(album_list);
    mutil_makefile_free(makefile);
    g_free(makefile_text);

    return ret_value;
} /* mutil_run_command_archive */

gint mutil_run_command_generate_xml(
        gchar const * const *audio_filenames,
        gint audio_filename_cnt,
        gboolean opt_flag_auto_track_no_tags,
        gboolean opt_flag_simple_album,
        gboolean opt_flag_create_global_section,
        GError **o_error)
{
    gint ret_value;
    gint status;
    GList *track_list = NULL;
    GList *album_list = NULL;
    xmlDoc *xml_doc = NULL;

    g_assert(o_error == NULL || *o_error == NULL);

    /* Create the XML document and write it to standard out. */

    g_assert(track_list == NULL);
    status = mutil_create_track_list_from_audio_files(
            audio_filenames,
            audio_filename_cnt,
            &track_list,
            o_error);
    if (status == -1) {
        goto error_handling;
    }

    if (opt_flag_auto_track_no_tags) {

        g_assert(album_list == NULL);
        status = mutil_album_list_create_from_track_list(
                &album_list,
                track_list,
                opt_flag_simple_album,
                FALSE, /* disable sanity warnings */
                TRUE, /* auto-generate track-number tags */
                NULL);
        if (status == -1) {
            /* There isn't enough metadata information to generate albums.
             * Number the tracks sequentially. */
            mutil_track_list_generate_track_number_tags(track_list);
        } else {
            mutil_track_free_list_of(track_list);
            track_list = mutil_album_list_create_track_list(album_list);
        }
    }


    xml_doc = mutil_create_xml_doc_from_track_list(
            track_list,
            opt_flag_create_global_section);

    status = mutil_print_xml_doc_to_stdout(xml_doc, o_error);
    if (status == -1) {
        goto error_handling;
    }

    g_assert(o_error == NULL || *o_error == NULL);
    ret_value = 0;
    goto cleanup;

error_handling:

    g_assert(o_error == NULL || *o_error != NULL);

    ret_value = -1;

cleanup:

    mutil_track_free_list_of(track_list);
    mutil_album_list_free(album_list);
    xmlFreeDoc(xml_doc);

    return ret_value;
} /* mutil_run_command_generate_xml */

gint mutil_run_command_oggify(
        gchar const * const *audio_filenames,
        gint audio_filename_cnt,
        gboolean opt_flag_verbose_makefile,
        gboolean opt_flag_simple_album,
        gboolean opt_flag_use_echo_e,
        GError **o_error)
{
    gint ret_value;
    gint status;
    GList *track_list = NULL;
    GList *album_list = NULL;
    mutil_makefile_t *makefile = NULL;
    gchar *makefile_text = NULL;

    g_assert(o_error == NULL || *o_error == NULL);

    /* Create a track for each audio file, and separate the tracks into albums.
     * */

    status = mutil_create_track_list_from_audio_files(
            audio_filenames,
            audio_filename_cnt,
            &track_list,
            o_error);
    if (status == -1) {
        goto error_handling;
    }

    g_assert(album_list == NULL);
    status = mutil_album_list_create_from_track_list(
            &album_list,
            track_list,
            opt_flag_simple_album,
            FALSE, /* disable sanity warnings */
            FALSE, /* don't auto-generate track-number tags */
            o_error);
    if (status == -1) {
        goto error_handling;
    }

    /* Generate and print the makefile. */
    g_assert(makefile == NULL);
    makefile = mutil_album_list_generate_oggify_makefile(
            album_list,
            opt_flag_verbose_makefile,
            opt_flag_use_echo_e);
    makefile_text = mutil_makefile_to_string(makefile);
    g_printf("%s", makefile_text);

    g_assert(o_error == NULL || *o_error == NULL);
    ret_value = 0;
    goto cleanup;

error_handling:

    g_assert(o_error == NULL || *o_error != NULL);

    ret_value = -1;

cleanup:

    mutil_album_list_free(album_list);
    mutil_track_free_list_of(track_list);
    mutil_makefile_free(makefile);
    g_free(makefile_text);

    return ret_value;
} /* mutil_run_command_oggify */

