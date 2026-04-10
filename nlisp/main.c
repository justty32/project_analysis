#include "eval.h"
#include "parser.h"
#include "tokenizer.h"
#include <stdio.h>
#include <stdlib.h> // For exit()
#include <string.h> // For strcmp

//
int test_print(list_node_t *node, int index) {
    if (strcmp(node->data.type, "list") == 0) {
        printf("(");
        list_foreach((list_node_t *)node->data.ptr, test_print);
        printf(")");
    } else if (strcmp(node->data.type, "atom") == 0) {
        printf(" ");
        printf("%s", (char *)node->data.ptr);
        printf(" ");
    }
    return 0;
}

char *read_file_into_buffer(const char *filename) {
    FILE *file = NULL;
    fopen_s(&file, filename, "rb");
    if (file == NULL) {
        perror("Error opening file");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buffer = (char *)malloc(file_size + 1);
    if (buffer == NULL) {
        perror("Error allocating memory");
        fclose(file);
        return NULL;
    }
    fread(buffer, 1, file_size, file);
    buffer[file_size] = '\0';
    fclose(file);
    return buffer;
}

int main(int argc, char *argv[]) {

    char *file_content_buffer = read_file_into_buffer(argv[1]);
    char **tokens = tokenize(file_content_buffer);
    for (int i = 0; tokens[i] != NULL; i++) {
        // printf("%s\n", tokens[i]);
    }
    list_node_t *list = NULL;
    parse(tokens, &list, 0);
    free_tokens(&tokens);
    free(file_content_buffer);

    list_foreach((list_node_t *)list, test_print);
    data_t a = eval(list);
    if (data_is_type(&a, "list")) {
        list_foreach((list_node_t *)a.ptr, test_print);
    } else {
        printf("%s", (char *)a.ptr);
    }

    // list_foreach(list, deal_not);

    list_free(list);
    printf("\nsdfsdf\n");
    return 0;
}
