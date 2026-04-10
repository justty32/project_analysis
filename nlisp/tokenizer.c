#include "tokenizer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STATE_START 1
#define STATE_ATOM 2
#define STATE_COMMENT 3
#define STATE_STRING 4

static const char *TOKEN_L_PAREN = "(";
static const char *TOKEN_R_PAREN = ")";

int is_space(char c) { return c == ' ' || c == '\t' || c == '\n' || c == '\r'; }

int is_atom_stop(char c) {
  return is_space(c) || c == '(' || c == ')' || c == ';' || c == '"';
}

void free_tokens(char ***tokens) {
  for (size_t i = 0; (*tokens)[i] != NULL; i++) {
    if ((*tokens)[i] == NULL)
      continue;
    if ((*tokens)[i] == TOKEN_L_PAREN || (*tokens)[i] == TOKEN_R_PAREN)
      continue;
    free((*tokens)[i]);
  }
  free(*tokens);
  *tokens = NULL;
}

void push_token(char *token, char ***tokens, size_t *tokens_cap,
                size_t *tokens_count) {
  (*tokens)[*tokens_count] = token;
  (*tokens_count)++;
  if (*tokens_count >= *tokens_cap) {
    (*tokens_cap) *= 2;
    *tokens = realloc(*tokens, sizeof(char *) * (*tokens_cap));
  }
  if (*tokens == NULL)
    perror("push_token failed: realloc failed\n");
}

char *duplicate_token(const char *str) {
  size_t len = strlen(str);
  char *dup = (char *)malloc(len + 1);
  strcpy(dup, str);
  return dup;
}

char *make_token(char *start, char *end) {
  char *atom = (char *)malloc(end - start + 1);
  for (int i = 0; start + i != end; i++) {
    atom[i] = *(start + i);
  }
  atom[end - start] = '\0';
  return atom;
}

char *make_token_string(char *start, char *end) {
  char *atom = (char *)malloc(end - start + 1);
  int ai = 0;
  for (int i = 0; start + i != end; i++) {
    char c = *(start + i);
    if (c == '\\' && start + i + 1 < end) {
      char next_c = *(start + i + 1);
      switch (next_c) {
      case 'n':
        atom[ai] = '\n';
        break;
      case 't':
        atom[ai] = '\t';
        break;
      case 'r':
        atom[ai] = '\r';
        break;
      case '\\':
        atom[ai] = '\\';
        break;
      case '"':
        atom[ai] = '"';
        break;
      default:
        atom[ai] = next_c;
        break;
      }
      i++;
    } else {
      atom[ai] = c;
    }
    ai++;
  }
  atom[ai] = '\0';
  atom = realloc(atom, ai + 1);
  return atom;
}

char **tokenize(const char *str) {
  int state = STATE_START;
  size_t tokens_cap = 16;
  size_t tokens_count = 0;
  char **tokens = (char **)malloc(sizeof(char *) * tokens_cap);
  char *read = (char *)str;
  char *token_start = read;
  while (*read != '\0') {
    switch (state) {
    case STATE_START:
      if (*read == ';') {
        state = STATE_COMMENT;
      } else if (*read == '"') {
        token_start = read;
        state = STATE_STRING;
      } else if (is_space(*read)) {
      } else if (*read == '(') {
        push_token((char *)TOKEN_L_PAREN, &tokens, &tokens_cap, &tokens_count);
      } else if (*read == ')') {
        push_token((char *)TOKEN_R_PAREN, &tokens, &tokens_cap, &tokens_count);
      } else {
        token_start = read;
        state = STATE_ATOM;
      }
      break;
    case STATE_ATOM:
      if (is_atom_stop(*read)) {
        char *token = make_token(token_start, read);
        push_token(token, &tokens, &tokens_cap, &tokens_count);
        state = STATE_START;
        read--;
      }
      break;
    case STATE_STRING:
      if (*read == '"') {
        if (read > str && *(read - 1) == '\\') {
          read++;
        } else {
          char *token = make_token_string(token_start, read + 1);
          push_token(token, &tokens, &tokens_cap, &tokens_count);
          state = STATE_START;
        }
      }
      break;
    case STATE_COMMENT:
      if (*read == '\n') {
        state = STATE_START;
      }
      break;
    }
    read++;
  }
  if (state == STATE_STRING) {
    char *token = make_token(token_start, read);
    token = realloc(token, read - token_start + 1);
    token[read - token_start - 1] = '"';
    token[read - token_start] = '\0';
    push_token(token, &tokens, &tokens_cap, &tokens_count);
  } else if (state == STATE_ATOM) {
    if (read > token_start) {
      char *token = make_token(token_start, read);
      push_token(token, &tokens, &tokens_cap, &tokens_count);
    }
  }
  push_token(NULL, &tokens, &tokens_cap, &tokens_count);
  tokens = realloc(tokens, sizeof(char *) * tokens_count);
  return tokens;
}
