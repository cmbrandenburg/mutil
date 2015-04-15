/* vim: tabstop=4:shiftwidth=4:expandtab:tw=80
 */

#ifndef mutil_makefile_h
#define mutil_makefile_h

#include "mutil_common.h"

#define mutil_makefile_phony ".PHONY"

struct mutil_make_rule;
typedef struct mutil_make_rule mutil_make_rule_t;

struct mutil_makefile;
typedef struct mutil_makefile mutil_makefile_t;

/* make_rule: */

void mutil_make_rule_append_command(
        mutil_make_rule_t *make_rule,
        gchar const *command);

void mutil_make_rule_append_prereq(
        mutil_make_rule_t *make_rule,
        gchar const *filename);

void mutil_make_rule_append_target(
        mutil_make_rule_t *make_rule,
        gchar const *filename);

mutil_make_rule_t *mutil_make_rule_alloc(void);

void mutil_make_rule_free(
        mutil_make_rule_t *make_rule);

/* makefile: */

mutil_makefile_t *mutil_makefile_alloc(void);

void mutil_makefile_append_rule(
        mutil_makefile_t *makefile,
        mutil_make_rule_t *make_rule);

void mutil_makefile_free(
        mutil_makefile_t *makefile);

void mutil_makefile_prepend_rule(
        mutil_makefile_t *makefile,
        mutil_make_rule_t *make_rule);

gchar *mutil_makefile_to_string(
        mutil_makefile_t *makefile);

#endif /* #ifndef mutil_makefile_h */

