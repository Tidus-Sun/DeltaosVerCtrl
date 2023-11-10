#include <openssl/evp.h>

#define BUFFER_SIZE 1024

char *calculateMD5(const char *file_path)
{
    FILE *file;
    EVP_MD_CTX *mdCtx;
    const EVP_MD *md;
    unsigned char buffer[BUFFER_SIZE];
    unsigned char digest[EVP_MAX_MD_SIZE];
    size_t bytes;
    char *md5_string = (char *) malloc(2 * EVP_MAX_MD_SIZE + 1); // Each byte of MD5 is represented by 2 characters + 1 for null terminator

    OpenSSL_add_all_algorithms();

    file = fopen(file_path, "rb");
    if (file == NULL)
    {
        printf("Error opening file %s.\n", file_path);
        free(md5_string);
        return NULL;
    }

    md = EVP_get_digestbyname("md5");
    if (md == NULL)
    {
        printf("Error: md5 not supported.\n");
        free(md5_string);
        return NULL;
    }

    mdCtx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(mdCtx, md, NULL);

    while ((bytes = fread(buffer, 1, BUFFER_SIZE, file)) != 0)
    {
        EVP_DigestUpdate(mdCtx, buffer, bytes);
    }

    EVP_DigestFinal_ex(mdCtx, digest, NULL);
    EVP_MD_CTX_free(mdCtx);

    fclose(file);

    // Format the MD5 digest as a string
    for (int i = 0; i < EVP_MD_size(md); i++)
    {
        snprintf(md5_string + 2 * i, 3, "%02x", digest[i]);
    }
    md5_string[2 * EVP_MD_size(md)] = '\0'; // Null-terminate the string

    return md5_string;
}