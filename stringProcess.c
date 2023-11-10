#include <string.h>

void replaceBackslash(char *string)
{
    size_t length;

    // 移除输入字符串中的换行符
    string[strcspn(string, "\n")] = '\0';

    length = strlen(string);

    for (int i = 0; i < length; i++)
    {
        if (string[i] == '\\')
        {
            string[i] = '/';
        }
    }
}

/* 返回字符串中从后向前第一个'/'字符的位置，如果字符串的最后一个字符是'/'则不统计 */
int getBaseIndex (char *string)
{
    int index = -1;
    int i;
    size_t length;

    length = strlen(string);

    if (string[length - 1] == '/')
    {
        length--;
    }

    for (i = (int)length - 1; i >= 0; i--)
    {
        if (string[i] == '/')
        {
            index = (int)i;
            break;
        }
    }

    return index;
}
