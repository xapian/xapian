
#include "base64.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char ** argv)
{
    int encode = 0;
    
    if (argc < 3) {
        printf("usage: %s [-d|-e] data\n", argv[0]);
        return EXIT_FAILURE;
    }

    if (strcmp(argv[1], "-d") == 0) {
        encode = 0;
    } else if (strcmp(argv[1], "-e") == 0) {
        encode = 1;
    } else {
        fprintf(stderr, "expected -d or -e\n");
        return EXIT_FAILURE;
    }

    if (encode) {
        int inlen = strlen(argv[2]);
        int outlen;
        char * outbuf = malloc(base64_encode_size(inlen));
        outlen = base64_encode(argv[2], inlen, outbuf);
        printf("Encoded form, length %d, is `%s'\n", outlen, outbuf);
    } else {
        int inlen = strlen(argv[2]);
        int outlen;
        char * outbuf = malloc(base64_decode_size(inlen));
        outlen = base64_decode(argv[2], inlen, outbuf);
        printf("Decoded form, length %d, is `%s'\n", outlen, outbuf);
    }

    return EXIT_SUCCESS;
}
