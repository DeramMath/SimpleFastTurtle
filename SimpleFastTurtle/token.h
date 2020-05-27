#ifndef F_TOKEN_H
#define F_TOKEN_H

void token_fprintf(FILE *output, void *data);
struct TokenNode *token_new(const unsigned long int current_line,
                            const char type,
                            const char id,
                            const char *token);
void token_free(void *data);



struct TokenNode {
    unsigned long int line;
    char type;
    char id;
    char *token;
};

#if 0
struct TokenTree *token_tree_new();
void token_tree_clear(struct TokenTree *list);
void token_tree_delete(struct TokenTree *list);
#endif


#if 0
struct TokenTreeNode {
    struct TokenNode *token;
    struct 
};

struct FonctionDeclaration {
    struct TokenNode *identifier;
    struct TokenList *parameters;
    struct StatementList *statements;
};

struct TokenTreeStatement {
    struct TokenNode *token;
    struct ExpressionList *expression;
    struct StatementList *statements;
}




struct TokenTree {
    int size;

};

union TokenTreeListNode {
    struct TokenTreeStatement *statement;
    struct TokenTreeExpression *expression;
};

struct TokenTreeList {
    int size;
    union TokenTreeListNode data;
    struct TokenTreeList *next;
};

struct TokenTreeStatement {
    char keyword;
    struct TokenTreeExpression *expression;
    struct TokenTreeList *block;
};

struct TokenTreeExpression {

};


/*Logical, comparison, mathematical*/

struct Operator {
    char name;
    struct *left;
    struct *right;
};

union {
    struct TokenListNode *identifier;
    struct Operator *operator;
};

#endif

enum {
    /*
     *All the literal number are choosen depending on the space
     *available in the ascii table, that does not interfere with symbols
     *like a-z and A-Z
     */
    TOK_TYPE_OP     = 48,
    TOK_TYPE_SEP    = 49,
    TOK_TYPE_KEY    = 50,
    TOK_TYPE_LI     = 51,
    TOK_TYPE_ID     = 52,

    /*OPerator*/
    TOK_OP_EQUAL    = '=',
    TOK_OP_NOT      = '!',
    TOK_OP_INF      = '<',
    TOK_OP_SUP      = '>',
    TOK_OP_OR       = '|',
    TOK_OP_AND      = '&',
    TOK_OP_XOR      = '^',
    TOK_OP_ADD      = '+',
    TOK_OP_SUB      = '-',
    TOK_OP_BY       = '*',
    TOK_OP_DIV      = '/',
    TOK_OP_MOD      = '%',

    /*SEParator
     *
     * R=round
     * C=curly
     * S=square
     *
     * B=bracket
     * S=start
     * E=end
     */
    TOK_SEP_RBS     = '(',
    TOK_SEP_RBE     = ')',
    TOK_SEP_CBS     = '{',
    TOK_SEP_CBE     = '}',
    TOK_SEP_SBS     = '[',
    TOK_SEP_SBE     = ']',
    TOK_SEP_SEMI    = ';',
    TOK_SEP_COMMA   = ',',
    TOK_SEP_DOT     = '.',
    TOK_SEP_CHAR    = '\'',
    TOK_SEP_STR     = '"',

    /*KEYword*/
    TOK_KEY_FOR     = 65,   /*yes*/
    TOK_KEY_FLOAT   = 66,
    TOK_KEY_IF      = 67,   /*yes*/
    TOK_KEY_INT     = 68,
    TOK_KEY_WHILE   = 69,   /*yes*/
    TOK_KEY_ELSE    = 70,   /*yes*/
    TOK_KEY_BOOL    = 71,
    TOK_KEY_BREAK   = 72,   /*yes*/
    TOK_KEY_CHAR    = 73,
    TOK_KEY_RETURN  = 74,   /*yes*/
    TOK_KEY_STR     = 75,
    TOK_KEY_SWITCH  = 76,
    TOK_KEY_STATIC  = 77,
    TOK_KEY_DEFAULT = 78,
    TOK_KEY_CASE    = 79,
    TOK_KEY_ASSERT  = 80,
    TOK_KEY_ELIF    = 81,   /*yes*/
    TOK_KEY_NEW     = 82,
    TOK_KEY_CLASS   = 83,
    TOK_KEY_FN      = 84,   /*yes*/
    TOK_KEY_VAR     = 85,   /*yes*/

    /*LIteral*/
    TOK_LI_NUMBER   = 97,
    TOK_LI_BOOL     = 98,
    TOK_LI_STRING   = 99,
};

#endif
