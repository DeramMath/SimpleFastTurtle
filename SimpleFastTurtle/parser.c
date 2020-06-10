#include "parser.h"

/*
 *  Create the Abstract Syntax Tree
 *  Take a list of token
 *  Return a tree of token
 */

static void save_in_file(struct List *s_tree_token, struct List *s_list_token);
static void parse(struct List *s_tree_token, struct List *s_list_token);
static int next_node(struct ParserNode *s_cur);
static struct Statement *parse_statement(struct ParserNode *s_cur);
static struct Expression *parse_expression(struct ParserNode *s_cur);
static struct Expression *parse_nested_expression(
                                            struct TokenNode **expression_arr,
                                            int start, int stop);
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
        return 1;

    s_cur->token = (struct TokenNode *)s_cur->node->data;
    return 0;
}

static struct Statement *parse_statement(struct ParserNode *s_cur)
{
    printf("statement\n");

    /*remove all ; before parsing the statement*/
    unsigned long int tmp_line = s_cur->token->line;
    while (s_cur->token->id == TOK_SEP_SEMI)
    {
        if (next_node(s_cur)) /* ; */
            error_printd(ERROR_PARSER_INVALID_STATEMENT_BLOCK_END, &tmp_line);
    }

    if (s_cur->token->id == TOK_SEP_CBE)
        return NULL;

    /*parse the statement recursively*/
    struct Statement *s_statement = malloc(sizeof(struct Statement));
    s_statement->token = s_cur->token;
    s_statement->name = NULL;

    if (s_cur->token->type == TOK_TYPE_KEY)
    {
        switch(s_cur->token->id)
        {
        case TOK_KEY_FOR: /*for transformed to a while ?*/
        case TOK_KEY_WHILE:
        case TOK_KEY_IF:
        case TOK_KEY_ELIF:
            s_statement->expressions = list_new();
            s_statement->statements = list_new();

            if (next_node(s_cur)) /* for/while/if/elif */
                error_printd(ERROR_PARSER_INVALID_STATEMENT_BLOCK_START, &s_statement->token->line);

            while (s_cur->token->id != TOK_SEP_CBS)
            {
                list_push(s_statement->expressions,
                            parse_expression(s_cur)); /*todo check if expression not null*/
            }

            if (s_statement->expressions->size == 0)
                error_printd(ERROR_PARSER_INVALID_NUMBER_PARAMETERS, &s_statement->token->line);

            if (next_node(s_cur)) /* { */
                error_printd(ERROR_PARSER_INVALID_STATEMENT_BLOCK_START, &s_statement->token->line);

            while (s_cur->token->id != TOK_SEP_CBE)
            {
                struct Statement *s_new_statement
                        = parse_statement(s_cur);
                if (s_new_statement)
                    list_push(s_statement->statements, s_new_statement);
            }

            if (s_statement->statements->size == 0)
                warning_printd(WARNING_PARSER_EMPTY_STATEMENT, &s_statement->token->line);

            if (next_node(s_cur)) /* } */
                break;
            //error_printd(ERROR_PARSER_INVALID_STATEMENT_BLOCK_END, &s_statement->token->line);

            break;

        case TOK_KEY_FN:
            s_statement->expressions = list_new();
            s_statement->statements = list_new();

            if (next_node(s_cur)) /* fn */
                error_printd(ERROR_PARSER_INVALID_STATEMENT_BLOCK_END, &s_statement->token->line);

            s_statement->name = s_cur->token;

            /*todo error not identifier */
            if (next_node(s_cur)) /* identifier (name) */
                error_printd(ERROR_PARSER_INVALID_STATEMENT_BLOCK_START, &s_statement->token->line);

            while (s_cur->token->id != TOK_SEP_CBS)
            {
                list_push(s_statement->expressions,
                            parse_expression(s_cur));
            }

            if (next_node(s_cur)) /* { */
                error_printd(ERROR_PARSER_INVALID_STATEMENT_BLOCK_START, &s_statement->token->line);

            while (s_cur->token->id != TOK_SEP_CBE)
            {
                struct Statement *s_new_statement
                        = parse_statement(s_cur);
                if (s_new_statement)
                    list_push(s_statement->statements, s_new_statement);
            }

            if (s_statement->statements->size == 0)
                warning_printd(WARNING_PARSER_EMPTY_STATEMENT, &s_statement->token->line);

            if (next_node(s_cur)) /* } */
                break;
            //error_printd(ERROR_PARSER_INVALID_STATEMENT_BLOCK_END, &s_statement->token->line);

            break;
        #if 0
        case TOK_KEY_ELSE:
            s_statement->statements = list_new();

            if (next_node(p_cur_node, p_cur_token)) /* else */
                error_printd(ERROR_PARSER_INVALID_STATEMENT_BLOCK_START, &s_statement->token->line);

            if (next_node(p_cur_node, p_cur_token)) /* { */
                error_printd(ERROR_PARSER_INVALID_STATEMENT_BLOCK_START, &s_statement->token->line);

            while (cur_token->id != TOK_SEP_CBE)
            {
                struct Statement *s_new_statement
                        = parse_statement(p_cur_node, p_cur_token, cur_node, cur_token);

                if (s_new_statement)
                    list_push(s_statement->statements, s_new_statement);
            }

            if (s_statement->statements->size == 0)
                warning_printd(WARNING_PARSER_EMPTY_STATEMENT, &s_statement->token->line);

            if (next_node(p_cur_node, p_cur_token)) /* } */
                error_printd(ERROR_PARSER_INVALID_STATEMENT_BLOCK_END, &s_statement->token->line);

            break;

        case TOK_KEY_VAR:
            if (next_node(p_cur_node, p_cur_token)) /* var */
                error_printd(ERROR_PARSER_INVALID_STATEMENT_BLOCK_END, &s_statement->token->line);

            s_statement->name = cur_token;

            if (next_node(p_cur_node, p_cur_token)) /* identifier (name) */
                error_printd(ERROR_PARSER_INVALID_STATEMENT_BLOCK_START, &s_statement->token->line);

            if (cur_token->id == TOK_OP_EQUAL)
            {
                if (next_node(p_cur_node, p_cur_token)) /* = */
                    error_printd(ERROR_PARSER_INVALID_STATEMENT_BLOCK_START,
                                &s_statement->token->line);

                s_statement->expression
                        = parse_expression(p_cur_node, p_cur_token, cur_node, cur_token);
            }
            else
            {
                s_statement->expression = get_token_null();
            }

            break;

        case TOK_KEY_BREAK:
            if (next_node(p_cur_node, p_cur_token)) /* break */
                error_printd(ERROR_PARSER_INVALID_STATEMENT_BLOCK_END, &s_statement->token->line);
            break;

        case TOK_KEY_RETURN:
            break;
        #endif
        }
    }
    return s_statement;
}

static struct Expression *parse_expression(struct ParserNode *s_cur)
{
    /*remove all ',' before parsing the statement*/
    unsigned long int tmp_line = s_cur->token->line;
    while (s_cur->token->id == TOK_SEP_COMMA)
    {
        if (next_node(s_cur)) /* , */
            error_printd(ERROR_PARSER_INVALID_EXPRESSION, &tmp_line);
    }

    /* Count Expression size and check if there is as much opening and closing round brackets */
    int bracket_stack = 0;
    unsigned int expression_size = 0;
    struct ListNode *copy_node = s_cur->node;

    while (s_cur->token->id != TOK_SEP_CBS
        && s_cur->token->id != TOK_SEP_SEMI
        && s_cur->token->id != TOK_SEP_COMMA)
    {
        expression_size++;

        if (s_cur->token->id == TOK_SEP_RBS)
            bracket_stack++;
        else if (s_cur->token->id == TOK_SEP_RBE)
            bracket_stack--;

        if (bracket_stack < 0 || next_node(s_cur))
            break;
    }

    if (!bracket_stack)
        ; //error invalid expression

    /* Copy the expression to an array */
    struct TokenNode **expression_arr = malloc(sizeof(struct TokenNode*)*expression_size);

    for (int i = 0; i < expression_size; i++)
    {
        expression_arr[i] = (struct TokenNode*)copy_node->data;
        copy_node = copy_node->next;
    }

    struct Expression *s_expression
            = parse_nested_expression(expression_arr, 0, expression_size-1);

    free(expression_arr);
    return s_expression;
}

/*
 * Find a litteral or identifier
 * Then check the operator/separator at his left, and at his right
 * If there is only one OP then apply the op to him (which mean that the op nest him either at
 * his left of his right)
 * If there are 2 OP then check for the precedence between them, then nest the operation that
 * have a highest precedence with the LI/ID to the left/right of the OP with the least precedence
 *
 * Separators are being check more "manually" in the code because they come together
 * for the brackets and the behavior is different than operator
 */
static struct Expression *parse_nested_expression(
                                            struct TokenNode **expression_arr,
                                            int start, int stop)
{
    struct Expression *s_expression = malloc(sizeof(struct Expression));

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
            printf("error parse_nested_expression not possible\n");
        }
    }
    else
    {
        s_expression->type = EXPRESSION_TYPE_LI;
        s_expression->identifier = get_token_null();
    }

    return s_expression;
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
