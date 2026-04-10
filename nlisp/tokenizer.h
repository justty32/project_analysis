#ifndef NLISP
#define NLISP

/*
int is_space(char c);
int is_atom_stop(char c);
void push_token(char *token, char ***tokens, size_t *tokens_cap, size_t *tokens_count);
char* duplicate_token(const char* str);
char *make_token(char *start, char *end);
char *make_token_string(char *start, char *end);
*/

// return tokens
// tokens: char[], end with '\0'
char **tokenize(const char *str);

// use this to release tokens
void free_tokens(char ***tokens);

#endif // NLISP