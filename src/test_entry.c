#include <pool/log.h>
#include <lexer/lexer.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

char* read_file(const char* filename, uint64_t* out_len) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Failed to open file");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buffer = (char*)malloc(length + 1);
    if (!buffer) {
        perror("Failed to allocate buffer");
        fclose(file);
        return NULL;
    }
    fread(buffer, 1, length, file);
    buffer[length] = '\0';

    fclose(file);

    *out_len = (unsigned)length;
    return buffer;
}

void lex_file_from_disk(const char* filename) {
    uint64_t len;
    char *buffer = read_file(filename, &len);
    if (!buffer) {
        printf("Failed to read file\n");
        return;
    }

    struct Lexer *lexer = lexer_create(buffer, len);
    lexer_parse(lexer);
    __lexer__dump_tokens(lexer);
    __lexer_dump_macros(lexer);

    free(buffer);
}

int __test_main(void) {
    lex_file_from_disk("test/lxt0.gal");
    return 0;
}