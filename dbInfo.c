#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <cjson/cJSON.h>
#include "config.h"

extern void replaceBackslash(char *string);
extern cJSON *openDBFile(char *dbFile);

void baseDBInfoShow()
{
    char dirPath[256] = {};
    char compressedDBFilePath[256] = {};
    char dbFilePath[256] = {};
    char zipCommand[256] = {};
    cJSON *parsedDB;
    cJSON *item;
    int arraySize;
    int i;

    printf("请输入基础数据库文件路径: ");
    fgets(dirPath, sizeof(dirPath), stdin);
    replaceBackslash(dirPath);

    sprintf(compressedDBFilePath, "%s/%s", dirPath, COMPRESSED_BASE_DATABASE_FILE_NAME);

    if (access(compressedDBFilePath, F_OK) != 0)
    {
        printf("基础数据库文件不存在！\n");

        return;
    }

    sprintf(zipCommand, "data\\u.dat -u -q -d %s %s", dirPath, compressedDBFilePath);
    system(zipCommand);

    sprintf(dbFilePath, "%s/%s", dirPath, BASE_DATABASE_FILE_NAME);
    parsedDB = openDBFile(dbFilePath);

    printf("\n");

    printf("┌────────────────────┬───────────────────────────┐\n");
    item = cJSON_GetObjectItem(parsedDB, "vendor");
    printf("│ 厂商               │ %-26s│\n", item->valuestring);

    printf("├────────────────────┼───────────────────────────┤\n");

    item = cJSON_GetObjectItem(parsedDB, "product");
    printf("│ 产品               │ %-26s│\n", item->valuestring);

    printf("├────────────────────┼───────────────────────────┤\n");

    item = cJSON_GetObjectItem(parsedDB, "architecture");
    printf("│ 架构               │ %-26s│\n", item->valuestring);

    printf("├────────────────────┼───────────────────────────┤\n");

    item = cJSON_GetObjectItem(parsedDB, "type");
    printf("│ 类型               │ %-26s│\n", item->valuestring);

    printf("├────────────────────┼───────────────────────────┤\n");

    item = cJSON_GetObjectItem(parsedDB, "create_date");
    printf("│ 基础数据库创建日期 │ %-26s│\n", item->valuestring);

    printf("├────────────────────┼───────────────────────────┤\n");

    item = cJSON_GetObjectItem(parsedDB, "patch_date");
    printf("│ 补丁日期           │ %-26s│\n", item->valuestring);

    printf("├────────────────────┼───────────────────────────┤\n");

    item = cJSON_GetObjectItem(parsedDB, "number_of_updates");
    printf("│ 打补丁次数         │ %-26d│\n", item->valueint);

    printf("├────────────────────┼───────────────────────────┤\n");

    item = cJSON_GetObjectItem(parsedDB, "last_update_date");
    printf("│ 最近打补丁日期     │ %-26s│\n", item->valuestring);

    printf("├────────────────────┼───────────────────────────┤\n");

    item = cJSON_GetObjectItem(parsedDB, "patch_producer");
    printf("│ 补丁制作人         │ %-26s│\n", item->valuestring);

    printf("├────────────────────┼───────────────────────────┤\n");

    item = cJSON_GetObjectItem(parsedDB, "files_recording");
    arraySize = cJSON_GetArraySize(item);
    printf("│ 记录文件数量       │ %-26d│\n", arraySize);

    printf("└────────────────────┴───────────────────────────┘\n");

    item = cJSON_GetObjectItem(parsedDB, "patch_log");
    arraySize = cJSON_GetArraySize(item);

    if (arraySize > 0)
    {
        printf("补丁记录:\n");
        for (i = 0; i < arraySize; i++)
        {
            printf("%s\n", (cJSON_GetArrayItem(item, i)->valuestring));
        }
    }

    cJSON_Delete(parsedDB);

    remove(dbFilePath);
}

void patchDBInfoShow()
{
    char dirPath[256] = {};
    char compressedDBFilePath[256] = {};
    char dbFilePath[256] = {};
    char zipCommand[256] = {};
    cJSON *parsedDB;
    cJSON *item;
    int arraySize;

    printf("请输入补丁数据库文件路径: ");
    fgets(dirPath, sizeof(dirPath), stdin);
    replaceBackslash(dirPath);

    sprintf(compressedDBFilePath, "%s/%s", dirPath, COMPRESSED_PATCH_DATABASE_FILE_NAME);

    if (access(compressedDBFilePath, F_OK) != 0)
    {
        printf("补丁数据库文件不存在！\n");

        return;
    }

    sprintf(zipCommand, "data\\u.dat -u -q -d %s %s", dirPath, compressedDBFilePath);
    system(zipCommand);

    sprintf(dbFilePath, "%s/%s", dirPath, PATCH_DATABASE_FILE_NAME);
    parsedDB = openDBFile(dbFilePath);

    printf("\n");

    item = cJSON_GetObjectItem(parsedDB, "patch_md5");
    printf("补丁文件MD5 ---> [%s]\n", item->valuestring);

    printf("┌────────────────────┬───────────────────────────┐\n");
    item = cJSON_GetObjectItem(parsedDB, "vendor");
    printf("│ 厂商               │ %-26s│\n", item->valuestring);

    printf("├────────────────────┼───────────────────────────┤\n");

    item = cJSON_GetObjectItem(parsedDB, "product");
    printf("│ 产品               │ %-26s│\n", item->valuestring);

    printf("├────────────────────┼───────────────────────────┤\n");

    item = cJSON_GetObjectItem(parsedDB, "architecture");
    printf("│ 架构               │ %-26s│\n", item->valuestring);

    printf("├────────────────────┼───────────────────────────┤\n");

    item = cJSON_GetObjectItem(parsedDB, "type");
    printf("│ 类型               │ %-26s│\n", item->valuestring);

    printf("├────────────────────┼───────────────────────────┤\n");

    item = cJSON_GetObjectItem(parsedDB, "create_date");
    printf("│ 补丁数据库创建日期 │ %-26s│\n", item->valuestring);

    printf("├────────────────────┼───────────────────────────┤\n");

    item = cJSON_GetObjectItem(parsedDB, "producer");
    printf("│ 补丁制作人         │ %-26s│\n", item->valuestring);

    printf("├────────────────────┼───────────────────────────┤\n");

    item = cJSON_GetObjectItem(parsedDB, "files_recording");
    arraySize = cJSON_GetArraySize(item);
    printf("│ 记录文件数量       │ %-26d│\n", arraySize);

    printf("└────────────────────┴───────────────────────────┘\n");

    cJSON_Delete(parsedDB);

    remove(dbFilePath);
}