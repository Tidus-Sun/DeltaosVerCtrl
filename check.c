#include <stdio.h>
#include <unistd.h>
#include "config.h"

int toolIntegrityCheck()
{
    int zipFlag;
    int unzipFlag;

    zipFlag = access("data\\z.dat", F_OK);
    unzipFlag = access("data\\u.dat", F_OK);

    if (zipFlag != 0 || unzipFlag != 0)
    {
        return -1;
    }

    if (access("Result", F_OK) != 0)
    {
        mkdir("Result");
        printf("缺少Result文件夹，已自动创建...\n\n");
    }

    if (access("Patch", F_OK) != 0)
    {
        mkdir("Patch");
        printf("缺少Patch文件夹，已自动创建...\n\n");
    }

    return 0;
}

int checkDirValid (char *path)
{
    return (access(path, F_OK));
}

int patchIntegrityCheck()
{
    int zipFlag;
    int unzipFlag;
    char patchFile[256] = "Patch/";

    zipFlag = access("data\\z.dat", F_OK);
    unzipFlag = access("data\\u.dat", F_OK);

    if (zipFlag != 0 || unzipFlag != 0)
    {
        return -1;
    }

    strcat(patchFile, COMPRESSED_PATCH_DATABASE_FILE_NAME);
    if (access(patchFile, F_OK) != 0)
    {
        printf("缺少补丁数据库,无法打补丁!\n");
        return -1;
    }

    patchFile[6] = '\0';
    strcat(patchFile, COMPRESSED_PATCH_FILE_NAME);
    if (access(patchFile, F_OK) != 0)
    {
        printf("缺少补丁文件,无法打补丁!\n\n");
        return -1;
    }

    return 0;
}