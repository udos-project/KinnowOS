#ifndef JDA_H
#define JDA_H

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

/* Job Descriptor Algorithm */
#include "token.hxx"
#include "object.hxx"
#include "context.hxx"

int jda_puts(jda_context& ctx, const char *text);
int jda_vprintf(jda_context& ctx, const char *fmt, va_list args);
int jda_printf(jda_context& ctx, const char *fmt, ...);
int jda_report_error(jda_context& ctx, const char *fmt, ...);
int jda_get_input(jda_context& ctx, char *buf, size_t n);

int jda_lex_line(jda_context& ctx, const char *line);
int jda_lexer_to_rpn(jda_context& ctx);
int jda_parse(jda_context& ctx);
const char *jda_get_unitsize(size_t unit, size_t *disp_val);

int jda_builtin_getmain(jda_context& ctx);
int jda_builtin_resizemain(jda_context& ctx);
int jda_builtin_freemain(jda_context& ctx);
int jda_builtin_test(jda_context& ctx);
int jda_builtin_push(jda_context& ctx);
int jda_builtin_pop(jda_context& ctx);
int jda_builtin_dup(jda_context& ctx);
int jda_builtin_getinput(jda_context& ctx);
int jda_builtin_print(jda_context& ctx);
int jda_builtin_stackdump(jda_context& ctx);
int jda_builtin_info(jda_context& ctx);
int jda_builtin_clear(jda_context& ctx);
int jda_builtin_dump(jda_context& ctx);
int jda_builtin_graph(jda_context& ctx);
int jda_builtin_quit(jda_context& ctx);
int jda_builtin_string(jda_context& ctx);
int jda_builtin_number(jda_context& ctx);
int jda_builtin_help(jda_context& ctx);
int jda_builtin_cat(jda_context& ctx);
int jda_builtin_mkcat(jda_context& ctx);
int jda_builtin_get_user(jda_context& ctx);
int jda_builtin_get_username(jda_context& ctx);
int jda_builtin_exec(jda_context& ctx);
int jda_builtin_openfile(jda_context& ctx);
int jda_builtin_newfile(jda_context& ctx);
int jda_builtin_parpgm(jda_context& ctx);
int jda_builtin_seqpgm(jda_context& ctx);
int jda_builtin_popm(jda_context& ctx);
int jda_builtin_sqrt(jda_context& ctx);
int jda_builtin_factorial(jda_context& ctx);
int jda_builtin_sin(jda_context& ctx);
int jda_builtin_cos(jda_context& ctx);
int jda_builtin_tan(jda_context& ctx);
int jda_builtin_sinh(jda_context& ctx);
int jda_builtin_cosh(jda_context& ctx);
int jda_builtin_tanh(jda_context& ctx);
int jda_builtin_ceuler(jda_context& ctx);
int jda_builtin_cpi(jda_context& ctx);
int jda_builtin_real(jda_context& ctx);
int jda_builtin_imgy(jda_context& ctx);
void jda_term(void);

#endif
