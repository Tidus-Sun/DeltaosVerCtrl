#include <stdio.h>
#include <stdlib.h>
#include <cjson/cJson.h>
#include <unistd.h>
#include "config.h"

extern int getBaseIndex (char *string);
extern int checkDirValid (char *path);
extern void getCurrentDateAsString(char *dateString);
extern cJSON *getDBFilesRecording(cJSON *parsedDB);
extern void writeDB(char *filePath, cJSON *parsedDB);
extern void recordMd5(cJSON *dbFilesRecording, char *dirPath, int pathStartIndex);
extern void replaceBackslash(char *str);
extern cJSON *openDBFile(char *dbFile);

void createDBFile(char *filename, char *vendor, char *product, char *architecture, char *date)
{
    cJSON *patchRecord;
    cJSON *filesRecording;
    char *jsonString;
    FILE *file;

    // 创建一个JSON对象
    cJSON *root = cJSON_CreateObject();

    // 向JSON对象中添加键值对
    cJSON_AddItemToObject(root, "vendor", cJSON_CreateString(vendor));
    cJSON_AddItemToObject(root, "product", cJSON_CreateString(product));
    cJSON_AddItemToObject(root, "architecture", cJSON_CreateString(architecture));
    cJSON_AddItemToObject(root, "type", cJSON_CreateString("Base DB"));
    cJSON_AddItemToObject(root, "create_date", cJSON_CreateString(date));
    cJSON_AddItemToObject(root, "patch_date", cJSON_CreateString(""));
    cJSON_AddItemToObject(root, "number_of_updates", cJSON_CreateNumber(0));
    cJSON_AddItemToObject(root, "last_update_date", cJSON_CreateString(""));
    cJSON_AddItemToObject(root, "patch_producer", cJSON_CreateString(""));

    patchRecord = cJSON_CreateArray();
    cJSON_AddItemToObject(root, "patch_log", patchRecord);

    // 创建包含文件记录的数组
    filesRecording = cJSON_CreateArray();
    cJSON_AddItemToObject(root, "files_recording", filesRecording);

    // 将JSON对象打印为字符串
    jsonString = cJSON_Print(root);
    printf("Generated Files Database: %s\n", jsonString);

    // 将JSON对象写入文件
    file = fopen(filename, "w");
    if (file)
    {
        fputs(jsonString, file);
        fclose(file);
    }

    // 释放内存
    cJSON_Delete(root);
    free(jsonString);
}

void makeBaseDatabase(char *filename)
{
    char vendor[16] = {};
    char product[16] = {};
    char architecture[16] = {};
    char date[9] = {};
    char dirPath[256] = {};
    cJSON *parsedDB;
    cJSON *dbFilesRecording;
    char zipCommand[256] = {};
    char initialPath[256] = {};
    int dirBaseIndex;
    int ret;

    getcwd(initialPath, sizeof(initialPath));

    printf("请输入厂商信息: ");
    fgets(vendor, sizeof(vendor), stdin);
    vendor[strcspn(vendor, "\n")] = '\0';

    printf("请输入产品信息: ");
    fgets(product, sizeof(product), stdin);
    product[strcspn(product, "\n")] = '\0';

    printf("请输入架构信息: ");
    fgets(architecture, sizeof(architecture), stdin);
    architecture[strcspn(architecture, "\n")] = '\0';

    getCurrentDateAsString(date);

    ret = chdir("Result");
    if (ret != 0)
    {
        printf("访问Result目录出错，退出基础数据库制作!\n");
        return;
    }

    createDBFile(filename, vendor, product, architecture, date);

    printf("请输入开发环境路径: ");
    fgets(dirPath, sizeof(dirPath), stdin);
    replaceBackslash(dirPath);

    strcat(dirPath, "/deltaos6.2");
    if (checkDirValid(dirPath) != 0)
    {
        printf("开发环境目录错误，请确认后再进行制作!\n");
        remove(filename);
        return;
    }

    /* 计算用于记录文件路径的起始索引号 */
    dirBaseIndex = getBaseIndex(dirPath) + 1;

    printf("基础数据库制作中...");

    parsedDB = openDBFile(filename);
    dbFilesRecording = getDBFilesRecording(parsedDB);
    recordMd5(dbFilesRecording, dirPath, dirBaseIndex);
    writeDB(filename, parsedDB);

    sprintf(zipCommand, "%s\\data\\z.dat %s -r -q %s", initialPath, COMPRESSED_BASE_DATABASE_FILE_NAME, filename);
    system(zipCommand);
    remove(filename);

    chdir(initialPath);

    printf("完成!\n");

    system("explorer.exe Result");
}