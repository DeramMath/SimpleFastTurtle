#include "parser.h"

/*
 *  Create the Abstract Syntax Tree
 *  Take a list of token
 *  Return a tree of token
 */

static void save_in_file(struct List *s_tree_token, struct List *s_list_token);
static void parse(struct List *s_tree_token, struct List *s_list_token);
static int next_node(struct ParserNode *s_cur);
static void remove_all_token(struct ParserNode *s_cur, char to_remove);
static void parse_expression_block(struct ParserNode *s_cur,
                                   struct Statement *s_statement);
static void parse_statement_block(struct ParserNode *s_cur,
                                  struct Statement *s_statement);
static void statement_inline(struct ParserNode *s_cur,
                             struct Statement *s_statement);
static void statement_block(struct ParserNode *s_cur,
                            struct Statement *s_statement);
static void statement_start(struct ParserNode *s_cur,
                                  struct Statement *s_statement);
static struct Statement *parse_statement(struct ParserNode *s_cur);
static int is_token_valid_in_expression(struct TokenNode *token,
                                        int bracket_stack);
static int expression_can_token_follow(struct TokenNode *last2,
                                       struct TokenNode *last,
                                       struct TokenNode *cur);
static struct Expression *parse_expression(struct ParserNode *s_cur);
static struct Expression *parse_nested_expression(
                                            struct TokenNode **expression_arr,
                                            int start, int stop);
static int is_function(struct TokenNode **expression_arr, int start, int stop);
static void remove_useless_rb(struct TokenNode **expression_arr,
                              int *start, int *stop);
static struct TokenNode *get_token_null();



static const short OPERATOR_PRECEDENCE[127];



void parser_process(struct List *s_tree_token,
                    struct List *s_list_token,
                    int option_save,
                    int option_print_tree)
{
    parse(s_tree_token, s_list_token);

    /*OPTIONS*/
    if (option_save)
        save_in_file(s_tree_token, s_list_token);


    if (option_print_tree)
        token_tree_fprintf(stdout, s_tree_token);
}

static void save_in_file(struct List *s_tree_token, struct List *s_list_token)
{

}

static void parse(struct List *s_tree_token, struct List *s_list_token)
{
    if (!s_list_token->size)
        return;

    struct ParserNode s_cur = {NULL, NULL};
    s_cur.node = s_list_token->head;
    s_cur.token = (struct TokenNode *)s_list_token->head->data;

    while (s_cur.node != NULL)
    {
        list_push(s_tree_token, parse_statement(&s_cur));
    }
}

static int next_node(struct ParserNode *s_cur)
{
    printf("ID: %d\n", s_cur->token->id);
    s_cur->node = s_cur->node->next;
    if (s_cur->node == NULL)
        return 0;

    s_cur->token = (struct TokenNode *)s_cur->node->data;
    return 1;
}

static void remove_all_token(struct ParserNode *s_cur, char to_remove)
{
    while ((s_cur->token->id == to_remove) && next_node(s_cur))
        ;
}

static void parse_expression_block(struct ParserNode *s_cur,
                                   struct Statement *s_statement)
{
    struct Expression *new_expression = parse_expression(s_cur);
    if (new_expression != NULL)
        list_push(s_statement->expressions, new_expression);

    while (s_cur->token->id == TOK_SEP_COMMA)
    {
        struct Expression *new_expression = parse_expression(s_cur);
        if (new_expression != NULL)
            list_push(s_statement->expressions, new_expression);
    }
}

static void parse_statement_block(struct ParserNode *s_cur,
                            struct Statement *s_statement)
{
    if (s_cur->token->id != TOK_SEP_CBS)
    {
        statement_inline(s_cur, s_statement);
    }
    else
    {
        statement_block(s_cur, s_statement);
    }
}

static void statement_inline(struct ParserNode *s_cur,
                                   struct Statement *s_statement)
{
    struct Statement *s_new_statement = parse_statement(s_cur);
    if (s_new_statement != NULL)
        list_push(s_statement->statements, s_new_statement);

    remove_all_token(s_cur, TOK_SEP_SEMI);
}

static void statement_block(struct ParserNode *s_cur,
                                  struct Statement *s_statement)
{
    if (!next_node(s_cur)) /* { */
    {
        error_printd(ERROR_PARSER_INVALID_STATEMENT_BLOCK,
                     &s_statement->token->line);
    }

    while (s_cur->token->id != TOK_SEP_CBE)
    {
        struct Statement *s_new_statement = parse_statement(s_cur);
        if (s_new_statement != NULL)
            list_push(s_statement->statements, s_new_statement);
    }

    next_node(s_cur); /* } */
}

static void statement_start(struct ParserNode *s_cur,
                            struct Statement *s_statement)
{
    s_statement->token = s_cur->token;
    if (!next_node(s_cur))
    {
        error_printd(ERROR_PARSER_INVALID_STATEMENT_START,
                     &s_statement->token->line);
    }
}

static struct Statement *parse_statement(struct ParserNode *s_cur)
{
    printf("statement\n");

    unsigned long int tmp_line = s_cur->token->line;
    remove_all_token(s_cur, TOK_SEP_SEMI);
    if (s_cur->node == NULL) //todo
        error_printd(ERROR_PARSER_INVALID_STATEMENT_BLOCK_END, &tmp_line);

    if (s_cur->token->id == TOK_SEP_CBE)
        return NULL;

    struct Statement *s_statement = statement_new();

    if (s_cur->token->type == TOK_TYPE_KEY)
    {
        switch(s_cur->token->id)
        {
        case TOK_KEY_FOR: /*for transformed to a while ?*/
        case TOK_KEY_WHILE:
        case TOK_KEY_IF:
        case TOK_KEY_ELIF:
            statement_start(s_cur, s_statement);
            parse_expression_block(s_cur, s_statement);

            if (s_statement->expressions->size == 0)
            {
                error_printd(ERROR_PARSER_INVALID_NUMBER_PARAMETERS,
                             &s_statement->token->line);
            }

            parse_statement_block(s_cur, s_statement);
            break;
        case TOK_KEY_FN:
            statement_start(s_cur, s_statement);

            if (s_cur->token->type != TOK_TYPE_ID)
            {
                error_printd(ERROR_PARSER_INVALID_STATEMENT_START,
                             &s_statement->token->line);
            }
            statement_start(s_cur, s_statement);

            parse_expression_block(s_cur, s_statement);
            parse_statement_block(s_cur, s_statement);
            break;
        case TOK_KEY_ELSE:
            statement_start(s_cur, s_statement);
            parse_statement_block(s_cur, s_statement);
            break;
        case TOK_KEY_VAR:
            statement_start(s_cur, s_statement);
            parse_expression_block(s_cur, s_statement);

            if (s_statement->expressions->size == 0)
            {
                error_printd(ERROR_PARSER_INVALID_VAR_ASSIGNMENT,
                             &s_statement->token->line);
            }
            break;
        case TOK_KEY_BREAK:
            statement_start(s_cur, s_statement);
            break;
        case TOK_KEY_RETURN:
            statement_start(s_cur, s_statement);
            break;
        }
    }
    else if (is_token_valid_in_expression(s_cur->token, 1))
    {
        s_statement->token = NULL;
        parse_expression_block(s_cur, s_statement);
    }
    else
    {
        error_printd(ERROR_PARSER_INVALID_STATEMENT, &s_statement->token->line);
    }
    return s_statement;
}

static int is_token_valid_in_expression(struct TokenNode *token,
                                        int bracket_stack)
{
    if (token->id == TOK_SEP_COMMA)
        return bracket_stack;

    return (token->type == TOK_TYPE_ID
         || token->type == TOK_TYPE_LI
         || token->type == TOK_TYPE_OP
         || token->id   == TOK_SEP_RBS
         || token->id   == TOK_SEP_RBE
         || token->id   == TOK_SEP_SBS
         || token->id   == TOK_SEP_SBE
         || token->id   == TOK_SEP_DOT
         || token->id   == TOK_KEY_NULL);
}

static int expression_can_token_follow(struct TokenNode *last2,
                                       struct TokenNode *last,
                                       struct TokenNode *cur)
{
    if (last == NULL)
        return 1;

    int is_last_id_li = last->type == TOK_TYPE_ID || last->type == TOK_TYPE_LI;
    int is_cur_id_li = cur->type == TOK_TYPE_ID || cur->type == TOK_TYPE_LI;

    if (is_last_id_li && is_cur_id_li)
        return 0;

    if (last2 == NULL)
        return 1;

    if (last->id == TOK_OP_INCR || last->id == TOK_OP_DECR)
    {
        return !(
                    (cur->type == TOK_TYPE_ID
                  || cur->type == TOK_TYPE_LI
                  || cur->type == TOK_TYPE_KEY)
                &&
                    (last2->type == TOK_TYPE_ID
                  || last2->type == TOK_TYPE_LI
                  || last2->type == TOK_TYPE_KEY)
                );
    }

    return 1;
}

static struct Expression *parse_expression(struct ParserNode *s_cur)
{
    unsigned long int tmp_line = s_cur->token->line;
    remove_all_token(s_cur, TOK_SEP_COMMA);
    if (s_cur->node == NULL)
        error_printd(ERROR_PARSER_INVALID_EXPRESSION, &tmp_line);

    /* Count Expression size and check if there is as much opening and closing round brackets */
    int bracket_stack = 0;
    unsigned int expression_size = 0;
    struct ListNode *copy_node = s_cur->node;
    struct TokenNode *last2 = NULL;
    struct TokenNode *last = NULL;

    while (is_token_valid_in_expression(s_cur->token, bracket_stack)
        && expression_can_token_follow(last2, last, s_cur->token))
    {
        expression_size++;

        if (s_cur->token->id == TOK_SEP_RBS)
            bracket_stack++;
        else if (s_cur->token->id == TOK_SEP_RBE)
            bracket_stack--;

        last2 = last;
        last = s_cur->token;
        if (bracket_stack < 0 || !next_node(s_cur))
            break;
    }

    if (bracket_stack)
        error_printd(ERROR_PARSER_INVALID_EXPRESSION, &tmp_line);

    /* Copy the expression to an array */
    struct TokenNode **expression_arr = malloc(sizeof(struct TokenNode*)*expression_size);

    for (int i = 0; i < expression_size; i++)
    {
        expression_arr[i] = (struct TokenNode*)copy_node->data;
        printf("%s", expression_arr[i]->token);
        copy_node = copy_node->next;
    }
    printf("\n");

    struct Expression *s_expression
            = parse_nested_expression(expression_arr, 0, expression_size-1);

    free(expression_arr);
    return s_expression;
}

/*
 *  Remove all useless round brackets that are arround the nested expression
 *  Find the operator with the lowest precedence and that is not inside brackets
 *  If there is an operator create the Left and Right branch from that operator
 *  If only one element, it's an end node (ID or LI)
 *  Else it's a null node
 */
static struct Expression *parse_nested_expression(
                                            struct TokenNode **expression_arr,
                                            int start, int stop)
{
    struct Expression *s_expression = expression_new();

    remove_useless_rb(expression_arr, &start, &stop);

    /* Expression parsing */
    short lowest_precedence = 100;
    int lowest_precedence_index = 0;
    int bracket_stack = 0;
    for (int i = start; i <= stop; i++)
    {
        if (expression_arr[i]->id == TOK_SEP_RBS)
        {
            bracket_stack++;
        }
        else if (expression_arr[i]->id == TOK_SEP_RBE)
        {
            bracket_stack--;
        }
        else if (!bracket_stack && expression_arr[i]->type == TOK_TYPE_OP)
        {
            short cur_precedence = OPERATOR_PRECEDENCE[expression_arr[i]->id];
            if (cur_precedence < lowest_precedence)
            {
                lowest_precedence = cur_precedence;
                lowest_precedence_index = i;
            }
        }
    }

    if (lowest_precedence < 100)
    {
        s_expression->type = EXPRESSION_TYPE_OP;
        s_expression->operator = malloc(sizeof(struct Operator));
        s_expression->operator->token = expression_arr[lowest_precedence_index];
        s_expression->operator->left
                = parse_nested_expression(expression_arr, start, lowest_precedence_index-1);
        s_expression->operator->right
                = parse_nested_expression(expression_arr, lowest_precedence_index+1, stop);
    }
    else if (start == stop) /* only one token in the array */
    {
        if (expression_arr[start]->type == TOK_TYPE_LI
            || expression_arr[start]->id == TOK_KEY_NULL)
        {
            s_expression->type = EXPRESSION_TYPE_LI;
            s_expression->literal = expression_arr[start];
        }
        else if (expression_arr[start]->type == TOK_TYPE_ID)
        {
            s_expression->type = EXPRESSION_TYPE_ID;
            s_expression->identifier = expression_arr[start];
        }
        else
        {
            error_printd(ERROR_PARSER_INVALID_EXPRESSION,
                         &expression_arr[start]->line);
        }
    }
    else if (is_function(expression_arr, start, stop))
    {
        printf("function\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        s_expression->type = EXPRESSION_TYPE_LI;
        s_expression->identifier = get_token_null();
    }

    return s_expression;
}

static int is_function(struct TokenNode **expression_arr, int start, int stop)
{
    if ((stop - start + 1) < 3)
        return 0;

    return expression_arr[start]->type == TOK_TYPE_ID
        && expression_arr[start+1]->id == TOK_SEP_RBS
        && expression_arr[stop]->id    == TOK_SEP_RBE;
}

static void remove_useless_rb(struct TokenNode *expression[],
                              int *start, int *stop)
{
    int lowest_inside = 0;
    int bracket_stack = 0;
    int min = *start;
    int max = *stop;
    while(expression[min]->id == TOK_SEP_RBS
       && expression[max]->id == TOK_SEP_RBE)
    {
        lowest_inside++;
        bracket_stack++;
        min++;
        max--;
    }

    while(min <= max)
    {
        if (!lowest_inside)
            return;

        if (expression[min]->id == TOK_SEP_RBS)
            bracket_stack++;
        else if (expression[min]->id == TOK_SEP_RBE)
            bracket_stack--;

        if (bracket_stack < lowest_inside)
            lowest_inside--;

        min++;
    }

    *start += lowest_inside;
    *stop -= lowest_inside;

    return;
}

static struct TokenNode *get_token_null()
{
    //File line start at one so using 0 show that the parser added it to the code
    return token_new(0, TOK_TYPE_KEY, TOK_KEY_NULL, "null");
}



/*
 * From :
 * https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Operators/Operator_Precedence
 */
static const short OPERATOR_PRECEDENCE[127] = {
/* Single operators */
    [TOK_OP_ASIGN]      = 3,
    [TOK_OP_NOT]        = 17,
    [TOK_OP_INF]        = 12,
    [TOK_OP_SUP]        = 12,
    [TOK_OP_OR]         = 8,
    [TOK_OP_AND]        = 10,
    [TOK_OP_XOR]        = 9,
    [TOK_OP_ADD]        = 14,
    [TOK_OP_SUB]        = 14,
    [TOK_OP_BY]         = 15,
    [TOK_OP_DIV]        = 15,
    [TOK_OP_MOD]        = 15,

/* Double operators */
    [TOK_OP_INCR]       = 17,
    [TOK_OP_DECR]       = 17,
    [TOK_OP_EXPO]       = 16,
    [TOK_OP_SQRT]       = 16,
    [TOK_OP_EQUAL]      = 11,
    [TOK_OP_NOT_EQUAL]  = 11,
    [TOK_OP_INF_EQUAL]  = 12,
    [TOK_OP_SUP_EQUAL]  = 12,
    [TOK_OP_LOGIC_AND]  = 6,
    [TOK_OP_LOGIC_OR]   = 5,
    [TOK_OP_ADD_ASIGN]  = 3,
    [TOK_OP_SUB_ASIGN]  = 3,
    [TOK_OP_BY_ASIGN]   = 3,
    [TOK_OP_DIV_ASIGN]  = 3,
    [TOK_OP_MOD_ASIGN]  = 3,

/* Separator */
    [TOK_SEP_RBS]       = 21,
    [TOK_SEP_RBE]       = 21,
    [TOK_SEP_SBS]       = 20,
    [TOK_SEP_SBE]       = 20,
    [TOK_SEP_DOT]       = 20,
};

const short OPERATOR_ASSOCIATIVITY[127] = {
/* Single operators */
    [TOK_OP_ASIGN]      = ASSOCIATIVITY_RIGHT_TO_LEFT,
    [TOK_OP_NOT]        = ASSOCIATIVITY_RIGHT_TO_LEFT,
    [TOK_OP_INF]        = ASSOCIATIVITY_LEFT_TO_RIGHT,
    [TOK_OP_SUP]        = ASSOCIATIVITY_LEFT_TO_RIGHT,
    [TOK_OP_OR]         = ASSOCIATIVITY_LEFT_TO_RIGHT,
    [TOK_OP_AND]        = ASSOCIATIVITY_LEFT_TO_RIGHT,
    [TOK_OP_XOR]        = ASSOCIATIVITY_LEFT_TO_RIGHT,
    [TOK_OP_ADD]        = ASSOCIATIVITY_RIGHT_TO_LEFT,
    [TOK_OP_SUB]        = ASSOCIATIVITY_RIGHT_TO_LEFT,
    [TOK_OP_BY]         = ASSOCIATIVITY_LEFT_TO_RIGHT,
    [TOK_OP_DIV]        = ASSOCIATIVITY_LEFT_TO_RIGHT,
    [TOK_OP_MOD]        = ASSOCIATIVITY_LEFT_TO_RIGHT,

/* Double operators */
    [TOK_OP_INCR]       = ASSOCIATIVITY_NA,
    [TOK_OP_DECR]       = ASSOCIATIVITY_NA,
    [TOK_OP_EXPO]       = ASSOCIATIVITY_RIGHT_TO_LEFT,
    [TOK_OP_SQRT]       = ASSOCIATIVITY_RIGHT_TO_LEFT,
    [TOK_OP_EQUAL]      = ASSOCIATIVITY_LEFT_TO_RIGHT,
    [TOK_OP_NOT_EQUAL]  = ASSOCIATIVITY_LEFT_TO_RIGHT,
    [TOK_OP_INF_EQUAL]  = ASSOCIATIVITY_LEFT_TO_RIGHT,
    [TOK_OP_SUP_EQUAL]  = ASSOCIATIVITY_LEFT_TO_RIGHT,
    [TOK_OP_LOGIC_AND]  = ASSOCIATIVITY_LEFT_TO_RIGHT,
    [TOK_OP_LOGIC_OR]   = ASSOCIATIVITY_LEFT_TO_RIGHT,
    [TOK_OP_ADD_ASIGN]  = ASSOCIATIVITY_RIGHT_TO_LEFT,
    [TOK_OP_SUB_ASIGN]  = ASSOCIATIVITY_RIGHT_TO_LEFT,
    [TOK_OP_BY_ASIGN]   = ASSOCIATIVITY_RIGHT_TO_LEFT,
    [TOK_OP_DIV_ASIGN]  = ASSOCIATIVITY_RIGHT_TO_LEFT,
    [TOK_OP_MOD_ASIGN]  = ASSOCIATIVITY_RIGHT_TO_LEFT,

/* Separator */
    [TOK_SEP_RBS]       = ASSOCIATIVITY_NA,
    [TOK_SEP_RBE]       = ASSOCIATIVITY_NA,
    [TOK_SEP_SBS]       = ASSOCIATIVITY_LEFT_TO_RIGHT,
    [TOK_SEP_SBE]       = ASSOCIATIVITY_LEFT_TO_RIGHT,
    [TOK_SEP_DOT]       = ASSOCIATIVITY_LEFT_TO_RIGHT,
};
