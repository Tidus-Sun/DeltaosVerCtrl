#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <cjson/cJson.h>

extern char *calculateMD5(const char *filePath);
extern int getBaseIndex (char *string);

cJSON *getDBFilesRecording(cJSON *parsedDB)
{
    cJSON *dbFilesRecording;

    // 从解析后的JSON对象中获取 "dbFilesRecording" 数组
    dbFilesRecording = cJSON_GetObjectItem(parsedDB, "files_recording");

    return dbFilesRecording;
}

void writeDB(char *filePath, cJSON *parsedDB)
{
    FILE *file;
    // 生成更新后的JSON字符串
    char *updatedJsonString = cJSON_Print(parsedDB);

    file = fopen(filePath, "w");
    if (file)
    {
        fputs(updatedJsonString, file);
        fclose(file);
    }

    // 释放内存
    cJSON_Delete(parsedDB);
    free(updatedJsonString);
}

void insertFileRecord(cJSON *dbFilesRecording, const char *filename, const char *md5)
{
    // 创建新的文件记录对象
    cJSON *fileRecord = cJSON_CreateObject();
    cJSON_AddItemToObject(fileRecord, "file_name", cJSON_CreateString(filename));
    cJSON_AddItemToObject(fileRecord, "md5", cJSON_CreateString(md5));

    // 将新文件记录对象添加到 "dbFilesRecording" 数组
    cJSON_AddItemToArray(dbFilesRecording, fileRecord);
}

void recordMd5(cJSON *dbFilesRecording, const char *dirPath, int pathStartIndex)
{
    DIR *dir;
    struct dirent *entry;
    struct stat fileInfo;
    char filePath[PATH_MAX];

    char *md5;

    if ((dir = opendir(dirPath)) == NULL)
    {
        perror("Open Dir Error");
        return;
    }

    while ((entry = readdir(dir)) != NULL)
    {
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

            printf("file: %s\n", filePath + pathStartIndex);
            insertFileRecord(dbFilesRecording, filePath + pathStartIndex, md5);
            free(md5);
        }
        else if (S_ISDIR(fileInfo.st_mode) && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
        {
            recordMd5(dbFilesRecording, filePath, pathStartIndex); // Recursively traverse directories
        }
    }

    closedir(dir);
}

cJSON *openDBFile(char *dbFile)
{
    FILE *file;
    long fileSize;
    char *fileContent;
    cJSON *parsedDB;

    file = fopen(dbFile, "r");
    if (file)
    {
        fseek(file, 0, SEEK_END);
        fileSize = ftell(file);
        fseek(file, 0, SEEK_SET);

        fileContent = (char *) malloc(fileSize + 1);
        fread(fileContent, 1, fileSize, file);
        fileContent[fileSize] = '\0';
        fclose(file);

        parsedDB = cJSON_Parse(fileContent);
        free(fileContent);

        return parsedDB;
    }

    return NULL;
}

/* 返回基础数据中指定文件在files_recording数组中的索引号 */
int getFileRecordIndex(cJSON *parsedDB, char *fileName)
{
    cJSON *filesRecording;
    cJSON *arrayItem;
    cJSON *item;
    int arraySize;
    int i;

    filesRecording = getDBFilesRecording(parsedDB);
    arraySize = cJSON_GetArraySize(filesRecording);

    for (i = 0; i < arraySize; i++)
    {
        arrayItem = cJSON_GetArrayItem(filesRecording, i);
        item = cJSON_GetObjectItem(arrayItem, "file_name");

        if (strcmp(fileName, item->valuestring) == 0)
        {
            return i;
        }
    }

    return -1;
}

/* 返回基础数据中指定文件的MD5 */
char *getFileRecordMd5(cJSON *parsedDB, char *fileName)
{
    cJSON *filesRecording;
    cJSON *arrayItem;
    cJSON *item;
    int arraySize;
    int i;

    filesRecording = getDBFilesRecording(parsedDB);
    arraySize = cJSON_GetArraySize(filesRecording);

    for (i = 0; i < arraySize; i++)
    {
        arrayItem = cJSON_GetArrayItem(filesRecording, i);
        item = cJSON_GetObjectItem(arrayItem, "file_name");

        if (strcmp(fileName, item->valuestring) == 0)
        {
            item = cJSON_GetObjectItem(arrayItem, "md5");

            return item->valuestring;
        }
    }

    return NULL;
}

void updateFileRecordMd5(cJSON *parsedDB, int index, char *newMd5)
{
    cJSON *filesRecording;
    cJSON *arrayItem;

    filesRecording = getDBFilesRecording(parsedDB);
    arrayItem = cJSON_GetArrayItem(filesRecording, index);
    cJSON_ReplaceItemInObject(arrayItem, "md5", cJSON_CreateString(newMd5));
}