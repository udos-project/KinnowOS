#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <x3270.h>
#include <assert.h>
#include <job.h>

#include "context.hxx"
#include "jda.hxx"
#include "globals.hxx"

/* TODO: Don't use this ugly hack */
static jda_context *g_ctx;
static void jda_print_top(jda_context& ctx)
{
    job_stats stats;
    const char *free_prefix, *used_prefix;
    size_t real_free_size, real_used_size;

    job_get_stats(&stats); /* Query memory stats (might take a while) */
    free_prefix = jda_get_unitsize(stats.free_size, &real_free_size);
    used_prefix = jda_get_unitsize(stats.used_size, &real_used_size);
    jda_printf(ctx, "JDA on UDOS - %u %sB (%u %sB used) @ %u regions\r\n", real_free_size, free_prefix, real_used_size, used_prefix, stats.n_regions);
}

static int jda_puts_vpage(x3270_term *term)
{
    // Allow the user to quickly review next pages
    term->x = 0;
    term->y = 0;

    x3270_clear_screen(term);
    jda_print_top(*g_ctx);
    return 0;
}

int jda_context_init(jda_context& ctx)
{
    g_ctx = &ctx; // TODO: Don't do this hack

    auto *dev = css_get_device(1, 1);
    if(dev == nullptr) {
        printf("No terminal\r\n");
        return -1;
    }

    if(x3270_start_term(&ctx.term, dev) < 0) {
        printf("Can't enable term\r\n");
        return -1;
    }

    /* Initialize the buffer for the terminal */
    ctx.n_term_buf = (size_t)ctx.term.cols * (size_t)ctx.term.rows * 2;
    ctx.term_buf = (char *)calloc(ctx.n_term_buf, sizeof(ctx.term_buf[0]));
    if(ctx.term_buf == nullptr) {
        printf("Can't get term buffer\r\n");
        return -1;
    }
    ctx.term.vert_paging = &jda_puts_vpage;
    x3270_clear_screen(&ctx.term);

    dprintf("Registering builtin JDA objects");
    // Initialize JDA
    struct jda_object *fn_obj;
    fn_obj = jda_object_create_routine(ctx, "TEST", &jda_builtin_test);
    fn_obj->man_desc = g_msg[MSG_MANUAL_TEST];
    jda_add_object(ctx, fn_obj);
    fn_obj = jda_object_create_routine(ctx, "STACKDUMP", &jda_builtin_stackdump);
    fn_obj->man_desc = g_msg[MSG_MANUAL_STACKDUMP];
    jda_add_object(ctx, fn_obj);
    fn_obj = jda_object_create_routine(ctx, "INFO", &jda_builtin_info);
    fn_obj->man_desc = g_msg[MSG_MANUAL_INFO];
    jda_add_object(ctx, fn_obj);
    fn_obj = jda_object_create_routine(ctx, "CLEAR", &jda_builtin_clear);
    fn_obj->man_desc = g_msg[MSG_MANUAL_CLEAR];
    jda_add_object(ctx, fn_obj);
    fn_obj = jda_object_create_routine(ctx, "DUMP", &jda_builtin_dump);
    fn_obj->man_desc = g_msg[MSG_MANUAL_DUMP];
    jda_add_object(ctx, fn_obj);
    fn_obj = jda_object_create_routine(ctx, "LOGOUT", &jda_builtin_quit);
    fn_obj->man_desc = g_msg[MSG_MANUAL_LOGOUT];
    jda_add_object(ctx, fn_obj);
    fn_obj = jda_object_create_routine(ctx, "GETMAIN", &jda_builtin_getmain);
    fn_obj->man_desc = g_msg[MSG_MANUAL_GETMAIN];
    jda_add_object(ctx, fn_obj);
    fn_obj = jda_object_create_routine(ctx, "RESIZEMAIN", &jda_builtin_resizemain);
    fn_obj->man_desc = g_msg[MSG_MANUAL_RESIZEMAIN];
    jda_add_object(ctx, fn_obj);
    fn_obj = jda_object_create_routine(ctx, "FREEMAIN", &jda_builtin_freemain);
    fn_obj->man_desc = g_msg[MSG_MANUAL_FREEMAIN];
    jda_add_object(ctx, fn_obj);
    fn_obj = jda_object_create_routine(ctx, "PUSH", &jda_builtin_push);
    fn_obj->man_desc = g_msg[MSG_MANUAL_PUSH];
    jda_add_object(ctx, fn_obj);
    fn_obj = jda_object_create_routine(ctx, "POP", &jda_builtin_pop);
    fn_obj->man_desc = g_msg[MSG_MANUAL_POP];
    jda_add_object(ctx, fn_obj);
    fn_obj = jda_object_create_routine(ctx, "DUP", &jda_builtin_dup);
    fn_obj->man_desc = g_msg[MSG_MANUAL_DUP];
    jda_add_object(ctx, fn_obj);
    fn_obj = jda_object_create_routine(ctx, "INPUT", &jda_builtin_getinput);
    fn_obj->man_desc = g_msg[MSG_MANUAL_INPUT];
    jda_add_object(ctx, fn_obj);
    fn_obj = jda_object_create_routine(ctx, "PRINT", &jda_builtin_print);
    fn_obj->man_desc = g_msg[MSG_MANUAL_PRINT];
    jda_add_object(ctx, fn_obj);
    fn_obj = jda_object_create_routine(ctx, "STRING", &jda_builtin_string);
    fn_obj->man_desc = g_msg[MSG_MANUAL_STRING];
    jda_add_object(ctx, fn_obj);
    fn_obj = jda_object_create_routine(ctx, "NUMBER", &jda_builtin_number);
    fn_obj->man_desc = g_msg[MSG_MANUAL_NUMBER];
    jda_add_object(ctx, fn_obj);
    fn_obj = jda_object_create_routine(ctx, "HELP", &jda_builtin_help);
    fn_obj->man_desc = g_msg[MSG_MANUAL_HELP];
    jda_add_object(ctx, fn_obj);
    fn_obj = jda_object_create_routine(ctx, "LIST", &jda_builtin_cat);
    fn_obj->man_desc = g_msg[MSG_MANUAL_LIST];
    jda_add_object(ctx, fn_obj);
    fn_obj = jda_object_create_routine(ctx, "MKCAT", &jda_builtin_mkcat);
    fn_obj->man_desc = g_msg[MSG_MANUAL_MKCAT];
    jda_add_object(ctx, fn_obj);
    fn_obj = jda_object_create_routine(ctx, "GET_USER", &jda_builtin_get_user);
    fn_obj->man_desc = g_msg[MSG_MANUAL_CURRID];
    jda_add_object(ctx, fn_obj);
    fn_obj = jda_object_create_routine(ctx, "GET_USERNAME", &jda_builtin_get_username);
    fn_obj->man_desc = g_msg[MSG_MANUAL_USERNAME];
    jda_add_object(ctx, fn_obj);
    fn_obj = jda_object_create_routine(ctx, "EXEC", &jda_builtin_exec);
    fn_obj->man_desc = g_msg[MSG_MANUAL_EXEC];
    jda_add_object(ctx, fn_obj);
    fn_obj = jda_object_create_routine(ctx, "OPENFILE", &jda_builtin_openfile);
    fn_obj->man_desc = g_msg[MSG_MANUAL_OPENFILE];
    jda_add_object(ctx, fn_obj);
    fn_obj = jda_object_create_routine(ctx, "NEWFILE", &jda_builtin_newfile);
    fn_obj->man_desc = g_msg[MSG_MANUAL_NEWFILE];
    jda_add_object(ctx, fn_obj);
    fn_obj = jda_object_create_routine(ctx, "PARPGM", &jda_builtin_parpgm);
    fn_obj->man_desc = g_msg[MSG_MANUAL_PGM];
    jda_add_object(ctx, fn_obj);
    fn_obj = jda_object_create_routine(ctx, "SEQPGM", &jda_builtin_seqpgm);
    fn_obj->man_desc = g_msg[MSG_MANUAL_SEQPGM];
    jda_add_object(ctx, fn_obj);
    fn_obj = jda_object_create_routine(ctx, "POPM", &jda_builtin_popm);
    fn_obj->man_desc = "Pops multiple objects from the stack";
    jda_add_object(ctx, fn_obj);
    fn_obj = jda_object_create_routine(ctx, "SQRT", &jda_builtin_sqrt);
    fn_obj->man_desc = g_msg[MSG_MANUAL_SQRT];
    jda_add_object(ctx, fn_obj);
    fn_obj = jda_object_create_routine(ctx, "FACTORIAL", &jda_builtin_factorial);
    fn_obj->man_desc = g_msg[MSG_MANUAL_FACTORIAL];
    jda_add_object(ctx, fn_obj);
    fn_obj = jda_object_create_routine(ctx, "SIN", &jda_builtin_sin);
    fn_obj->man_desc = "Sine of the given number";
    jda_add_object(ctx, fn_obj);
    fn_obj = jda_object_create_routine(ctx, "COS", &jda_builtin_cos);
    fn_obj->man_desc = "Cosine of the given number";
    jda_add_object(ctx, fn_obj);
    fn_obj = jda_object_create_routine(ctx, "TAN", &jda_builtin_tan);
    fn_obj->man_desc = "Tangent of the given number";
    jda_add_object(ctx, fn_obj);
    fn_obj = jda_object_create_routine(ctx, "SINH", &jda_builtin_sinh);
    fn_obj->man_desc = "Hyperbolic sine of the given number";
    jda_add_object(ctx, fn_obj);
    fn_obj = jda_object_create_routine(ctx, "COSH", &jda_builtin_cosh);
    fn_obj->man_desc = "Hyperbolic cosine of the given number";
    jda_add_object(ctx, fn_obj);
    fn_obj = jda_object_create_routine(ctx, "TANH", &jda_builtin_tanh);
    fn_obj->man_desc = "Hyperbolic tangent of the given number";
    jda_add_object(ctx, fn_obj);
    fn_obj = jda_object_create_routine(ctx, "CEULER", &jda_builtin_ceuler);
    fn_obj->man_desc = "Euler's Constant";
    jda_add_object(ctx, fn_obj);
    fn_obj = jda_object_create_routine(ctx, "CPI", &jda_builtin_cpi);
    fn_obj->man_desc = "Pi Constant";
    jda_add_object(ctx, fn_obj);
    fn_obj = jda_object_create_routine(ctx, "REAL", &jda_builtin_real);
    fn_obj->man_desc = "Obtain the real part of a number";
    jda_add_object(ctx, fn_obj);
    fn_obj = jda_object_create_routine(ctx, "IMGY", &jda_builtin_imgy);
    fn_obj->man_desc = "Obtain the imaginary part of a number";
    jda_add_object(ctx, fn_obj);
    return 0;
}
