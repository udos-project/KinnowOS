#ifndef JDA_TOKEN_H
#define JDA_TOKEN_H

#include <stddef.h>
#include "number.hxx"

enum jda_token_type {
#if defined DEBUG
    JDA_TOK_INVALID = 0,
#endif
    JDA_TOK_ASSIGN,
    JDA_TOK_SUM,
    JDA_TOK_SUB,
    JDA_TOK_MUL,
    JDA_TOK_DIV,
    JDA_TOK_REM,
    JDA_TOK_EXPONENT,
    JDA_TOK_DOT,
    JDA_TOK_COMMA,
    JDA_TOK_LPAREN,
    JDA_TOK_RPAREN,
    JDA_TOK_LBRACE,
    JDA_TOK_RBRACE,
    JDA_TOK_IDENT,
    JDA_TOK_NUMBER,
    JDA_TOK_STRING,
    JDA_TOK_FUNCTION,
    JDA_TOK_COLON,
    JDA_TOK_SEMICOLON,
    JDA_TOK_DUMMY,
};

struct jda_token {
    enum jda_token_type type;
    char *data;
    size_t arg_count;
    jda_number_t numval;
};

struct jda_token_info {
    int precedence;
};

extern const char *g_token_type_names[32];

#include "context.hxx"
void jda_push_token(jda_context& ctx, const jda_token *tok);
void jda_dump_tokens(jda_context& ctx);

void jda_tokens_delete(jda_token *tokens, size_t n_tokens);

#endif
