#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "token.hxx"
#include "jda.hxx"
#include "globals.hxx"

constinit const char *g_token_type_names[32] = {
#if defined DEBUG
    "invalid",
#endif
    "=",
    "+",
    "-",
    "*",
    "/",
    "%",
    "^",
    ".",
    ",",
    "(",
    ")",
    "{",
    "}",
    "ident",
    "number",
    "string",
    "function",
    "colon",
    "semicolon",
    "dummy",
    nullptr,
};

void jda_push_token(jda_context& ctx, const struct jda_token *tok)
{
    assert(tok != nullptr);
    ctx.tokens = (jda_token *)realloc(ctx.tokens, (ctx.n_tokens + 1) * sizeof(jda_token));
    if(ctx.tokens == nullptr) {
        jda_printf(ctx, g_msg[MSG_OUT_OF_MEMORY]);
        return;
    }
    memcpy(&ctx.tokens[ctx.n_tokens++], tok, sizeof(struct jda_token));
    return;
}

void jda_dump_tokens(jda_context& ctx)
{
    size_t i;
    dprintf("*** BEGIN-Tokens\r\n");
    for(i = 0; i < ctx.n_tokens; i++) {
        const struct jda_token *tok = &ctx.tokens[i];
        if(tok->type == JDA_TOK_IDENT || tok->type == JDA_TOK_STRING || tok->type == JDA_TOK_FUNCTION) {
            dprintf("%i#token=%s,arg=%u,%i,data=%s\r\n", i, g_token_type_names[(tok->type > 32) ? 0 : tok->type], tok->arg_count, (int)tok->type, tok->data);
        } else if(tok->type == JDA_TOK_NUMBER) {
            dprintf("%i#token=%s,arg=%u,%i,real=%llf,img=%llf,pow=%llf\r\n", i, g_token_type_names[(tok->type > 32) ? 0 : tok->type], tok->arg_count, (int)tok->type, tok->numval.real_value, tok->numval.imaginary_value, tok->numval.power);
        } else {
            dprintf("%i#token=%s,arg=%u,%i\r\n", i, g_token_type_names[(tok->type > 32) ? 0 : tok->type], tok->arg_count, (int)tok->type);
        }
    }
    dprintf("*** END-Tokens\r\n");
}

void jda_tokens_delete(struct jda_token *tokens, size_t n_tokens)
{
    size_t i;
    for(i = 0; i < n_tokens; i++) {
        struct jda_token *tok = &tokens[i];
        if(tok->type == JDA_TOK_IDENT || tok->type == JDA_TOK_STRING || tok->type == JDA_TOK_FUNCTION) {
            assert(tok->data != nullptr);
            free(tok->data);
        }
    }
    return;
}
