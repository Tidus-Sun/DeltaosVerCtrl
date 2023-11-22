#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <cjson/cJSON.h>
#include "config.h"

extern void replaceBackslash(char *str);
extern int checkDirValid (char *path);
extern cJSON *openDBFile(char *dbFile);
extern int getBaseIndex (char *string);
extern char *calculateMD5(const char *file_path);
extern char *getFileRecordMd5(cJSON *parsedDB, char *fileName);

void checksumMd5(cJSON *parsedBaseDB, char *recordingStr1, char *recordingStr2, const char *dirPath, int pathStartIndex)
{
    DIR *dir;
    struct dirent *entry;
    struct stat fileInfo;
    char filePath[PATH_MAX];
    char *baseDBMd5;
    char *md5;

    if ((dir = opendir(dirPath)) == NULL)
    {
        perror("Open Dir Error");
        return;
    }

    while ((entry = readdir(dir)) != NULL)
    {
        /* 跳过baseDB.cd和baseDB文件 */
        if (strcmp(COMPRESSED_BASE_DATABASE_FILE_NAME, entry->d_name) == 0 || strcmp(BASE_DATABASE_FILE_NAME, entry->d_name) == 0)
        {
            continue;
        }

        snprintf(filePath, sizeof(filePath), "%s/%s", dirPath, entry->d_name);

        if (stat(filePath, &fileInfo) != 0)
        {
            perror("stat");
            closedir(dir);
            return;
        }

        if (S_ISREG(fileInfo.st_mode))
        {
            // If it's a regular file
            md5 = calculateMD5(filePath);

            baseDBMd5 = getFileRecordMd5(parsedBaseDB, filePath + pathStartIndex);

            if (baseDBMd5 != NULL)
            {
                if (strcmp(baseDBMd5, md5) != 0)
                {
                    printf("发现变化文件: %s\n", filePath + pathStartIndex);

                    /* 记录md5与基础数据库不符的文件路径 */
                    strcat(recordingStr1, filePath + pathStartIndex);
                    strcat(recordingStr1, "\n");
                }
            }
            else
            {
                printf("发现未记录文件: %s\n", filePath + pathStartIndex);

                /* 记录基础数据库中不存在的文件路径 */
                strcat(recordingStr2, filePath + pathStartIndex);
                strcat(recordingStr2, "\n");
            }


            free(md5);
        }
        else if (S_ISDIR(fileInfo.st_mode) && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
        {
            checksumMd5(parsedBaseDB, recordingStr1, recordingStr2, filePath, pathStartIndex); // Recursively traverse directories
        }
    }

    closedir(dir);
}

void fileChecksum()
{
    char dirPath[256] = {};
    char initialPath[256] = {};
    char compressedDBFilePath[256] = {};
    char command[256] = {};
    char baseDBFilePath[256] = {};
    cJSON *parsedBaseDB;
    void *recordingStr1 = NULL;
    void *recordingStr2 = NULL;
    int dirBaseIndex;
    FILE *file;
    int ret;

    getcwd(initialPath, sizeof(initialPath));

    printf("请输入开发环境根目录路径: ");
    fgets(dirPath, sizeof(dirPath), stdin);
    replaceBackslash(dirPath);

    strcat(dirPath, "/deltaos6.2");
    if (checkDirValid(dirPath) != 0)
    {
        printf("开发环境目录错误，请确认后再进行校验!\n");

        return;
    }

    /*设置compressedDBFilePath为开发环境根目录中基础数据库的路径*/
    sprintf(compressedDBFilePath, "%s/%s", dirPath, COMPRESSED_BASE_DATABASE_FILE_NAME);

    if (access(compressedDBFilePath, F_OK) != 0)
    {
        printf("基础数据库文件不存在！\n");

        return;
    }

    /* 计算用于记录文件路径的起始索引号 */
    dirBaseIndex = getBaseIndex(dirPath) + 1;

    /*解压缩开发环境中的基础数据库*/
    sprintf(command, "data\\u.dat -o -q -d %s %s", dirPath, compressedDBFilePath);
    system(command);

    sprintf(baseDBFilePath, "%s/%s", dirPath, BASE_DATABASE_FILE_NAME);
    parsedBaseDB = openDBFile(baseDBFilePath);

    /* 申请空间用于记录md5与数据库不符的文件 */
    recordingStr1 = malloc(STRING_SIZE);
    memset(recordingStr1, 0, STRING_SIZE);
    /* 申请空间用于记录数据库中未记录的文件 */
    recordingStr2 = malloc(STRING_SIZE);
    memset(recordingStr2, 0, STRING_SIZE);

    printf("校验中...\n");

    checksumMd5(parsedBaseDB, recordingStr1, recordingStr2, dirPath, dirBaseIndex);

    ret = chdir("Result");
    if (ret != 0)
    {
        printf("访问Result目录出错，退出文件校验!\n");
        free(recordingStr1);
        free(recordingStr2);
        cJSON_Delete(parsedBaseDB);

        return;
    }

    if (*(char *)recordingStr1 != '\0' || *(char *)recordingStr2 != '\0')
    {
        file = fopen(CHECKSUM_FILE_NAME, "w");
        if (file)
        {
            if (*(char *)recordingStr1 != '\0')
            {
                fputs("与基础数据库不同的文件:\n", file);
                fputs(recordingStr1, file);

                fputs("\n", file);
            }

            if (*(char *)recordingStr2 != '\0')
            {
                fputs("基础数据库未记录的文件:\n", file);
                fputs(recordingStr2, file);
            }

            fclose(file);

            sprintf(command, "start %s", CHECKSUM_FILE_NAME);
            system(command);
        }
    }
    else
    {
        printf("开发环境中所有文件均与数据库中一致!\n");
    }

    chdir(initialPath);

    remove(baseDBFilePath);
    free(recordingStr1);
    free(recordingStr2);
    cJSON_Delete(parsedBaseDB);

    printf("校验完成!\n");
}