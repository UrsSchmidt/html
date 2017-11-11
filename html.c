#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* multi-purpose buffer size */
#define BUFFER_MAX 50

/* usage: wget -qO- 'http://www.ursbjoernschmidt.de/' | html | more */
int main(int argc, char **argv) {
    enum {
        NORMAL,
        ENTITY,
        TAG_NAME,
        TAG_ATTR,
    } mode = NORMAL;
    /* current content of c */
    int c;
    /* last content of c */
    int lastc = 0;
    /* multi-purpose buffer */
    char buffer[BUFFER_MAX];
    /* multi-purpose buffer pointer */
    int ptr = 0;
    /* are we inside <body></body>? */
    bool inbody = false;
    /* do we have to collapse whitespace? */
    bool collapse = true;
    /* was last printed character \n? */
    bool lastwasnl = true;
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
                            putchar(' ');
                            lastwasnl = false;
                        }
                    } else {
                        putchar(c);
                        lastwasnl = (c == '\n');
                    }
                }
                break;
            case '&':
                mode = ENTITY;
                break;
            case '<':
                mode = TAG_NAME;
                break;
            default:
                if (inbody) {
                    putchar(c);
                    lastwasnl = false;
                }
            }
            break;
        case ENTITY:
            switch (c) {
            case ';':
                if (ptr >= BUFFER_MAX)
                    return EXIT_FAILURE;
                buffer[ptr] = 0;
                // printf("&%s;\n", buffer);
                if (inbody) {
                    if (buffer[0] == 0) {
                        return EXIT_FAILURE;
                    } else if (!strcmp(buffer, "amp")) {
                        putchar('&');
                    } else if (!strcmp(buffer, "apos")) {
                        putchar('\'');
                    } else if (!strcmp(buffer, "gt")) {
                        putchar('>');
                    } else if (!strcmp(buffer, "lt")) {
                        putchar('<');
                    } else if (!strcmp(buffer, "quot")) {
                        putchar('"');
                    } else if (buffer[0] == '#') {
                        if (buffer[1] == 0) {
                            return EXIT_FAILURE;
                        } else if (buffer[1] == 'x') {
                            /* TODO &#x123; */
                        } else {
                            putchar(atoi(&buffer[1]));
                        }
                    } else {
                        /* TODO more */
                    }
                    lastwasnl = false;
                }
                ptr = 0;
                mode = NORMAL;
                break;
            default:
                if (ptr >= BUFFER_MAX)
                    return EXIT_FAILURE;
                buffer[ptr++] = c;
            }
            break;
        case TAG_NAME:
        case TAG_ATTR:
            switch (c) {
            case '>':
                if (ptr >= BUFFER_MAX)
                    return EXIT_FAILURE;
                buffer[ptr] = 0;
                // printf("<%s>\n", buffer);
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
                            putchar('\n');
                            lastwasnl = true;
                        }
                    }
                } else if (!strcmp(buffer, "code") ||
                           !strcmp(buffer, "pre")) {
                    collapse = false;
                } else if (!strcmp(buffer, "/code") ||
                           !strcmp(buffer, "/pre")) {
                    if (inbody) {
                        putchar('\n');
                        lastwasnl = true;
                    }
                    collapse = true;
                } else if (!strcmp(buffer, "hr")) {
                    if (inbody) {
                        if (!lastwasnl)
                            putchar('\n');
                        printf("-------------------------\n");
                        lastwasnl = true;
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
                        mode = TAG_ATTR;
                    }
                }
            }
            break;
        }
        lastc = c;
    }
    if (!lastwasnl)
        putchar('\n');
    return EXIT_SUCCESS;
}
