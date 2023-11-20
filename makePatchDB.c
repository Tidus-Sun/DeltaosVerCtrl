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
extern char *calculateMD5(const char *file_path);

void createPatchFile(char *filename, char *vendor, char *product, char *architecture, char *date, char *producer)
{
    cJSON *filesRecording;
    FILE *file;
    char *jsonString;

    // 创建一个JSON对象
    cJSON *root = cJSON_CreateObject();

    // 向JSON对象中添加键值对
    cJSON_AddItemToObject(root, "vendor", cJSON_CreateString(vendor));
    cJSON_AddItemToObject(root, "product", cJSON_CreateString(product));
    cJSON_AddItemToObject(root, "architecture", cJSON_CreateString(architecture));
    cJSON_AddItemToObject(root, "type", cJSON_CreateString("Patch DB"));
    cJSON_AddItemToObject(root, "create_date", cJSON_CreateString(date));
    cJSON_AddItemToObject(root, "producer", cJSON_CreateString(producer));

    // 创建包含文件记录的数组
    filesRecording = cJSON_CreateArray();
    cJSON_AddItemToObject(root, "files_recording", filesRecording);

    // 将JSON对象打印为字符串
    jsonString = cJSON_Print(root);
    //printf("Generated Files Database: %s\n", jsonString);

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

void makePatchDatabase(char *filename)
{
    char vendor[16] = {};
    char product[16] = {};
    char architecture[16] = {};
    char date[9] = {};
    char producer[32] = {};
    char dirPath[256] = {};
    cJSON *parsedDB;
    cJSON *dbFilesRecording;
    char zipCommand[256] = {};
    char initialPath[256] = {};
    char *md5;
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

    printf("请输入补丁制作人姓名(英文): ");
    fgets(producer, sizeof(producer), stdin);
    producer[strcspn(producer, "\n")] = '\0';

    getCurrentDateAsString(date);

    ret = chdir("Result");
    if (ret != 0)
    {
        printf("访问Result目录出错，退出补丁制作!\n");
        chdir(initialPath);
        return;
    }

    /* 删除已存在的patch.zip补丁文件 */
    if (access(COMPRESSED_PATCH_FILE_NAME, F_OK) == 0)
    {
        remove(COMPRESSED_PATCH_FILE_NAME);
    }

    createPatchFile(filename, vendor, product, architecture, date, producer);

    printf("请输入补丁路径: ");
    fgets(dirPath, sizeof(dirPath), stdin);
    replaceBackslash(dirPath);

    strcat(dirPath, "/deltaos6.2");
    if (checkDirValid(dirPath) != 0)
    {
        printf("补丁目录错误，请确认后再进行制作!\n");
        remove(filename);
        return;
    }

    /* 计算用于记录文件路径的起始索引号 */
    dirBaseIndex = getBaseIndex(dirPath) + 1;

    printf("补丁文件压缩中...");
    chdir(dirPath);
    chdir("..");

    sprintf(zipCommand, "%s\\data\\z.dat -r -q %s\\Result\\%s deltaos6.2 -x \"*/%s\"", initialPath, initialPath, COMPRESSED_PATCH_FILE_NAME, COMPRESSED_BASE_DATABASE_FILE_NAME);
    system(zipCommand);
    printf("完成!\n");

    printf("补丁数据库制作中...\n");

    chdir(initialPath);
    chdir("Result");

    md5 = calculateMD5(COMPRESSED_PATCH_FILE_NAME);

    parsedDB = openDBFile(filename);
    dbFilesRecording = getDBFilesRecording(parsedDB);
    recordMd5(dbFilesRecording, dirPath, dirBaseIndex);
    cJSON_AddItemToObject(parsedDB, "patch_md5", cJSON_CreateString(md5));
    free(md5);

    writeDB(filename, parsedDB);

    sprintf(zipCommand, "%s\\data\\z.dat -r -q %s %s", initialPath, COMPRESSED_PATCH_DATABASE_FILE_NAME, filename);
    system(zipCommand);
    remove(filename);
    printf("完成!\n");

    chdir(initialPath);
    system("explorer.exe Result");
}