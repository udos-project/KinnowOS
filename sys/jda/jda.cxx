#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <charset.h>
#include <assert.h>
#include <user.h>
#include <x3270.h>
#include <math.h>

#include "jda.hxx"
#include "globals.hxx"

int jda_puts(jda_context& ctx, const char *text)
{
    size_t offset = 0;
    dprintf("JDA_PRINTF=\"%s\",Y=%u", text, ctx.term.y);

    /* Set color to pink */
    ctx.term_buf[offset++] = X3270_ATTR(0);
    ctx.term_buf[offset++] = X3270_ORDER_START_FIELD_EXTENDED;
    ctx.term_buf[offset++] = 2;
    ctx.term_buf[offset++] = X3270_EXT_ORDER_COLOR;
    ctx.term_buf[offset++] = X3270_COLOR_CYAN;
    ctx.term_buf[offset++] = X3270_EXT_ORDER_HIGHLIGHT;
    ctx.term_buf[offset++] = X3270_HL_INTENSIFY;
    if(x3270_puts(&ctx.term, ctx.term_buf, &offset, ctx.n_term_buf, text) < 0) {
        return -1;
    }

    if(write(ctx.term.fd, ctx.term_buf, offset) < 0) {
        return -1;
    }
    return 0;
}

int jda_vprintf(jda_context& ctx, const char *fmt, va_list args)
{
    char tmpbuf[80];
    vsnprintf(tmpbuf, sizeof tmpbuf, fmt, args);
    jda_puts(ctx, tmpbuf);
    return 0;
}

int jda_printf(jda_context& ctx, const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    jda_vprintf(ctx, fmt, args);
    va_end(args);
    return 0;
}

int jda_report_error(jda_context& ctx, const char *fmt, ...)
{
    char tmpbuf[80];
    va_list args;

    strcpy(tmpbuf, "Error: ");
    strncat(tmpbuf, fmt, sizeof tmpbuf);
    strncat(tmpbuf, "!", sizeof tmpbuf);

    va_start(args, fmt);
    jda_vprintf(ctx, fmt, args);
    ctx.fail_cnt++;
    va_end(args);
    return 0;
}

int jda_get_input(jda_context& ctx, char *buf, size_t n)
{
    if(x3270_get_input(&ctx.term, buf, n) < 0) {
        jda_report_error(ctx, "Can't read console\r\n");
        return -1;
    }

    for(size_t i = 0; i < n; i++) {
        printf("%c(%x)", buf[i], (unsigned int)buf[i]);
    }

    ctx.term.x = 0;
    ctx.term.y++;

    /* Call the vertical paging handler if any */
    if(ctx.term.y >= ctx.term.rows - 1 && ctx.term.vert_paging != nullptr) {
        ctx.term.vert_paging(&ctx.term);
    }
    return 0;
}

int jda_lex_line(jda_context& ctx, const char *line)
{
    const char *read_ptr = line;

    assert(line != nullptr && read_ptr != nullptr);
    dprintf("JDA.LexLine");

    /* If the line starts with an asterisk, then it's a comment */
    if(read_ptr[0] == '*') {
        return 1;
    }

    /* Lexer */
    while(*read_ptr != '\0') {
        struct jda_token tok = {};
        dprintf("ReadPtr=%p,Chr=%x", read_ptr, (unsigned int)*read_ptr);
        switch(*read_ptr) {
        case '(':
            /* Identifier before ( parenthesis becomes a function */
            if(ctx.n_tokens && ctx.tokens[ctx.n_tokens - 1].type == JDA_TOK_IDENT) {
                ctx.tokens[ctx.n_tokens - 1].type = JDA_TOK_FUNCTION;
            }

            tok.type = JDA_TOK_LPAREN;
            jda_push_token(ctx, &tok);
            read_ptr++;
            break;
        case ')':
            tok.type = JDA_TOK_RPAREN;
            jda_push_token(ctx, &tok);
            read_ptr++;
            break;
        case '{':
            tok.type = JDA_TOK_LBRACE;
            jda_push_token(ctx, &tok);
            read_ptr++;
            break;
        case '}':
            tok.type = JDA_TOK_RBRACE;
            jda_push_token(ctx, &tok);
            read_ptr++;
            break;
        case '+':
            tok.type = JDA_TOK_SUM;
            jda_push_token(ctx, &tok);
            read_ptr++;
            break;
        case '-':
            tok.type = JDA_TOK_SUB;
            jda_push_token(ctx, &tok);
            read_ptr++;
            break;
        case '/':
            tok.type = JDA_TOK_DIV;
            jda_push_token(ctx, &tok);
            read_ptr++;
            break;
        case '%':
            tok.type = JDA_TOK_REM;
            jda_push_token(ctx, &tok);
            read_ptr++;
            break;
        case '*':
            tok.type = JDA_TOK_MUL;
            jda_push_token(ctx, &tok);
            read_ptr++;
            break;
        case '^':
            tok.type = JDA_TOK_EXPONENT;
            jda_push_token(ctx, &tok);
            read_ptr++;
            break;
        case '=':
            tok.type = JDA_TOK_ASSIGN;
            jda_push_token(ctx, &tok);
            read_ptr++;
            break;
        case '.':
            tok.type = JDA_TOK_DOT;
            jda_push_token(ctx, &tok);
            read_ptr++;
            break;
        case ',':
            tok.type = JDA_TOK_COMMA;
            jda_push_token(ctx, &tok);
            read_ptr++;
            break;
        case ':':
            tok.type = JDA_TOK_COLON;
            jda_push_token(ctx, &tok);
            read_ptr++;
            break;
        case ';':
            tok.type = JDA_TOK_SEMICOLON;
            jda_push_token(ctx, &tok);
            read_ptr++;
            break;
        case ' ':
        case '\t':
        case '\r':
        case '\n': {
            /* Skip identation */
            while(isspace(*read_ptr) || *read_ptr == '\r' || *read_ptr == '\n') {
                read_ptr++;
            }
        } break;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9': {
            const char *start_ptr = read_ptr;
            size_t len;
            char is_xd = 0, dec_cnt = 0;

            if(*read_ptr == '0') {
                read_ptr++;
                if(*read_ptr == 'x') {
                    is_xd = 1;
                    read_ptr++;
                } else {
                    read_ptr--;
                }
            }

            /* For hexadecimal numbers */
            if(is_xd) {
                while(isxdigit(*read_ptr)) {
                    read_ptr++;
                }
            } else {
                while(isdigit(*read_ptr)) {
                    read_ptr++;
                    if(*read_ptr == '.') {
                        /** @todo Support multiple decimal points */
                        /* More than 1 decimal specified */
                        if(dec_cnt) {
                            jda_report_error(ctx, "Multiple decimals are not supported yet\r\n");
                            goto end_error;
                        }
                        read_ptr++;
                        dec_cnt++;
                        continue;
                    }
                }
            }

            len = (uintptr_t)((ptrdiff_t)read_ptr - (ptrdiff_t)start_ptr);
            if(len == 0) {
                jda_report_error(ctx, "Zero-length number\r\n");
                goto end_error;
            }
            
            tok.type = JDA_TOK_NUMBER;

            /** @todo Parse the number more properly to support imaginaries */
            auto *data = (char *)malloc(len + 1);
            if(data == nullptr) {
                jda_report_error(ctx, g_msg[MSG_OUT_OF_MEMORY]);
                goto end_error;
            }
            strncpy(data, start_ptr, len);
            tok.numval.real_value = atof(data);
            tok.numval.imaginary_value = 0.f;
            tok.numval.power = 1.f;
            free(data);
            jda_push_token(ctx, &tok);
        } break;
        case '"': {
            const char *start_ptr = read_ptr + 1;
            char *esc_str;
            size_t len;
            
            read_ptr++;
            while(*read_ptr != '\0' && *read_ptr != '"') {
                read_ptr++;
                if(*read_ptr == '\\') {
                    read_ptr++;
                    if(*read_ptr == '"') {
                        read_ptr++;
                    }
                    continue;
                }
            }

            len = (uintptr_t)((ptrdiff_t)read_ptr - (ptrdiff_t)start_ptr);
            if(*read_ptr == '"') {
                read_ptr++;
            }
            if(len == 0) {
                jda_report_error(ctx, "Zero-length string\r\n");
                goto end_error;
            }
            
            tok.type = JDA_TOK_STRING;
            tok.data = (char *)malloc(len + 1);
            if(tok.data == nullptr) {
                jda_report_error(ctx, g_msg[MSG_OUT_OF_MEMORY]);
                goto end_error;
            }
            strncpy(tok.data, start_ptr, len);

            /* Escape the string obtained */
            esc_str = tok.data;
            while(*esc_str != '\0') {
                if(*esc_str == '\\') {
                    esc_str++;
                    switch(*esc_str) {
                    case 'n':
                        esc_str[-1] = '\n';
                        esc_str[0] = ' ';
                        break;
                    case 'r':
                        esc_str[-1] = '\r';
                        esc_str[0] = ' ';
                        break;
                    case 't':
                        esc_str[-1] = '\t';
                        esc_str[0] = ' ';
                        break;
                    default:
                        esc_str[-1] = *esc_str;
                        esc_str[0] = ' ';
                        break;
                    }
                } else {
                    esc_str++;
                }
            }

            jda_push_token(ctx, &tok);
        } break;
        default:
            if(*read_ptr == '_' || isalnum(*read_ptr)) {
                const char *start_ptr = read_ptr;
                size_t len;

                tok.type = JDA_TOK_IDENT;
                while(*read_ptr == '_' || isalnum(*read_ptr)) {
                    read_ptr++;
                }

                len = (uintptr_t)((ptrdiff_t)read_ptr - (ptrdiff_t)start_ptr);
                assert(len != 0);

                tok.data = (char *)malloc(len + 1);
                if(tok.data == nullptr) {
                    jda_report_error(ctx, g_msg[MSG_OUT_OF_MEMORY]);
                    goto end_error;
                }
                strncpy(tok.data, start_ptr, len);
                jda_push_token(ctx, &tok);
            } else {
                jda_report_error(ctx, "Unknown character %c(X'%X)\r\n", *read_ptr, (unsigned int)*read_ptr);
                goto end_error;
            }
            break;
        }
    }
    goto end;
end_error:
    /* Free all the tokens */
    jda_tokens_delete(ctx.tokens, ctx.n_tokens);
    return -1;
end:
#if defined DEBUG
    jda_dump_tokens(ctx);
#endif
    return 0;
}

int jda_lexer_to_rpn(jda_context& ctx)
{
    jda_token *outputs = nullptr;
    size_t n_outputs = 0;
    jda_token *operators = nullptr;
    size_t n_operators = 0;
    size_t *arg_counts = nullptr;
    size_t n_arg_counts = 0;
    size_t i;

    int r = 0;

    dprintf("JDA.LexerToRpn");
    if(ctx.n_tokens == 0) {
        jda_report_error(ctx, "No tokens\r\n");
        goto end_error;
    }

    /* WORKAROUND: Memory is overlapped by the allocator which causes a lot of issues */
    for(size_t i = 0; i < ctx.n_tokens; i++) {
        const auto *tok = &ctx.tokens[i];
#if defined DEBUG
        if(tok->type == JDA_TOK_INVALID) {
            dprintf("Token#%i invalid", i);
            goto end_error;
        }
#endif

        if(tok->type == JDA_TOK_NUMBER || tok->type == JDA_TOK_IDENT || tok->type == JDA_TOK_ASSIGN || tok->type == JDA_TOK_STRING) {
            if(n_arg_counts && arg_counts[n_arg_counts - 1] == 0) {
                arg_counts[n_arg_counts - 1] = 1;
            }
            outputs = (jda_token *)realloc(outputs, (n_outputs + 1) * sizeof(struct jda_token));
            memcpy(&outputs[n_outputs++], tok, sizeof(struct jda_token));
        } else if(tok->type == JDA_TOK_DOT || tok->type == JDA_TOK_COLON) {
            outputs = (jda_token *)realloc(outputs, (n_outputs + 1) * sizeof(struct jda_token));
            memcpy(&outputs[n_outputs++], tok, sizeof(struct jda_token));  
        } else if(tok->type == JDA_TOK_FUNCTION) {
            if(n_arg_counts && arg_counts[n_arg_counts - 1] == 0) {
                arg_counts[n_arg_counts - 1] = 1;
            }
            operators = (jda_token *)realloc(operators, (n_operators + 1) * sizeof(struct jda_token));
            memcpy(&operators[n_operators++], tok, sizeof(struct jda_token));
        } else if(tok->type == JDA_TOK_COMMA) {
            if(n_operators == 0) {
                jda_report_error(ctx, "No operators before comma\r\n");
                goto end_error;
            } else if(n_arg_counts == 0) {
                jda_report_error(ctx, "Comma is before any parameters\r\n");
                goto end_error;
            }

            /* Increment argument count */
            arg_counts[n_arg_counts - 1]++;

            while(operators[n_operators - 1].type != JDA_TOK_LPAREN) {
                /* Pop from operator stack and add to output stack */
                outputs = (jda_token *)realloc(outputs, (n_outputs + 1) * sizeof(struct jda_token));
                memcpy(&outputs[n_outputs++], &operators[n_operators - 1], sizeof(struct jda_token));
                n_operators--;
                if(n_operators == 0) {
                    jda_report_error(ctx, "Mismatched parenthesis on argument list\r\n");
                    goto end_error;
                }
            }
        } else if(tok->type == JDA_TOK_LPAREN) {
            arg_counts = (size_t *)realloc(arg_counts, (n_arg_counts + 1) * sizeof(size_t));
            arg_counts[n_arg_counts++] = 0;

            operators = (jda_token *)realloc(operators, (n_operators + 1) * sizeof(struct jda_token));
            memcpy(&operators[n_operators++], tok, sizeof(struct jda_token));
        } else if(tok->type == JDA_TOK_RPAREN) {
            /* Pop tokens onto the output queue until a left-parenthesis is found (must match!) */
            while(operators[n_operators - 1].type != JDA_TOK_LPAREN) {
                outputs = (jda_token *)realloc(outputs, (n_outputs + 1) * sizeof(struct jda_token));
                memcpy(&outputs[n_outputs++], &operators[n_operators - 1], sizeof(struct jda_token));
                n_operators--;
                if(n_operators == 0) {
                    jda_report_error(ctx, "Mismatched left-parenthesis\r\n");
                    goto end_error;
                }
            }

            /* Discard the left parenthesis */
            n_operators--;
            if(n_operators && operators[n_operators - 1].type == JDA_TOK_FUNCTION) {
                /* Take the function token from the operator stack and place
                 * it onto the output one*/
                outputs = (jda_token *)realloc(outputs, (n_outputs + 1) * sizeof(struct jda_token));
                operators[n_operators - 1].arg_count = arg_counts[n_arg_counts - 1];
                n_arg_counts--;

                memcpy(&outputs[n_outputs++], &operators[n_operators - 1], sizeof(struct jda_token));
                n_operators--;
            }
        } else {
            operators = (jda_token *)realloc(operators, (n_operators + 1) * sizeof(struct jda_token));
            memcpy(&operators[n_operators++], tok, sizeof(struct jda_token));
        }
#if defined DEBUG
        for(size_t j = 0; j < ctx.n_tokens; j++) {
            if(ctx.tokens[j].type == JDA_TOK_INVALID) {
                dprintf("Tokens#%i is invalid", (int)j);
                goto end_error;
            }
        }

        for(size_t j = 0; j < n_operators; j++) {
            if(operators[j].type == JDA_TOK_INVALID) {
                dprintf("Operator#%i is invalid", (int)j);
                goto end_error;
            }
        }

        for(size_t j = 0; j < n_outputs; j++) {
            if(outputs[j].type == JDA_TOK_INVALID) {
                dprintf("Outputs#%i is invalid", (int)j);
                goto end_error;
            }
        }
#endif
    }

    while(n_operators != 0) {
        if(operators[n_operators - 1].type == JDA_TOK_LPAREN) {
            jda_report_error(ctx, "Lone left-parenthesis\r\n");
            goto end_error;
        }

        outputs = (jda_token *)realloc(outputs, (n_outputs + 1) * sizeof(struct jda_token));
        memcpy(&outputs[n_outputs++], &operators[n_operators - 1], sizeof(struct jda_token));
        n_operators--;
    }

    ctx.fail_cnt = 0;
    goto real_end;
end_error:
    ctx.fail_cnt++;
    r = -1;
real_end:
    /* Free the rest of the stuff */
    if(operators != nullptr) {
        /** @todo Properly duplicate data elements on tokens */
        /*jda_tokens_delete(operators, n_operators);*/
        free(operators);
    }
    if(arg_counts != nullptr) {
        free(arg_counts);
    }

#if defined DEBUG
    jda_dump_tokens(ctx);
#endif
    /* Swap output with the context's tokens */
    if(ctx.tokens != nullptr) {
        ctx.tokens = nullptr;
        ctx.n_tokens = 0;
        /** @todo Properly duplicate data elements on tokens */
        /*jda_tokens_delete(ctx.tokens, ctx.n_tokens);*/
        free(ctx.tokens);
    }

    if(outputs != nullptr) {
        ctx.tokens = outputs;
        ctx.n_tokens = n_outputs;
    } else {
        jda_report_error(ctx, "RPN yielded no tokens\r\n");
        r = -1;
    }
#if defined DEBUG
    jda_dump_tokens(ctx);
#endif
    return r;
}

int jda_parse(jda_context& ctx)
{
    size_t i;
    dprintf("JDA.Parse");
    if(ctx.n_tokens == 0) {
        jda_report_error(ctx, "No tokens to read\r\n");
        goto end_error;
    }
    assert(ctx.tokens != nullptr);
#if defined DEBUG
    jda_dump_tokens(ctx);
#endif

    dprintf("Evaluating tokens (step 1/2)");
    i = 0;
    while(1) {
        if(i >= ctx.n_tokens) {
            break;
        }

        switch(ctx.tokens[i].type) {
        case JDA_TOK_IDENT: {
            struct jda_object *obj;
            /* Assigment after identifier means no substitution */
            if(i + 1 < ctx.n_tokens && ctx.tokens[i + 1].type == JDA_TOK_ASSIGN) {
                i += 2;
                break;
            }

            /* Perform substitution */
            obj = jda_get_object(ctx, nullptr, ctx.tokens[i].data);
            if(obj == nullptr) {
                jda_report_error(ctx, "No variable named %s\r\n", ctx.tokens[i].data);
                goto end_error;
            }

            if(obj->type == JDA_OBJ_ROUTINE) {
                obj->func(ctx);
            }

            if(jda_object_to_token(ctx, obj, &ctx.tokens[i]) < 0) {
                goto end_error;
            }
            i++;
        } break;
        case JDA_TOK_FUNCTION: {
            struct jda_object *fobj;
            int n_ret = 0;
            size_t n_arg = 0;

            fobj = jda_get_object(ctx, nullptr, ctx.tokens[i].data);
            if(fobj == nullptr) {
                jda_report_error(ctx, "No function %s exists\r\n", ctx.tokens[i].data);
                goto end_error;
            }

            /* Convert tokens before this function into objects on the stack */
            if(ctx.tokens[i].arg_count > i) {
                jda_report_error(ctx, "Expected more arguments for %s\r\n", ctx.tokens[i].data);
                goto end_error;
            }

            for(size_t j = i - ctx.tokens[i].arg_count; j < i; j++) {
                struct jda_object *aobj = nullptr;
                if(ctx.tokens[j].type == JDA_TOK_NUMBER) {
                    aobj = jda_object_create_number(ctx, "__tmp", ctx.tokens[j].numval);
                } else if(ctx.tokens[j].type == JDA_TOK_STRING) {
                    assert(ctx.tokens[j].data != nullptr);
                    aobj = jda_object_create_string(ctx, "__tmp", ctx.tokens[j].data);
                    free(ctx.tokens[j].data);
                } else {
                    jda_report_error(ctx, "Can't convert '%s' to an object\r\n", g_token_type_names[ctx.tokens[j].type]);
                    goto end_error;
                }

                if(aobj != nullptr) {
                    jda_stack_push_object(ctx, aobj);
                }
            }

            n_ret = (size_t)fobj->func(ctx);
            if(n_ret < 0) {
                jda_report_error(ctx, g_msg[MSG_FAILURE_ON_CALL], ctx.tokens[i].data);
                goto end_error;
            } else if(n_ret > 1) {
                jda_report_error(ctx, "Returning more than 1 object is not supported yet\r\n");
                goto end_error;
            }

            /** @todo free the datan used by the removed tokens */
#if defined DEBUG
            ctx.tokens[i].type = JDA_TOK_INVALID;
#endif
            n_arg = ctx.tokens[i].arg_count;

            /* Remove the argument tokens along with the functions itself */
            /** @todo free the data used by the removed tokens */
            memmove(&ctx.tokens[i - n_arg], &ctx.tokens[i], sizeof(struct jda_token) * (ctx.n_tokens - i));
            ctx.n_tokens -= n_arg;
            i -= n_arg;

            /* Take the object at the top of the stack as the return value */
            if(ctx.n_stacks) {
                struct jda_object *obj = jda_stack_pop_object(ctx);
                if(jda_object_to_token(ctx, obj, &ctx.tokens[i]) < 0) {
                    goto end_error;
                }
            }
            i++;
        } break;
        case JDA_TOK_SUM:
        case JDA_TOK_SUB:
        case JDA_TOK_MUL:
        case JDA_TOK_DIV:
        case JDA_TOK_REM:
        case JDA_TOK_EXPONENT: {
            jda_number_t lhs_val, rhs_val, end_val;

            if(i < 2) {
                jda_report_error(ctx, "Expected 2 operands for \"%s\"\r\n", g_token_type_names[ctx.tokens[i].type]);
                goto end_error;
            }
            
            if(ctx.tokens[i - 2].type != JDA_TOK_NUMBER) {
                jda_report_error(ctx, g_msg[MSG_NUMBER_EXPECTED]);
                goto end_error;
            }
            if(ctx.tokens[i - 1].type != JDA_TOK_NUMBER) {
                jda_report_error(ctx, g_msg[MSG_NUMBER_EXPECTED]);
                goto end_error;
            }

            /* Convert to a number & drop */
            lhs_val = ctx.tokens[i - 2].numval;
            rhs_val = ctx.tokens[i - 1].numval;

            end_val.real_value = 0.f;
            end_val.imaginary_value = 0.f;
            end_val.power = 1.f;

            switch(ctx.tokens[i].type) {
            case JDA_TOK_SUM:
                end_val.real_value = lhs_val.real_value + rhs_val.real_value;
                break;
            case JDA_TOK_SUB:
                end_val.real_value = lhs_val.real_value - rhs_val.real_value;
                break;
            case JDA_TOK_MUL:
                end_val.real_value = lhs_val.real_value * rhs_val.real_value;
                break;
            case JDA_TOK_DIV:
                if(lhs_val.real_value == 0 || rhs_val.real_value == 0) {
                    end_val.real_value = 0.f;
                } else {
                    end_val.real_value = lhs_val.real_value / rhs_val.real_value;
                }
                break;
            case JDA_TOK_REM:
                if(lhs_val.real_value == 0 || rhs_val.real_value == 0) {
                    end_val.real_value = 0.f;
                } else {
                    end_val.real_value = fmodl(lhs_val.real_value, rhs_val.real_value);
                }
                break;
            case JDA_TOK_EXPONENT:
                end_val.real_value = powl(lhs_val.real_value, rhs_val.real_value);
                break;
            default:
                assert(0);
                goto end_error;
            }
            
            /** @todo free the data used by the removed tokens */
            memmove(&ctx.tokens[i - 2], &ctx.tokens[i], sizeof(struct jda_token) * (ctx.n_tokens - i));
            ctx.n_tokens -= 2;
            i -= 2;

            ctx.tokens[i].type = JDA_TOK_NUMBER;
            ctx.tokens[i].numval = end_val;
        } break;
        default:
            i++;
            break;
        }
#if defined DEBUG
        dprintf("JDA.Parse.NextToken=");
        jda_dump_tokens(ctx);
#endif
    }

    /* By this point the only remaining tokens SHOULD be tokens that are values, that is
     * the tokens remaining are tokens that can be assigned to a variable
     * in even simpler words: goodbye function calls
     */
    dprintf("Perform assignments (step 2/2)");
    i = 0;
    while(1) {
        if(i >= ctx.n_tokens) {
            break;
        }

        switch(ctx.tokens[i].type) {
#if defined DEBUG
        case JDA_TOK_INVALID:
            jda_report_error(ctx, "Invalid token #%i\r\n", i);
            goto end_error;
#endif
        case JDA_TOK_ASSIGN: {
            /* Assignment is composed as follows:
             * [variable name] [=] [optional value (used only if stack is empty)] */
            jda_object *obj = nullptr, *old_obj;
            if(i == 0 || ctx.tokens[i - 1].type != JDA_TOK_IDENT) {
                jda_report_error(ctx, "Assignment expects an identifer at the left\r\n");
                goto end_error;
            }

            /* First try to pop from the stack */
            if(ctx.n_stacks) {
                obj = jda_stack_pop_object(ctx);
            }
            
            if(obj == nullptr) {
                // If that didn't work out then take a token from the stack, which will be the
                // token holding the value to be given to this variable
                size_t j = i + 1;
                if(j < ctx.n_tokens) {
                    if(ctx.tokens[j].type == JDA_TOK_NUMBER) {
                        obj = jda_object_create_number(ctx, "__tmp", ctx.tokens[j].numval);
                    } else if(ctx.tokens[j].type == JDA_TOK_STRING) {
                        assert(ctx.tokens[j].data != nullptr);
                        obj = jda_object_create_string(ctx, "__tmp", ctx.tokens[j].data);
                        free(ctx.tokens[j].data);
                    } else {
                        jda_report_error(ctx, "Can't convert '%s' to an object\r\n", g_token_type_names[ctx.tokens[j].type]);
                        goto end_error;
                    }

                    if(obj == nullptr) {
                        jda_report_error(ctx, "Can't create object\r\n");
                        goto end_error;
                    }

                    /** @todo Properly delete this token */
                    memmove(&ctx.tokens[j], &ctx.tokens[j + 1], sizeof(struct jda_token) * (ctx.n_tokens - (j + 1)));
                    ctx.n_tokens--;
                }

                if(obj == nullptr) {
                    jda_report_error(ctx, "Assignment requires storeable\r\n");
                    goto end_error;
                }
            }

            /* Override old object (if any) */
            dprintf("Old obj %s\r\n", ctx.tokens[i - 1].data);
            old_obj = jda_get_object(ctx, nullptr, ctx.tokens[i - 1].data);
            if(old_obj != nullptr) {
                /** @todo Delete the object's previous data (such as grouped children) */
                /* Override value of the object */
                memcpy(old_obj, obj, sizeof(struct jda_object));
            }

            /* Rename object to the given identifier before the assignment operator */
            obj->name = (char *)realloc(obj->name, strlen(ctx.tokens[i - 1].data) + 1);
            if(obj->name == nullptr) {
                jda_report_error(ctx, g_msg[MSG_OUT_OF_MEMORY]);
                goto end_error;
            }
            strcpy(obj->name, ctx.tokens[i - 1].data);
            jda_add_object(ctx, obj);

            i++;
        } break;
        default:
            i++;
            break;
        }
#if defined DEBUG
        jda_dump_tokens(ctx);
#endif
    }
    
    ctx.fail_cnt = 0;
    goto real_end;
end_error:
    ctx.fail_cnt++;
real_end:
#if defined DEBUG
    jda_dump_tokens(ctx);
#endif
    for(size_t j = 0; j < ctx.n_stacks; j++) {
        jda_object_delete(ctx.stacks[j]);
    }
    ctx.n_stacks = 0;

    if(ctx.tokens != nullptr) {
        jda_tokens_delete(ctx.tokens, ctx.n_tokens);
        free(ctx.tokens);
    }

    ctx.tokens = nullptr;
    ctx.n_tokens = 0;

    /* WORKAROUND: For recursive execution we will reuse the same context, however to achieve this
     * we will have to run any execution buffer AFTER we finish processing the callee */
    if(ctx.exec_buf != nullptr) {
        char *p = ctx.exec_buf, *q = nullptr;

        q = strchr(p, '\x0D');
        while(q != nullptr) {
            int r;

            *q = '\0';
            q++;

            dprintf("JDA.Exec \"%s\"", p);
            r = jda_lex_line(ctx, p);
            if(r > 0) {
                goto next_line;
            } else if(r < 0) {
                break;
            }
            r = jda_lexer_to_rpn(ctx);
            if(r < 0) {
                break;
            }
            r = jda_parse(ctx);
            if(r < 0) {
                break;
            }
        
        next_line:
            p = q;
            q = strchr(p, '\x0D');
        }

        free(ctx.exec_buf);
        ctx.exec_buf = nullptr;
    }
    return 0;
}

const char *jda_get_unitsize(size_t unit, size_t *disp_val)
{
    const char *units[] = {
        "", "K", "M", "G", "T", "E"
    };
    size_t i = 0;

    while(unit != 0 && unit > 1000) {
        if(i >= sizeof(units) / sizeof(units[0])) {
            i--;
            break;
        }
        unit /= 1000;
        i++;
    }
    *disp_val = unit;
    return units[i];
}

int jda_builtin_getmain(jda_context& ctx)
{
    jda_object *robj;
    size_t size;
    void *p;

    auto *obj = jda_stack_pop_object(ctx);
    if(obj == nullptr) {
        jda_report_error(ctx, "No size of allocation specified\r\n");
        return 0;
    }
    size = (size_t)jda_object_to_integer(ctx, obj);
    jda_object_delete(obj);

    p = calloc(1, size);
    if(p == nullptr) {
        jda_report_error(ctx, g_msg[MSG_OUT_OF_MEMORY]);
        return 0;
    }
    jda_printf(ctx, "Allocated %u bytes at %p\r\n", size, p);

    robj = jda_object_create_pointer(ctx, "__tmp", p);
    jda_stack_push_object(ctx, robj);
    return 1;
}

int jda_builtin_resizemain(jda_context& ctx)
{
    jda_object *obj, *robj;
    size_t size;
    void *p;

    obj = jda_stack_pop_object(ctx);
    if(obj == nullptr) {
        jda_report_error(ctx, "No size specified\r\n");
        return 0;
    }
    size = (size_t)jda_object_to_integer(ctx, obj);
    jda_object_delete(obj);

    obj = jda_stack_pop_object(ctx);
    if(obj == nullptr) {
        jda_report_error(ctx, g_msg[MSG_POINTER_EXPECTED]);
        return 0;
    } else if(obj->type != JDA_OBJ_RAWPTR) {
        jda_report_error(ctx, g_msg[MSG_POINTER_EXPECTED]);
        jda_object_delete(obj);
        return 0;
    }
    jda_object_delete(obj);

    p = calloc(1, size);
    if(p == nullptr) {
        jda_report_error(ctx, g_msg[MSG_OUT_OF_MEMORY]);
        return 0;
    }
    jda_printf(ctx, "Allocated %u bytes at %p\r\n", size, p);

    robj = jda_object_create_pointer(ctx, "__tmp", p);
    jda_stack_push_object(ctx, robj);
    return 1;
}

int jda_builtin_freemain(jda_context& ctx)
{
    auto *obj = jda_stack_pop_object(ctx);
    if(obj == nullptr) {
        jda_report_error(ctx, g_msg[MSG_ARGUMENT_EXPECTED]);
        return 0;
    } else if(obj->type != JDA_OBJ_RAWPTR) {
        jda_report_error(ctx, g_msg[MSG_POINTER_EXPECTED]);
        jda_object_delete(obj);
        return 0;
    }
    auto *p = jda_object_to_rawptr(ctx, obj);
    jda_object_delete(obj);

    free(p);
    jda_printf(ctx, "freed %p\r\n", p);
    return 0;
}

int jda_builtin_test(jda_context& ctx)
{
    jda_printf(ctx, "Hello, welcome to JDA! :)\r\n");
    return 0;
}

/**
 * @brief Dummy function, passes it's arguments
 * 
 * @param ctx 
 * @return int
 */
int jda_builtin_push(jda_context& ctx)
{
    return 0;
}

/**
 * @brief Pops an element from the object stack
 * 
 * @param ctx 
 * @return int 
 */
int jda_builtin_pop(jda_context& ctx)
{
    auto *obj = jda_stack_pop_object(ctx);
    if(obj == nullptr) {
        return 0;
    }
    jda_object_delete(obj);
    return 0;
}

int jda_builtin_dup(jda_context& ctx)
{
    jda_object *robj;
    auto *obj = jda_stack_get_object(ctx, 0);
    if(obj == nullptr) {
        return 0;
    }

    if(obj->type == JDA_OBJ_NUMBER) {
        robj = jda_object_create_number(ctx, obj->name, obj->numval);
    } else if(obj->type == JDA_OBJ_STRING) {
        robj = jda_object_create_string(ctx, obj->name, (const char *)obj->data);
    } else if(obj->type == JDA_OBJ_RAWPTR) {
        robj = jda_object_create_pointer(ctx, obj->name, obj->data);
    } else if(obj->type == JDA_OBJ_ROUTINE) {
        robj = jda_object_create_routine(ctx, obj->name, (int (*)(jda_context& ctx))obj->func);
    } else {
        jda_report_error(ctx, "Can't dup said object\r\n");
        return 0;
    }
    jda_stack_push_object(ctx, robj);
    return 1;
}

int jda_builtin_getinput(jda_context& ctx)
{
    struct jda_object *robj;
    char tmpbuf[25];
    int r;

    memset(tmpbuf, 0, sizeof tmpbuf);
    if(jda_get_input(ctx, tmpbuf, sizeof tmpbuf - 1) < 0) {
        return 0;
    }

    robj = jda_object_create_string(ctx, "__tmp", tmpbuf);
    jda_stack_push_object(ctx, robj);
    return 1;
}

int jda_builtin_print(jda_context& ctx)
{
    char endbuf[80];

    endbuf[0] = '\0';
    for(size_t i = 0; i < ctx.n_stacks; i++) {
        const struct jda_object *obj = ctx.stacks[i];
        const char *str = jda_object_to_string(ctx, obj);
        strncat(endbuf, str, sizeof(endbuf));
    }
    jda_printf(ctx, "%s\r\n", endbuf);
    return 0;
}

static void jda_object_dump(jda_context& ctx, struct jda_object *obj)
{
    size_t i;

    switch(obj->type) {
    case JDA_OBJ_GROUP:
        jda_printf(ctx, "%s<group> = {\r\n", obj->name);
        for(i = 0; i < obj->n_objects; i++) {
            jda_object_dump(ctx, obj->objects[i]);
        }
        jda_printf(ctx, "}\r\n");
        break;
    case JDA_OBJ_ROUTINE:
        jda_printf(ctx, "%s<routine> = %p\r\n", obj->name, obj->func);
        break;
    case JDA_OBJ_NUMBER:
        jda_printf(ctx, "%s<number> = %i\r\n", obj->name, (int)obj->numval.real_value);
        break;
    case JDA_OBJ_RAWPTR:
        jda_printf(ctx, "%s<rawptr> = %s\r\n", obj->name, obj->data);
        break;
    case JDA_OBJ_STRING:
        jda_printf(ctx, "%s<string> = \"%s\"\r\n", obj->name, obj->data);
        break;
    default:
        jda_printf(ctx, "%s<unknown>;\r\n", obj->name);
        break;
    }
}

int jda_builtin_stackdump(jda_context& ctx)
{
    for(size_t i = 0; i < ctx.n_stacks; i++) {
        jda_object_dump(ctx, ctx.stacks[i]);
    }
    return 0;
}

#include <job.h>
int jda_builtin_info(jda_context& ctx)
{
    struct job_stats stats;
    const char *free_prefix, *used_prefix;
    size_t real_free_size, real_used_size;

    job_get_stats(&stats);
    free_prefix = jda_get_unitsize(stats.free_size, &real_free_size);
    used_prefix = jda_get_unitsize(stats.used_size, &real_used_size);
    jda_printf(ctx, "JDA on UDOS. %u%sB (%u%sB used) of storage used across %u regions\r\n", real_free_size, free_prefix, real_used_size, used_prefix, stats.n_regions);
    /* jda_printf(ctx, "User: %s\r\n", usersys::user_get_by_id(usersys::user_get_current())->name); */
    return 0;
}

int jda_builtin_clear(jda_context& ctx)
{
    x3270_clear_screen(&ctx.term);
    ctx.term.x = 0;
    ctx.term.y = 0;
    return 0;
}

int jda_builtin_dump(jda_context& ctx)
{
    jda_builtin_info(ctx);

    // Dump all the root objects
    jda_printf(ctx, "<root> = {\r\n");
    for(size_t i = 0; i < ctx.n_objects; i++) {
        jda_object_dump(ctx, ctx.objects[i]);
    }
    jda_printf(ctx, "}\r\n");

    jda_printf(ctx, "<stack> = {\r\n");
    jda_builtin_stackdump(ctx);
    jda_printf(ctx, "}\r\n");
    return 0;
}

int jda_builtin_quit(jda_context& ctx)
{
#if 0
    jda_printf(ctx, "Taking down the CPU#%u\r\n", (size_t)s390_intrin::cpuid());
    s390_intrin::signal(s390_intrin::cpuid(), S390_SIGP_STOP);
    jda_printf(ctx, "CPU should be down by now\r\n");
#endif
    return 0;
}

int jda_builtin_string(jda_context& ctx)
{
    jda_object *robj;
    auto *obj = jda_stack_pop_object(ctx);
    if(obj == nullptr) {
        robj = jda_object_create_string(ctx, "__tmp", "");
        jda_stack_push_object(ctx, robj);
        return 1;
    }
    robj = jda_object_create_string(ctx, obj->name, jda_object_to_string(ctx, obj));
    jda_stack_push_object(ctx, robj);
    jda_object_delete(obj);
    return 0;
}

int jda_builtin_number(jda_context& ctx)
{
    jda_object *obj, *robj;
    obj = jda_stack_pop_object(ctx);
    if(obj == nullptr) {
        robj = jda_object_create_real_number(ctx, "__tmp", 0.f);
        jda_stack_push_object(ctx, robj);
        return 1;
    }
    robj = jda_object_create_number(ctx, obj->name, jda_object_to_number(ctx, obj));
    jda_stack_push_object(ctx, robj);
    jda_object_delete(obj);
    return 1;
}

int jda_builtin_help(jda_context& ctx)
{
    for(size_t i = 0; i < ctx.n_stacks; i++) {
        if(ctx.stacks[i]->man_desc != nullptr) {
            jda_printf(ctx, "%s - %s\r\n", ctx.stacks[i]->name, ctx.stacks[i]->man_desc);
        }
    }

    for(size_t i = 0; i < ctx.n_objects; i++) {
        if(ctx.objects[i]->man_desc != nullptr) {
            jda_printf(ctx, "%s - %s\r\n", ctx.objects[i]->name, ctx.objects[i]->man_desc);
        }
    }
    return 0;
}

int jda_builtin_cat(jda_context& ctx)
{
#if 0
    auto *node = virtual_disk::resolve_path("\\");
    if(node != nullptr) {
        jda_dir_print(ctx, node, 0);
    }
#endif
    return 0;
}

int jda_builtin_mkcat(jda_context& ctx)
{
    return 0;
#if 0
    virtual_disk::node *node;
    jda_object *obj;
    obj = jda_stack_pop_object(ctx);
    if(obj == nullptr) {
        jda_report_error(ctx, g_msg[MSG_ARGUMENT_EXPECTED]);
        return 0;
    }
    node = virtual_disk::node_create("\\", jda_object_to_string(ctx, obj));
    dprintf("new node: %s created", node->name);
    jda_object_delete(obj);
    return 0;
#endif
}

int jda_builtin_get_user(jda_context& ctx)
{
    return 0;
#if 0
    struct jda_object *robj;
    robj = jda_object_create_integer(ctx, "__tmp", usersys::user_get_current());
    jda_stack_push_object(ctx, robj);
    return 1;
#endif
}

int jda_builtin_get_username(jda_context& ctx)
{
    return 0;
#if 0
    struct jda_object *obj, *robj;
    obj = jda_stack_pop_object(ctx);
    if(obj == nullptr) {
        jda_report_error(ctx, g_msg[MSG_ARGUMENT_EXPECTED]);
        return 0;
    }

    if(obj->numval >= (int)(*usersys::users_limit())) {
        jda_report_error(ctx, "%i is an invalid user\r\n", obj->numval);
        return 0;
    }

    robj = jda_object_create_string(ctx, "__tmp", usersys::user_get_by_id((usersys::user_t)obj->numval)->name);
    jda_object_delete(obj);
    jda_stack_push_object(ctx, robj);
    return 1;
#endif
}

/**
 * @brief Executes a JDA script (not to be confused with PGM which executes programs)
 * the name is given from the stack.
 * 
 * @param ctx 
 * @return int 
 */
int jda_builtin_exec(jda_context& ctx)
{
    int fd = -1;
    ssize_t r;
    size_t size;
    char *buf = nullptr;

    auto *obj = jda_stack_pop_object(ctx);
    if(obj == nullptr) {
        jda_report_error(ctx, g_msg[MSG_DATASET_EXPECTED]);
        goto end_error;
    }

    fd = jda_object_to_integer(ctx, obj);
    jda_object_delete(obj);
    if(fd >= FOPEN_MAX || fd < 0) {
        jda_report_error(ctx, g_msg[MSG_INVALID_FD], fd);
        goto end_error;
    }
    dprintf("JDA FD=%i,\r\n", fd);

    size = 2048;
    buf = (char *)malloc(size);
    if(buf == nullptr) {
        jda_report_error(ctx, g_msg[MSG_OUT_OF_MEMORY]);
        goto end_error;
    }
    memset(buf, 0, size);

    r = read(fd, buf, size);
    if(r < 0) {
        jda_report_error(ctx, "Error %i reading dataset FD(%i)\r\n", r, fd);
        goto end_error;
    }
    dprintf("BufferSize=%u\r\n", size);

    buf = (char *)realloc(buf, size + 1);
    if(buf == nullptr) {
        jda_report_error(ctx, g_msg[MSG_OUT_OF_MEMORY]);
        goto end_error;
    }
    buf[size] = '\0';

    {
        char *p = buf, *q = nullptr;

        /* Convert the entire buffer into readable native EBCDIC */
        charset_ascii_to_native(buf, size);
        dprintf("Executing a JDA file");
        /* Convert all \x1A into \n */
        q = strchr(p, '\x1A');
        while(q != nullptr) {
            *q = '\n';
            q++;
            p = q;
            q = strchr(p, '\x1A');
        }
        ctx.exec_buf = buf;
        return 0;
    }
end_error:
    if(buf != nullptr) {
        free(buf);
    }
    return -1;
}

/**
 * @brief Opens a file and pushes the resulting fd to the stack.
 * 
 * @param ctx 
 * @return int 
 */
int jda_builtin_openfile(jda_context& ctx)
{
    char *filename = nullptr;

    auto *obj = jda_stack_pop_object(ctx);
    if(obj == nullptr) {
        jda_report_error(ctx, g_msg[MSG_DATASET_EXPECTED]);
        return 0;
    }

    /* Obtain the filename of the file and store it onto one of our fid table entry */ {
        const char *obj_str;
        obj_str = jda_object_to_string(ctx, obj);
        if(obj_str == nullptr) {
            jda_object_delete(obj);
            jda_report_error(ctx, g_msg[MSG_DATASET_EXPECTED]);
            return 0;
        }

        filename = strdup(obj_str);
        if(filename == nullptr) {
            jda_object_delete(obj);
            jda_report_error(ctx, g_msg[MSG_OUT_OF_MEMORY]);
            return 0;
        }
    }
    jda_object_delete(obj);

    int fd = open(filename, O_RDONLY);
    if(fd < 0) {
        jda_report_error(ctx, "Can't find %s\r\n", filename);
        free(filename);
        return 0;
    }

    auto *robj = jda_object_create_integer(ctx, "__tmp", fd);
    jda_stack_push_object(ctx, robj);
    return 1;
}

/**
 * @brief Creates a new file on the filesystem, argument 1 is string.
 * 
 * @param ctx 
 * @return int 
 */
int jda_builtin_newfile(jda_context& ctx)
{
    jda_object *robj;
    char *filename = nullptr;
    int fd;

    auto *obj = jda_stack_pop_object(ctx);
    if(obj == nullptr) {
        jda_report_error(ctx, g_msg[MSG_DATASET_EXPECTED]);
        return 0;
    }

    /* Obtain the filename of the file and store it onto one of our fid table entry */ {
        const char *obj_str;
        obj_str = jda_object_to_string(ctx, obj);
        if(obj_str == nullptr) {
            jda_object_delete(obj);
            jda_report_error(ctx, g_msg[MSG_DATASET_EXPECTED]);
            return 0;
        }

        filename = strdup(obj_str);
        if(filename == nullptr) {
            jda_object_delete(obj);
            jda_report_error(ctx, g_msg[MSG_OUT_OF_MEMORY]);
            return 0;
        }
        jda_object_delete(obj);
    }

    fd = creat(filename, 0);
    if(fd < 0) {
        jda_report_error(ctx, "Can't create file: %s\r\n", filename);
        free(filename);
        return 0;
    }
    
    robj = jda_object_create_integer(ctx, "__tmp", fd);
    jda_stack_push_object(ctx, robj);
    return 1;
}

#include <svc.h>
#include <elf.h>

/**
 * @brief Execute a program, pulls a string from the stack that tells the
 * dataset name to read from.
 * 
 * @param ctx
 * @return int 
 */
int jda_builtin_parpgm(jda_context& ctx)
{
    char *buf = nullptr;
    size_t size;
    int fd = -1, r = -1;
    void *entry;

    auto *obj = jda_stack_pop_object(ctx);
    if(obj == nullptr) {
        jda_report_error(ctx, g_msg[MSG_ARGUMENT_EXPECTED]);
        goto end_error;
    }

    fd = jda_object_to_integer(ctx, obj);
    jda_object_delete(obj);
    if(fd >= FOPEN_MAX || fd < 0) {
        jda_report_error(ctx, g_msg[MSG_INVALID_FD], fd);
        goto end_error;
    }
    dprintf("JDA FD=%i,\r\n", fd);

    size = 32784 * 2;
    buf = (char *)malloc(size);
    if(buf == nullptr) {
        jda_report_error(ctx, g_msg[MSG_OUT_OF_MEMORY]);
        goto end_error;
    }
    memset(buf, 0, size);

    r = read(fd, buf, size);
    if(r < 0) {
        jda_report_error(ctx, "Error %i reading dataset FD(%i)\r\n", r, fd);
        goto end_error;
    }
    dprintf("BufferSize=%u\r\n", size);

    buf = (char *)realloc(buf, size + 1);
    if(buf == nullptr) {
        jda_report_error(ctx, g_msg[MSG_OUT_OF_MEMORY]);
        goto end_error;
    }

    elf64_load(buf, size, &entry);
    io_svc(SVC_EXEC_AT, (uintptr_t)entry, (uintptr_t)"CHILD", 0);
    r = 0;
end_error:
    if(buf != nullptr) {
        free(buf);
    }
end:
    return r;
}

/**
 * @brief Execute a program, pulls a string from the stack that tells the
 * dataset name to read from.
 * 
 * @param ctx
 * @return int 
 */
int jda_builtin_seqpgm(jda_context& ctx)
{
    struct jda_object *obj;
    char *buf = nullptr;
    size_t size;
    int fd = -1, r = -1;
    void *entry;

    obj = jda_stack_pop_object(ctx);
    if(obj == nullptr) {
        jda_report_error(ctx, g_msg[MSG_ARGUMENT_EXPECTED]);
        goto end_error;
    }

    fd = jda_object_to_integer(ctx, obj);
    jda_object_delete(obj);
    if(fd >= FOPEN_MAX || fd < 0) {
        jda_report_error(ctx, g_msg[MSG_INVALID_FD], fd);
        goto end_error;
    }
    dprintf("JDA FD=%i,\r\n", fd);

    size = 32784 * 2;
    buf = (char *)malloc(size);
    if(buf == nullptr) {
        jda_report_error(ctx, g_msg[MSG_OUT_OF_MEMORY]);
        goto end_error;
    }
    memset(buf, 0, size);

    r = read(fd, buf, size);
    if(r < 0) {
        jda_report_error(ctx, "Error %i reading dataset FD(%i)\r\n", r, fd);
        goto end_error;
    }
    dprintf("BufferSize=%u\r\n", size);

    buf = (char *)realloc(buf, size + 1);
    if(buf == nullptr) {
        jda_report_error(ctx, g_msg[MSG_OUT_OF_MEMORY]);
        goto end_error;
    }

    elf64_load(buf, size, &entry);
    io_svc(SVC_THREAD_AT, (uintptr_t)entry, (uintptr_t)"CHILD", 0);

    r = 0;
end_error:
    if(buf != nullptr) {
        free(buf);
    }
end:
    return r;
}

/**
 * @brief Pops multiple elements from the object stack
 * 
 * @param ctx 
 * @return int 
 */
int jda_builtin_popm(jda_context& ctx)
{
    int num;

    auto *obj = jda_stack_pop_object(ctx);
    if(obj == nullptr) {
        jda_report_error(ctx, "No specified number of elements to pop\r\n");
        return 0;
    }

    num = jda_object_to_integer(ctx, obj);
    while(num--) {
        struct jda_object *nobj;
        nobj = jda_stack_pop_object(ctx);
        if(nobj != nullptr) {
            jda_object_delete(nobj);
        }
    }
    jda_object_delete(obj);
    return 0;
}

/**
 * @brief Obtains the square root of x, takes a number from the variable stack
 * and pushes the result.
 * 
 * @param ctx 
 * @return int 
 */
int jda_builtin_sqrt(jda_context& ctx)
{
    auto *obj = jda_stack_pop_object(ctx);
    if(obj == nullptr) {
        jda_report_error(ctx, "No number given\r\n");
        return 0;
    }
    jda_number_t num = jda_object_to_number(ctx, obj);
    jda_object_delete(obj);
    num.real_value = sqrtl(num.real_value); /// @todo Have JDA support float
    auto *robj = jda_object_create_number(ctx, "__tmp", num);
    jda_stack_push_object(ctx, robj);
    return 1;
}

/**
 * @brief Obtains the factorial of x, takes a number from the variable stack
 * and pushes the result.
 * 
 * @param ctx 
 * @return int 
 */
int jda_builtin_factorial(jda_context& ctx)
{
    auto *obj = jda_stack_pop_object(ctx);
    if(obj == nullptr) {
        jda_report_error(ctx, "No number given\r\n");
        return 0;
    }
    jda_number_t num = jda_object_to_number(ctx, obj);
    jda_object_delete(obj);
    num.real_value = factl(num.real_value); /// @todo Have JDA support float
    auto *robj = jda_object_create_number(ctx, "__tmp", num);
    jda_stack_push_object(ctx, robj);
    return 1;
}

/**
 * @brief Obtains the sine of x, takes a number from the variable stack
 * and pushes the result.
 * 
 * @param ctx 
 * @return int 
 */
int jda_builtin_sin(jda_context& ctx)
{
    auto *obj = jda_stack_pop_object(ctx);
    if(obj == nullptr) {
        jda_report_error(ctx, "No number given\r\n");
        return 0;
    }
    jda_number_t num = jda_object_to_number(ctx, obj);
    jda_object_delete(obj);
    num.real_value = sinl(num.real_value); /// @todo Have JDA support float
    auto *robj = jda_object_create_number(ctx, "__tmp", num);
    jda_stack_push_object(ctx, robj);
    return 1;
}

/**
 * @brief Obtains the cosine of x, takes a number from the variable stack
 * and pushes the result.
 * 
 * @param ctx 
 * @return int 
 */
int jda_builtin_cos(jda_context& ctx)
{
    auto *obj = jda_stack_pop_object(ctx);
    if(obj == nullptr) {
        jda_report_error(ctx, "No number given\r\n");
        return 0;
    }
    jda_number_t num = jda_object_to_number(ctx, obj);
    jda_object_delete(obj);
    num.real_value = cosl(num.real_value); /// @todo Have JDA support float
    auto *robj = jda_object_create_number(ctx, "__tmp", num);
    jda_stack_push_object(ctx, robj);
    return 1;
}

/**
 * @brief Obtains the tangent of x, takes a number from the variable stack
 * and pushes the result.
 * 
 * @param ctx 
 * @return int 
 */
int jda_builtin_tan(jda_context& ctx)
{
    auto *obj = jda_stack_pop_object(ctx);
    if(obj == nullptr) {
        jda_report_error(ctx, "No number given\r\n");
        return 0;
    }
    jda_number_t num = jda_object_to_number(ctx, obj);
    jda_object_delete(obj);
    num.real_value = tanl(num.real_value); /// @todo Have JDA support float
    auto *robj = jda_object_create_number(ctx, "__tmp", num);
    jda_stack_push_object(ctx, robj);
    return 1;
}


/**
 * @brief Obtains the hyperbolic sine of x, takes a number from the variable stack
 * and pushes the result.
 * 
 * @param ctx 
 * @return int 
 */
int jda_builtin_sinh(jda_context& ctx)
{
    auto *obj = jda_stack_pop_object(ctx);
    if(obj == nullptr) {
        jda_report_error(ctx, "No number given\r\n");
        return 0;
    }
    jda_number_t num = jda_object_to_number(ctx, obj);
    jda_object_delete(obj);
    num.real_value = sinhl(num.real_value); /// @todo Have JDA support float
    auto *robj = jda_object_create_number(ctx, "__tmp", num);
    jda_stack_push_object(ctx, robj);
    return 1;
}

/**
 * @brief Obtains the hyperbolic cosine of x, takes a number from the variable stack
 * and pushes the result.
 * 
 * @param ctx 
 * @return int 
 */
int jda_builtin_cosh(jda_context& ctx)
{
    auto *obj = jda_stack_pop_object(ctx);
    if(obj == nullptr) {
        jda_report_error(ctx, "No number given\r\n");
        return 0;
    }
    jda_number_t num = jda_object_to_number(ctx, obj);
    jda_object_delete(obj);
    num.real_value = coshl(num.real_value); /// @todo Have JDA support float
    auto *robj = jda_object_create_number(ctx, "__tmp", num);
    jda_stack_push_object(ctx, robj);
    return 1;
}

/**
 * @brief Obtains the hyperbolic tangent of x, takes a number from the variable stack
 * and pushes the result.
 * 
 * @param ctx 
 * @return int 
 */
int jda_builtin_tanh(jda_context& ctx)
{
    auto *obj = jda_stack_pop_object(ctx);
    if(obj == nullptr) {
        jda_report_error(ctx, "No number given\r\n");
        return 0;
    }
    jda_number_t num = jda_object_to_number(ctx, obj);
    jda_object_delete(obj);
    num.real_value = tanhl(num.real_value); /// @todo Have JDA support float
    auto *robj = jda_object_create_number(ctx, "__tmp", num);
    jda_stack_push_object(ctx, robj);
    return 1;
}

/**
 * @brief Obtains the euler constant
 * 
 * @param ctx 
 * @return int 
 */
int jda_builtin_ceuler(jda_context& ctx)
{
    constinit static jda_number_t rnum = {
        .real_value = (long double)M_E,
        .imaginary_value = 0.f,
        .power = 1.f,
    };
    auto *robj = jda_object_create_number(ctx, "__tmp", rnum);
    jda_stack_push_object(ctx, robj);
    return 1;
}

/**
 * @brief Obtains the pi constant
 * 
 * @param ctx 
 * @return int 
 */
int jda_builtin_cpi(jda_context& ctx)
{
    constinit static jda_number_t rnum = {
        .real_value = (long double)M_PI,
        .imaginary_value = 0.f,
        .power = 1.f,
    };
    auto *robj = jda_object_create_number(ctx, "__tmp", rnum);
    jda_stack_push_object(ctx, robj);
    return 1;
}

/**
 * @brief Obtains the real part of a number
 * 
 * @param ctx 
 * @return int 
 */
int jda_builtin_real(jda_context& ctx)
{
    auto *obj = jda_stack_pop_object(ctx);
    if(obj == nullptr) {
        jda_report_error(ctx, "No number given\r\n");
        return 0;
    }

    jda_number_t num = jda_object_to_number(ctx, obj);
    jda_object_delete(obj);

    jda_number_t rnum;
    rnum.real_value = num.real_value;
    rnum.imaginary_value = 0.f;
    rnum.power = num.power;
    auto *robj = jda_object_create_number(ctx, "__tmp", rnum);
    jda_stack_push_object(ctx, robj);
    return 1;
}

/**
 * @brief Obtains the imaginary part of a number
 * 
 * @param ctx 
 * @return int 
 */
int jda_builtin_imgy(jda_context& ctx)
{
    auto *obj = jda_stack_pop_object(ctx);
    if(obj == nullptr) {
        jda_report_error(ctx, "No number given\r\n");
        return 0;
    }

    jda_number_t num = jda_object_to_number(ctx, obj);
    jda_object_delete(obj);

    jda_number_t rnum;
    rnum.real_value = num.imaginary_value;
    rnum.imaginary_value = 0.f;
    rnum.power = num.power;
    auto *robj = jda_object_create_number(ctx, "__tmp", rnum);
    jda_stack_push_object(ctx, robj);
    return 1;
}

const char *init_cmd = "EXEC(OPENFILE(\"/TAPE/JDALIB$AUTOINIT\"))\r\n";

/**
 * @brief A single JDA session terminal
 * 
 */
void jda_term(void)
{
    dprintf("Starting JDA term");
    auto *ctx = (jda_context *)calloc(1, sizeof(jda_context));
    if(ctx == nullptr)
        goto end;

    if(jda_context_init(*ctx) < 0)
        goto end;
#if 0
    usersys::user_set_current(0);
#endif

    /* Read autonit.jda from the IPL disk */
    dprintf("Reading AUTOINIT.JDA, InitCmd=\"%s\"", init_cmd);
    if(jda_lex_line(*ctx, init_cmd) == 0) {
        if(jda_lexer_to_rpn(*ctx) == 0) {
            jda_parse(*ctx);
        }
    }
    
    /* Execute the command shell loop */
    while(1) {
        char tmpbuf[80];
#if 0
        jda_printf(*ctx, "jda-%s> ", usersys::user_get_by_id(usersys::user_get_current())->name);
#endif
        jda_printf(*ctx, "jda> ");

        memset(tmpbuf, 0, sizeof tmpbuf);
        if(jda_get_input(*ctx, tmpbuf, sizeof tmpbuf - 1) < 0)
            goto end;

        bool is_empty = true;
        for(size_t i = 0; i < strlen(tmpbuf); i++) {
            if(isspace(tmpbuf[i]) || tmpbuf[i] == '\r' || tmpbuf[i] == '\n' || tmpbuf[i] == '\0')
                continue;
            /* Anything not a whitespace or a null terminator is valid data */
            is_empty = false;
            break;
        }
        if(is_empty) continue;
        
        if(jda_lex_line(*ctx, tmpbuf) == 0) {
            if(jda_lexer_to_rpn(*ctx) == 0) {
                jda_parse(*ctx);
            }
        }

        if(ctx->fail_cnt > 1) {
            jda_printf(*ctx, "Problems? Try typing \"HELP()\"!\r\n");
        }
    }
end:
    if(ctx != nullptr) {
        free(ctx);
    }
    return;
}

int main(int argc, char **argv)
{
    jda_term();
    return 0;
}
