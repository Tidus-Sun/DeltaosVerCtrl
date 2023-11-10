#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <cjson/cJSON.h>
#include "config.h"

extern void replaceBackslash(char *str);
extern int patchIntegrityCheck();
extern int checkDirValid (char *path);
extern char *calculateMD5(const char *file_path);
extern cJSON *openDBFile(char *dbFile);
extern void writeDB(char *filePath, cJSON *parsedDB);
extern void getCurrentDateAsString(char *dateString);
extern int getFileRecordIndex(cJSON *parsedDB, char *fileName);
extern void updateFileRecordMd5(cJSON *parsedDB, int index, char *newMd5);
extern cJSON *getDBFilesRecording(cJSON *parsedDB);
extern void insertFileRecord(cJSON *dbFilesRecording, const char *filename, const char *md5);

void applyPatch()
{
    int ret;
    char dirPath[256] = {};
    char dirPathBackup[256] = {};
    char compressedDBFilePath[256] = {};
    char compressedPatchDBPath[32] = "Patch/";
    char compressedPatchFilePath[32] = "Patch/";
    char baseDBFilePath[256] = {};
    char patchFilePath[32] = "Patch/";
    char command[256] = {};
    cJSON *parsedBaseDB;
    cJSON *parsedPatchDB;
    cJSON *baseDBItem;
    cJSON *patchDBItem;
    char *md5;
    char date[9] = {};
    cJSON *patchFileRecordItem;
    cJSON *itemFilename;
    cJSON *itemMd5;
    int arraySize;
    int i;
    int index;
    char initialPath[256] = {};

    ret = patchIntegrityCheck();
    if (ret != 0)
    {
        return;
    }

    printf("请输入开发环境根目录路径: ");
    fgets(dirPath, sizeof(dirPath), stdin);
    strncpy(dirPathBackup, dirPath, strlen(dirPath) - 1);
    strcat(dirPathBackup, "\\deltaos6.2");
    replaceBackslash(dirPath);

    strcat(dirPath, "/deltaos6.2");
    if (checkDirValid(dirPath) != 0)
    {
        printf("开发环境目录错误，请确认后再进行补丁升级!\n");

        return;
    }

    /*设置compressedDBFilePath为开发环境根目录中基础数据库的路径*/
    sprintf(compressedDBFilePath, "%s/%s", dirPath, COMPRESSED_BASE_DATABASE_FILE_NAME);

    if (access(compressedDBFilePath, F_OK) != 0)
    {
        printf("基础数据库文件不存在！\n");

        return;
    }

    /*解压缩Patch目录中的补丁数据库*/
    strcat(compressedPatchDBPath, COMPRESSED_PATCH_DATABASE_FILE_NAME);
    strcat(compressedPatchFilePath, COMPRESSED_PATCH_FILE_NAME);
    sprintf(command, "data\\u.dat -o -q -d %s %s", "Patch", compressedPatchDBPath);
    system(command);

    strcat(patchFilePath, PATCH_DATABASE_FILE_NAME);
    parsedPatchDB = openDBFile(patchFilePath);

    /* 检查Patch.zip文件的MD5与补丁数据库是否符合 */
    md5 = calculateMD5(compressedPatchFilePath);
    patchDBItem = cJSON_GetObjectItem(parsedPatchDB, "patch_md5");
    if (strcmp(md5, patchDBItem->valuestring) != 0)
    {
        printf("%s的MD5与补丁数据库不符, 请确认版本！\n", COMPRESSED_PATCH_FILE_NAME);

        free(md5);
        cJSON_Delete(parsedPatchDB);
        remove(patchFilePath);

        return;
    }


    /*解压缩开发环境中的基础数据库*/
    sprintf(command, "data\\u.dat -o -q -d %s %s", dirPath, compressedDBFilePath);
    system(command);

    /*解压缩Patch目录中的Patch.zip到Patch目录*/
    sprintf(command, "data\\u.dat -o -q -d %s %s", "Patch", compressedPatchFilePath);
    system(command);

    sprintf(baseDBFilePath, "%s/%s", dirPath, BASE_DATABASE_FILE_NAME);
    parsedBaseDB = openDBFile(baseDBFilePath);


    baseDBItem = cJSON_GetObjectItem(parsedBaseDB, "vendor");
    patchDBItem = cJSON_GetObjectItem(parsedPatchDB, "vendor");

    if (strcmp(baseDBItem->valuestring, patchDBItem->valuestring) != 0)
    {
        printf("补丁厂商信息与基础数据库不符,中止补丁应用...\n");

        goto end;
    }

    baseDBItem = cJSON_GetObjectItem(parsedBaseDB, "product");
    patchDBItem = cJSON_GetObjectItem(parsedPatchDB, "product");

    if (strcmp(baseDBItem->valuestring, patchDBItem->valuestring) != 0)
    {
        printf("补丁产品信息与基础数据库不符,中止补丁应用...\n");

        goto end;
    }

    baseDBItem = cJSON_GetObjectItem(parsedBaseDB, "architecture");
    patchDBItem = cJSON_GetObjectItem(parsedPatchDB, "architecture");

    if (strcmp(baseDBItem->valuestring, patchDBItem->valuestring) != 0)
    {
        printf("补丁架构信息与基础数据库不符,中止补丁应用...\n");

        goto end;
    }

    /* 将补丁拷贝至开发环境 */
    sprintf(command, "xcopy Patch\\deltaos6.2 %s /S /E /Y", dirPathBackup);
    system(command);

    /* 记录补丁的日期 */
    patchDBItem = cJSON_GetObjectItem(parsedPatchDB, "create_date");
    cJSON_ReplaceItemInObject(parsedBaseDB, "patch_date", cJSON_CreateString(patchDBItem->valuestring));

    /* 记录补丁信息 */
    baseDBItem = cJSON_GetObjectItem(parsedBaseDB, "patch_log");
    cJSON_AddItemToArray(baseDBItem, cJSON_CreateString(patchDBItem->valuestring));

    /* 更新打补丁的次数 */
    baseDBItem = cJSON_GetObjectItem(parsedBaseDB, "number_of_updates");
    cJSON_ReplaceItemInObject(parsedBaseDB, "number_of_updates", cJSON_CreateNumber(baseDBItem->valueint + 1));

    /* 记录打补丁的日期 */
    getCurrentDateAsString(date);
    cJSON_ReplaceItemInObject(parsedBaseDB, "last_update_date", cJSON_CreateString(date));

    /* 记录补丁制作人 */
    patchDBItem = cJSON_GetObjectItem(parsedPatchDB, "producer");
    cJSON_ReplaceItemInObject(parsedBaseDB, "patch_producer", cJSON_CreateString(patchDBItem->valuestring));

    /* 记录补丁文件的MD5 */
    patchDBItem = cJSON_GetObjectItem(parsedPatchDB, "files_recording");
    arraySize = cJSON_GetArraySize(patchDBItem);

    /* 遍历Patch数据库的files_recording数组 */
    for (i = 0; i < arraySize; i++)
    {
        patchFileRecordItem = cJSON_GetArrayItem(patchDBItem, i);
        itemFilename = cJSON_GetObjectItem(patchFileRecordItem, "file_name");
        itemMd5 = cJSON_GetObjectItem(patchFileRecordItem, "md5");

        index = getFileRecordIndex(parsedBaseDB, itemFilename->valuestring);

        if (index >= 0)
        {
            updateFileRecordMd5(parsedBaseDB, index, itemMd5->valuestring);
        }
        else
        {
            insertFileRecord(getDBFilesRecording(parsedBaseDB), itemFilename->valuestring, itemMd5->valuestring);
        }
    }

    writeDB(baseDBFilePath, parsedBaseDB);
    cJSON_Delete(parsedPatchDB);

    remove(compressedDBFilePath);

    getcwd(initialPath, sizeof(initialPath));

    chdir(dirPath);
    sprintf(command, "%s\\data\\z.dat -r -q %s %s", initialPath, COMPRESSED_BASE_DATABASE_FILE_NAME, BASE_DATABASE_FILE_NAME);
    system(command);

    remove(baseDBFilePath);

    chdir(initialPath);

    /* 删除解压缩后的补丁数据库 */
    remove(patchFilePath);
    system("rmdir /s /q Patch\\deltaos6.2");

    printf("打补丁完成!\n");

    return;

end:
    cJSON_Delete(parsedBaseDB);
    cJSON_Delete(parsedPatchDB);

}