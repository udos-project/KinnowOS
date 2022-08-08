#ifndef JDA_OBJECT_H
#define JDA_OBJECT_H

#include <stddef.h>
#include "token.hxx"
#include "context.hxx"
#include "number.hxx"

enum jda_object_type {
    JDA_OBJ_STRING,
    JDA_OBJ_NUMBER,
    JDA_OBJ_GROUP,
    JDA_OBJ_ROUTINE,
    JDA_OBJ_RAWPTR,
    JDA_OBJ_BOOL,
};

struct jda_object {
    /* For now the manual is only for builtins */
    const char *man_desc;
    char *name;
    enum jda_object_type type;
    size_t refcount;

    /* data of the object */
    struct jda_object **objects;
    size_t n_objects;
    struct jda_token *tokens;
    size_t n_tokens;
    void *data;
    jda_number_t numval;
    int (*func)(jda_context& ctx);
};

/* Creation */
struct jda_object *jda_object_create_group(jda_context& ctx, const char *name);
struct jda_object *jda_object_create_routine(jda_context& ctx, const char *name, int (*func)(jda_context& ctx));
struct jda_object *jda_object_create_integer(jda_context& ctx, const char *name, int num);
struct jda_object *jda_object_create_real_number(jda_context& ctx, const char *name, long double num);
struct jda_object *jda_object_create_number(jda_context& ctx, const char *name, jda_number_t num);
struct jda_object *jda_object_create_string(jda_context& ctx, const char *name, const char *str);
struct jda_object *jda_object_create_pointer(jda_context& ctx, const char *name, void *ptr);

/* Operations */
void jda_object_delete(struct jda_object *obj);
void jda_add_object(jda_context& ctx, struct jda_object *slave);
int jda_object_add_object(jda_context& ctx, struct jda_object *master, struct jda_object *slave);
struct jda_object *jda_stack_get_object(jda_context& ctx, int i);
struct jda_object *jda_stack_pop_object(jda_context& ctx);
struct jda_object *jda_get_object(jda_context& ctx, struct jda_object *master, const char *name);
void jda_remove_object(jda_context& ctx, struct jda_object *master, struct jda_object *slave);
int jda_stack_push_object(jda_context& ctx, struct jda_object *obj);

/* Conversion */
int jda_object_to_integer(jda_context& ctx, const struct jda_object *obj);
jda_number_t jda_object_to_number(jda_context& ctx, const struct jda_object *obj);
void *jda_object_to_rawptr(jda_context& ctx, const struct jda_object *obj);
const char *jda_object_to_string(jda_context& ctx, const struct jda_object *obj);
int jda_object_to_token(jda_context& ctx, const struct jda_object *obj, struct jda_token *tok);

#endif
