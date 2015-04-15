/* vim: tabstop=4:shiftwidth=4:expandtab:tw=80
 */

#include "mutil_album.h"
#include "mutil_makefile.h"

struct mutil_make_rule {
    GList *targets;
    GList *prereqs;
    GList *commands;
};

struct mutil_makefile {
    GList *rules;
};

static gchar *mutil_format_filename_safe_for_make(
        gchar const *src_filename);

static gchar *mutil_format_filename_safe_for_make(
        gchar const *src_filename)
{
    GString *safe_filename;
    gchar const *src_pos_i;
    gunichar src_ch;

    safe_filename = g_string_new("");

    for (src_pos_i = src_filename;
         *src_pos_i != '\0';
         src_pos_i = g_utf8_next_char(src_pos_i)) {

        src_ch = g_utf8_get_char(src_pos_i);

        if (src_ch == ' ') {
            g_string_append_c(safe_filename, '\\');
        }

        g_string_append_unichar(safe_filename, src_ch);
    }

    return g_string_free(safe_filename, FALSE);
} /* mutil_format_filename_safe_for_make */

mutil_make_rule_t *mutil_make_rule_alloc(void)
{
    mutil_make_rule_t *new_rule;

    new_rule = g_malloc0(sizeof(mutil_make_rule_t));

    return new_rule;
} /* mutil_make_rule_alloc */

void mutil_make_rule_append_command(
        mutil_make_rule_t *make_rule,
        gchar const *command)
{
    gchar *command_cp;

    g_assert(make_rule != NULL);
    g_assert(command != NULL);

    command_cp = g_strdup(command);
    make_rule->commands = g_list_append(make_rule->commands, command_cp);

    return;
} /* mutil_make_rule_append_command */

void mutil_make_rule_append_prereq(
        mutil_make_rule_t *make_rule,
        gchar const *filename)
{
    gchar *safe_filename;

    g_assert(make_rule != NULL);
    g_assert(filename != NULL);

    safe_filename = mutil_format_filename_safe_for_make(filename);
    make_rule->prereqs = g_list_append(make_rule->prereqs, safe_filename);

    return;
} /* mutil_make_rule_append_prereq */

void mutil_make_rule_append_target(
        mutil_make_rule_t *make_rule,
        gchar const *filename)
{
    gchar *safe_filename;

    g_assert(make_rule != NULL);
    g_assert(filename != NULL);

    safe_filename = mutil_format_filename_safe_for_make(filename);
    make_rule->targets = g_list_prepend(make_rule->targets, safe_filename);

    return;
} /* mutil_make_rule_append_target */

void mutil_make_rule_free(
        mutil_make_rule_t *make_rule)
{
    if (make_rule != NULL) {

        while (make_rule->targets != NULL) {
            g_free(make_rule->targets->data);
            make_rule->targets = g_list_delete_link(
                    make_rule->targets,
                    make_rule->targets);
        }

        while (make_rule->prereqs != NULL) {
            g_free(make_rule->prereqs->data);
            make_rule->prereqs = g_list_delete_link(
                    make_rule->prereqs,
                    make_rule->prereqs);
        }

        while (make_rule->commands != NULL) {
            g_free(make_rule->commands->data);
            make_rule->commands = g_list_delete_link(
                    make_rule->commands,
                    make_rule->commands);
        }

        g_free(make_rule);
    }

    return;
} /* mutil_make_rule_free */

mutil_makefile_t *mutil_makefile_alloc(void)
{
    mutil_makefile_t *new_makefile;

    new_makefile = g_malloc0(sizeof(mutil_makefile_t));

    return new_makefile;
} /* mutil_makefile_alloc */

void mutil_makefile_append_rule(
        mutil_makefile_t *makefile,
        mutil_make_rule_t *make_rule)
{
    g_assert(makefile != NULL);
    g_assert(make_rule != NULL);

    makefile->rules = g_list_append(makefile->rules, make_rule);

    return;
} /* mutil_makefile_append_rule */

void mutil_makefile_free(
        mutil_makefile_t *makefile)
{
    if (makefile != NULL) {

        while (makefile->rules != NULL) {
            mutil_make_rule_free(makefile->rules->data);
            makefile->rules = g_list_delete_link(
                    makefile->rules,
                    makefile->rules);
        }

        g_free(makefile);
    }

    return;
} /* mutil_makefile_free */

void mutil_makefile_prepend_rule(
        mutil_makefile_t *makefile,
        mutil_make_rule_t *make_rule)
{
    g_assert(makefile != NULL);
    g_assert(make_rule != NULL);

    makefile->rules = g_list_prepend(makefile->rules, make_rule);

    return;
} /* mutil_makefile_prepend_rule */

gchar *mutil_makefile_to_string(
        mutil_makefile_t *makefile)
{
    GString *makefile_text = NULL;
    GList *node_i;
    mutil_make_rule_t *make_rule_i;
    GList *node_j;

    g_assert(makefile != NULL);

    makefile_text = g_string_new("# vim: set filetype=make:\n\n");

    for (node_i = makefile->rules;
         node_i != NULL;
         node_i = node_i->next) {

        make_rule_i = node_i->data;

        for (node_j = make_rule_i->targets;
             node_j != NULL;
             node_j = node_j->next) {
            g_string_append_printf(
                    makefile_text,
                    "%s%s",
                    (gchar const *) node_j->data,
                    node_j->next != NULL ? " " : ":");
        }
        
        for (node_j = make_rule_i->prereqs;
             node_j != NULL;
             node_j = node_j->next) {
            g_string_append_printf(
                    makefile_text,
                    " %s",
                    (gchar const *) node_j->data);
        }

        g_string_append(makefile_text, "\n");

        for (node_j = make_rule_i->commands;
             node_j != NULL;
             node_j = node_j->next) {
            g_string_append_printf(
                    makefile_text,
                    "\t%s\n",
                    (gchar const *) node_j->data);
        }
        
        g_string_append(makefile_text, "\n");
    }

    g_assert(makefile_text != NULL);
    return g_string_free(makefile_text, FALSE);
} /* mutil_makefile_to_string */

