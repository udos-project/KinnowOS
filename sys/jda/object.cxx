#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "jda.hxx"
#include "object.hxx"

struct jda_object *jda_object_create_group(jda_context& ctx, const char *name)
{
    assert(name != nullptr);
    auto *object = (jda_object *)calloc(1, sizeof(struct jda_object));
    if(object == nullptr) {
        return nullptr;
    }
    object->type = JDA_OBJ_GROUP;
    object->name = (char *)malloc(strlen(name) + 1);
    if(object->name == nullptr) {
        jda_report_error(ctx, "Out of memory\r\n");
        free(object);
        return nullptr;
    }
    strcpy(object->name, name);
    object->refcount++;
    return object;
}

struct jda_object *jda_object_create_routine(jda_context& ctx, const char *name, int (*func)(jda_context& ctx))
{
    assert(name != nullptr);
    auto *object = (jda_object *)calloc(1, sizeof(struct jda_object));
    if(object == nullptr) {
        return nullptr;
    }
    object->type = JDA_OBJ_ROUTINE;
    object->func = func;
    object->name = (char *)malloc(strlen(name) + 1);
    if(object->name == nullptr) {
        jda_report_error(ctx, "Out of memory\r\n");
        free(object);
        return nullptr;
    }
    strcpy(object->name, name);
    object->refcount++;
    return object;
}

struct jda_object *jda_object_create_integer(jda_context& ctx, const char *name, int num)
{
    assert(name != nullptr);
    auto *object = (jda_object *)calloc(1, sizeof(jda_object));
    if(object == nullptr) {
        return nullptr;
    }
    object->type = JDA_OBJ_NUMBER;
    object->numval.real_value = (long double)num;
    object->numval.imaginary_value = 0.f;
    object->numval.power = 1.f;
    object->name = (char *)malloc(strlen(name) + 1);
    if(object->name == nullptr) {
        jda_report_error(ctx, "Out of memory\r\n");
        free(object);
        return nullptr;
    }
    strcpy(object->name, name);
    object->refcount++;
    return object;
}

struct jda_object *jda_object_create_real_number(jda_context& ctx, const char *name, long double num)
{
    struct jda_object *object;

    assert(name != nullptr);
    object = (jda_object *)calloc(1, sizeof(struct jda_object));
    if(object == nullptr) {
        return nullptr;
    }
    object->type = JDA_OBJ_NUMBER;
    object->numval.real_value = num;
    object->numval.imaginary_value = 0.f;
    object->numval.power = 1.f;
    object->name = (char *)malloc(strlen(name) + 1);
    if(object->name == nullptr) {
        jda_report_error(ctx, "Out of memory\r\n");
        free(object);
        return nullptr;
    }
    strcpy(object->name, name);
    object->refcount++;
    return object;
}

struct jda_object *jda_object_create_number(jda_context& ctx, const char *name, jda_number_t num)
{
    assert(name != nullptr);
    auto *object = (jda_object *)calloc(1, sizeof(jda_object));
    if(object == nullptr) {
        return nullptr;
    }
    object->type = JDA_OBJ_NUMBER;
    object->numval = num;
    object->name = (char *)malloc(strlen(name) + 1);
    if(object->name == nullptr) {
        jda_report_error(ctx, "Out of memory\r\n");
        free(object);
        return nullptr;
    }
    strcpy(object->name, name);
    object->refcount++;
    return object;
}

struct jda_object *jda_object_create_string(jda_context& ctx, const char *name, const char *str)
{
    struct jda_object *object;

    assert(name != nullptr);
    object = (jda_object *)calloc(1, sizeof(struct jda_object));
    if(object == nullptr) {
        jda_report_error(ctx, "Out of memory\r\n");
        return nullptr;
    }
    object->type = JDA_OBJ_STRING;
    object->data = (void *)malloc(strlen(str) + 1);
    if(object->data == nullptr) {
        jda_report_error(ctx, "Out of memory\r\n");
        free(object);
        return nullptr;
    }
    strcpy((char *)object->data, str);
    object->name = (char *)malloc(strlen(name) + 1);
    if(object->name == nullptr) {
        jda_report_error(ctx, "Out of memory\r\n");
        free(object->data);
        free(object);
        return nullptr;
    }
    strcpy(object->name, name);
    object->refcount++;
    return object;
}

struct jda_object *jda_object_create_pointer(jda_context& ctx, const char *name, void *ptr)
{
    assert(name != nullptr);
    auto *object = (jda_object *)calloc(1, sizeof(jda_object));
    if(object == nullptr) {
        jda_report_error(ctx, "Out of memory\r\n");
        return nullptr;
    }
    object->type = JDA_OBJ_RAWPTR;
    object->data = ptr;
    object->name = (char *)malloc(strlen(name) + 1);
    if(object->name == nullptr) {
        jda_report_error(ctx, "Out of memory\r\n");
        free(object);
        return nullptr;
    }
    strcpy(object->name, name);
    object->refcount++;
    return object;
}

void jda_object_delete(struct jda_object *obj)
{
    assert(obj != nullptr);
    if(obj->objects != nullptr) {
        size_t i;
        for(i = 0; i < obj->n_objects; i++) {
            jda_object_delete(obj->objects[i]);
        }
        free(obj->objects);
    }
    if(obj->tokens != nullptr) {
        free(obj->tokens);
    }
    if(obj->name != nullptr) {
        free(obj->name);
    }
    if(obj->data != nullptr) {
        free(obj->data);
    }
    free(obj);
    return;
}

/**
 * @brief Add an object to the global list of objects
 * 
 * @param ctx 
 * @param slave The object in question
 */
void jda_add_object(jda_context& ctx, struct jda_object *slave)
{
    assert(slave != nullptr);
    ctx.objects = (jda_object **)realloc(ctx.objects, (ctx.n_objects + 1) * sizeof(jda_object *));
    if(ctx.objects == nullptr) {
        jda_report_error(ctx, "Out of memory\r\n");
        return;
    }
    ctx.objects[ctx.n_objects] = slave;
    ctx.n_objects++;
    return;
}

/**
 * @brief Add an object to an object group
 * 
 * @param ctx 
 * @param master Group that this object is to be added on
 * @param slave The object to be added
 * @return int Return condition. Negative means error.
 */
int jda_object_add_object(jda_context& ctx, struct jda_object *master, struct jda_object *slave)
{
    assert(master != nullptr && slave != nullptr);
    if(master->type != JDA_OBJ_GROUP) {
        jda_report_error(ctx, "Can't add %s to non-group %s\r\n", slave->name, master->name);
        return -1;
    }

    master->objects = (jda_object **)realloc(master->objects, (master->n_objects + 1) * sizeof(jda_object *));
    master->objects[master->n_objects++] = slave;
    slave->refcount++;
    return 0;
}

/**
 * @brief Obtain an object from the object stack
 * 
 * @param ctx 
 * @param i The index of the object in the stack
 * @return struct jda_object* 
 */
struct jda_object *jda_stack_get_object(jda_context& ctx, int i)
{
    size_t idx = ctx.n_stacks - (size_t)(1 + i);
    if(ctx.n_stacks == 0) {
        jda_printf(ctx, "Stack is empty\r\n");
        return nullptr;
    }

    if((int)idx < 0) {
        jda_report_error(ctx, "Index out of bounds\r\n");
        return nullptr;
    }

    auto *obj = ctx.stacks[idx];
    return obj;
}

struct jda_object *jda_stack_pop_object(jda_context& ctx)
{
    if(ctx.n_stacks == 0) {
        jda_printf(ctx, "Stack is empty\r\n");
        return nullptr;
    }

    auto *obj = ctx.stacks[ctx.n_stacks - 1];
    ctx.n_stacks--;
    return obj;
}

struct jda_object *jda_get_object(jda_context& ctx, struct jda_object *master, const char *name)
{
    size_t i;

    assert(name != nullptr);
    if(master == nullptr) {
        for(i = 0; i < ctx.n_objects; i++) {
            struct jda_object *obj = ctx.objects[i];
            if(!strcmp(obj->name, name)) {
                return obj;
            }
        }
    } else {
        for(i = 0; i < master->n_objects; i++) {
            struct jda_object *obj = master->objects[i];
            /* Only groups can have objects */
            assert(master->type == JDA_OBJ_GROUP);
            if(!strcmp(obj->name, name)) {
                return obj;
            }
        }
    }
    return nullptr;
}

void jda_remove_object(jda_context& ctx, struct jda_object *master, struct jda_object *slave)
{
    size_t i;

    assert(slave != nullptr);
    if(master == nullptr) {
        for(i = 0; i < ctx.n_objects; i++) {
            if(ctx.objects[i] == slave) {
                memmove(&ctx.objects[i], &ctx.objects[i + 1], sizeof(struct jda_object *) * (ctx.n_objects - i));
                ctx.n_objects--;
                break;
            }
        }
    } else {
        for(i = 0; i < master->n_objects; i++) {
            if(master->objects[i] == slave) {
                memmove(&master->objects[i], &master->objects[i + 1], sizeof(struct jda_object *) * (master->n_objects - i));
                master->n_objects--;
                break;
            }
        }
    }
    return;
}

int jda_stack_push_object(jda_context& ctx, struct jda_object *obj)
{
    assert(obj != nullptr);
    ctx.stacks = (jda_object **)realloc(ctx.stacks, (ctx.n_stacks + 1) * sizeof(struct jda_object *));
    if(ctx.stacks == nullptr) {
        jda_report_error(ctx, "Out of memory\r\n");
        return -1;
    }
    ctx.stacks[ctx.n_stacks++] = obj;
    obj->refcount++;
    return 0;
}

int jda_object_to_integer(jda_context& ctx, const struct jda_object *obj)
{
    assert(obj != nullptr);
    if(obj->type == JDA_OBJ_NUMBER) {
        return (int)obj->numval.real_value;
    } else if(obj->type == JDA_OBJ_STRING) {
        int num = atoi((const char *)obj->data);
        return num;
    }
    jda_report_error(ctx, "%s can't be converted to a number\r\n", obj->name);
    return 0;
}

jda_number_t jda_object_to_number(jda_context& ctx, const struct jda_object *obj)
{
    jda_number_t num = (jda_number_t){ 0.f, 0.f, 1.f };
    assert(obj != nullptr);
    if(obj->type == JDA_OBJ_NUMBER) {
        return obj->numval;
    } else if(obj->type == JDA_OBJ_STRING) {
        num.real_value = atof((const char *)obj->data);
        return num;
    }
    jda_report_error(ctx, "%s can't be converted to a number\r\n", obj->name);
    return num;
}

void *jda_object_to_rawptr(jda_context& ctx, const struct jda_object *obj)
{
    assert(obj != nullptr);
    if(obj->type != JDA_OBJ_RAWPTR) {
        jda_report_error(ctx, "%s expected to be a rawptr\r\n", obj->name);
        return nullptr;
    }
    return obj->data;
}

const char *jda_object_to_string(jda_context& ctx, const struct jda_object *obj)
{
    static char tmpbuf[100];
    assert(obj != nullptr);
    if(obj->type == JDA_OBJ_STRING) {
        return (const char *)obj->data;
    } else if(obj->type == JDA_OBJ_NUMBER) {
        if(obj->numval.imaginary_value == 0.f) {
            if(obj->numval.power == 1.f) {
                snprintf(tmpbuf, sizeof(tmpbuf), "%llf", obj->numval.real_value);
            } else {
                snprintf(tmpbuf, sizeof(tmpbuf), "%llf^%llf", obj->numval.real_value, obj->numval.power);
            }
        } else {
            if(obj->numval.power == 1.f) {
                snprintf(tmpbuf, sizeof(tmpbuf), "%llf+%llfi", obj->numval.real_value, obj->numval.imaginary_value);
            } else {
                snprintf(tmpbuf, sizeof(tmpbuf), "(%llf+%llfi)^%llf", obj->numval.real_value, obj->numval.imaginary_value, obj->numval.power);
            }
        }
        return tmpbuf;
    } else if(obj->type == JDA_OBJ_GROUP) {
        return obj->name;
    } else if(obj->type == JDA_OBJ_ROUTINE) {
        snprintf(tmpbuf, sizeof(tmpbuf), "%p", obj->func);
        return tmpbuf;
    } else if(obj->type == JDA_OBJ_RAWPTR) {
        snprintf(tmpbuf, sizeof(tmpbuf), "%p", obj->data);
        return tmpbuf;
    }
    jda_report_error(ctx, "%s expected to be a string\r\n", obj->name);
    return nullptr;
}

int jda_object_to_token(jda_context& ctx, const struct jda_object *obj, struct jda_token *tok)
{
    if(obj->type == JDA_OBJ_NUMBER) {
        tok->numval = obj->numval;
        tok->type = JDA_TOK_NUMBER;
    } else if(obj->type == JDA_OBJ_STRING) {
        tok->data = (char *)realloc(tok->data, strlen((const char *)obj->data) + 1);
        strcpy((char *)tok->data, (const char *)obj->data);
        tok->type = JDA_TOK_STRING;
    } else if(obj->type == JDA_OBJ_ROUTINE) {
        tok->data = (char *)realloc(tok->data, strlen((const char *)obj->name) + 1);
        strcpy((char *)tok->data, (const char *)obj->data);
        tok->type = JDA_TOK_FUNCTION;
        tok->arg_count = 0;
        /*jda_report_error(ctx, "%s is not an identifier, it's a function, use \"%s()\" instead\r\n", obj->name, obj->name);
        return -1;*/
    } else {
        jda_report_error(ctx, "Can't perform substitution for %s\r\n", obj->name);
        return -2;
    }
    return 0;
}
