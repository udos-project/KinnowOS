#ifndef JDA_CONTEXT_H
#define JDA_CONTEXT_H

#include <stddef.h>
#include <x3270.h>

struct jda_object;
struct jda_token;

struct jda_context {
    jda_object **objects;
    size_t n_objects;
    jda_object **stacks;
    size_t n_stacks;
    jda_token *tokens;
    size_t n_tokens;

    /* Used for parsing */
    /* Last token before performing a function jump */
    size_t fn_token_idx;

    /* Ease-of-use features */
    int fail_cnt;
    char *exec_buf;
    
    x3270_term term;
    char *term_buf;
    size_t n_term_buf;
};

int jda_context_init(jda_context& ctx);

#endif
