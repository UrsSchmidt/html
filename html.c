#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#define DEBUG 1

#define BUFFER_MAX 50
#define BUFFER_ENTITY_MAX 50

enum {
    NORMAL,
    TAG_NAME,
    TAG_ATTR,
} mode = NORMAL;

/* was last printed character \n? */
bool lastwasnl = true;

void emit(char c) {
#ifdef DEBUG
    fprintf(stderr, "emit: %c\n", c);
#endif
    putchar(c);
    lastwasnl = (c == '\n');
}

void emit_nl() {
#ifdef DEBUG
    fprintf(stderr, "emit: newline\n");
#endif
    putchar('\n');
    lastwasnl = true;
}

bool read_entity(bool inbody) {
#ifdef DEBUG
    fprintf(stderr, "read_entity(%s);\n", inbody ? "true" : "false");
#endif
    char buffer[BUFFER_ENTITY_MAX];
    int ptr = 0;
    int c;
    while ((c = getchar()) != EOF) {
        switch (c) {
        case '\t':
        case '\n':
        case '\v':
        case '\f':
        case '\r':
        case ' ':
        case ';':
            if (ptr >= BUFFER_ENTITY_MAX)
                return false;
            buffer[ptr] = 0;
#ifdef DEBUG
            fprintf(stderr, "&%s;\n", buffer);
#endif
            if (inbody) {
                if (buffer[0] == 0) {
                    return false;
                } else if (!strcmp(buffer, "amp")) {
                    emit('&');
                } else if (!strcmp(buffer, "apos")) {
                    emit('\'');
                } else if (!strcmp(buffer, "gt")) {
                    emit('>');
                } else if (!strcmp(buffer, "lt")) {
                    emit('<');
                } else if (!strcmp(buffer, "nbsp")) {
                    emit(' ');
                } else if (!strcmp(buffer, "quot")) {
                    emit('"');
                } else if (buffer[0] == '#') {
                    if (buffer[1] == 0) {
                        return false;
                    } else if (buffer[1] == 'x') {
                        /* TODO &#x123; */
                    } else {
                        emit(atoi(&buffer[1]));
                    }
                } else {
                    /* TODO more */
                }
            }
            mode = NORMAL;
            return true;
        default:
            if (ptr >= BUFFER_ENTITY_MAX)
                return false;
            buffer[ptr++] = c;
        }
    }
    return false;
}

bool read_comment() {
#ifdef DEBUG
    fprintf(stderr, "read_comment();\n");
#endif
    int c;
    while ((c = getchar()) != EOF) {
        if (c == '-') {
            if (getchar() == '-') {
                if (getchar() == '>') {
                    mode = NORMAL;
                    return true;
                }
            }
        }
    }
    return false;
}

/* usage: wget -qO- 'http://www.ursbjoernschmidt.de/' | html | more */
int main(int argc, char **argv) {
    int c;
    int lastc = 0;
    char buffer[BUFFER_MAX];
    int ptr = 0;
    /* are we inside <body></body>? */
    bool inbody = false;
    /* do we have to collapse whitespace? */
    bool collapse = true;
    while ((c = getchar()) != EOF) {
        switch (mode) {
        case NORMAL:
            switch (c) {
            case '\t':
            case '\n':
            case '\v':
            case '\f':
            case '\r':
            case ' ':
                if (inbody) {
                    if (collapse) {
                        if (lastc != '\t' && lastc != '\n' &&
                            lastc != '\v' && lastc != '\f' &&
                            lastc != '\r' && lastc != ' ' &&
                            lastc != '>') {
                            emit(' ');
                        }
                    } else {
                        emit(c);
                    }
                }
                break;
            case '&':
                if (!read_entity(inbody))
                    return EXIT_FAILURE;
                break;
            case '<':
                mode = TAG_NAME;
                break;
            default:
                if (inbody) {
                    emit(c);
                }
            }
            break;
        case TAG_NAME:
        case TAG_ATTR:
            switch (c) {
            case '>':
                if (ptr >= BUFFER_MAX)
                    return EXIT_FAILURE;
                buffer[ptr] = 0;
#ifdef DEBUG
                fprintf(stderr, "<%s>\n", buffer);
#endif
                if (!strcmp(buffer, "a")) {
                    // TODO print href
                    // printf("<%s> ", href);
                } else if (!strcmp(buffer, "body")) {
                    inbody = true;
                } else if (!strcmp(buffer, "/body")) {
                    inbody = false;
                } else if (!strcmp(buffer, "br") ||
                           !strcmp(buffer, "h1") || !strcmp(buffer, "/h1") ||
                           !strcmp(buffer, "h2") || !strcmp(buffer, "/h2") ||
                           !strcmp(buffer, "h3") || !strcmp(buffer, "/h3") ||
                           !strcmp(buffer, "h4") || !strcmp(buffer, "/h4") ||
                           !strcmp(buffer, "h5") || !strcmp(buffer, "/h5") ||
                           !strcmp(buffer, "h6") || !strcmp(buffer, "/h6") ||
                           !strcmp(buffer, "li") || !strcmp(buffer, "/li") ||
                           !strcmp(buffer, "p") || !strcmp(buffer, "/p")) {
                    if (inbody) {
                        if (!strcmp(buffer, "br") ||
                            (buffer[0] != '/' && !lastwasnl) ||
                            buffer[0] == '/') {
                            emit_nl();
                        }
                    }
                } else if (!strcmp(buffer, "code") ||
                           !strcmp(buffer, "pre")) {
                    collapse = false;
                } else if (!strcmp(buffer, "/code") ||
                           !strcmp(buffer, "/pre")) {
                    if (inbody) {
                        emit_nl();
                    }
                    collapse = true;
                } else if (!strcmp(buffer, "hr")) {
                    if (inbody) {
                        if (!lastwasnl)
                            emit_nl();
                        for (int i = 0; i < 25; i++)
                            emit('-');
                        emit_nl();
                    }
                }
                ptr = 0;
                mode = NORMAL;
                break;
            default:
                if (mode == TAG_NAME) {
                    if (c == '!' || c == '-' || c == '.' || c == '/' ||
                        ('0' <= c && c <= '9') || c == ':' || c == '?' ||
                        ('A' <= c && c <= 'Z') || c == '_' ||
                        ('a' <= c && c <= 'z')) {
                        if (ptr >= BUFFER_MAX)
                            return EXIT_FAILURE;
                        buffer[ptr++] = c;
                    } else {
                        buffer[ptr] = 0;
                        if (!strcmp(buffer, "!--")) {
                            if (!read_comment())
                                return EXIT_FAILURE;
                            ptr = 0;
                        } else {
                            mode = TAG_ATTR;
                        }
                    }
                }
            }
            break;
        }
        lastc = c;
    }
    if (!lastwasnl)
        emit_nl();
    return EXIT_SUCCESS;
}
