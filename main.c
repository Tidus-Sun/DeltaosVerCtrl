#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include "config.h"

#define VERSION "v0.9"

extern int toolIntegrityCheck();
extern void base64Encode(const unsigned char *input, int length, char *output);
extern void makeBaseDatabase(char *filename);
extern void makePatchDatabase(char *filename);
extern void applyPatch();
extern void baseDBInfoShow();
extern void patchDBInfoShow();

int main()
{
    char funcString[32] = {};
    int function;
    char *endPtr;
    char encodedString[64] = {};
    char password[32] = {};

    /* just for debug */
    //setbuf(stdout, NULL);

    SetConsoleOutputCP(CP_UTF8);

    system("color 3F");

    if (toolIntegrityCheck() != 0)
    {
        printf("请检查工具完整性!\n");
        system("pause");

        return -1;
    }

    printf("┌─────────────────────────────────────────┐\n");
    printf("│    欢迎使用道系统版本控制工具集 %s    │\n", VERSION);
    printf("│                           Write By Tidus│\n");
    printf("└─────────────────────────────────────────┘\n");

label:

    printf("请选择需要的功能: \n\n");
    printf("1.打补丁\n2.查看补丁数据库信息\n3.查看基础数据库信息\n4.制作补丁\n5.创建基础数据库\nQ.退出程序\n\n");
    fgets(funcString, sizeof(funcString), stdin);
    funcString[strcspn(funcString, "\n")] = '\0';

    if ((strncmp(funcString, "q", 1) == 0 || strncmp(funcString, "Q", 1) == 0) && (strlen(funcString) == 1))
    {
        printf("\n感谢使用,再见!\n\n");

        system("pause");

        return 0;
    }


    function = (int) strtol(funcString, &endPtr, 10);

    if (*endPtr == '\0')
    {
        switch (function)
        {
            case 1:
                applyPatch();
                break;
            case 2:
                patchDBInfoShow();
                break;
            case 3:
                baseDBInfoShow();
                break;
            case 4:
                makePatchDatabase(PATCH_DATABASE_FILE_NAME);
                break;
            case 5:
                printf("请输入密码:\n");
                fgets(password, sizeof(password), stdin);
                password[strcspn(password, "\n")] = '\0';

                base64Encode((const unsigned char *) password, (int) strlen(password), encodedString);

                if (strcmp(INTERNEL_PASSWORD, encodedString) == 0)
                {
                    makeBaseDatabase(BASE_DATABASE_FILE_NAME);
                }
                else
                {
                    printf("密码错误!\n");
                }
                break;

            default:
                break;
        }
    }

    printf("\n我的发?你选择的功能不存在!\n");
    printf("\n");

    goto label;
}
