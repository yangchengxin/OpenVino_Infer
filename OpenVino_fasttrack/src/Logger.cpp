#include <stdio.h>
#include <string>
#include <stdarg.h>
#include <time.h>

#include "Logger.h"

using namespace std;

#define RUN_LOG_FILENAME "RunLog.log"
#define LOG_FILE_MAX_SIZE (60 * 1024 * 1024)   // 单文件大小限制:60MB

class LogPrinter::LogPrinterDef
{
private:
    static const string m_printLevel[PRINT_LEV_NUM];

public:
    void prevInfoPrint(PRINT_LEV printLev, const char* fileName, int lineNo) {
        char buf[128];

        time_t now = time(0);
        tm ltm;
        localtime_s(&ltm, &now);

        sprintf_s(buf, "[%d/%02d/%02d %2d:%02d:%02d] [%s:%d] [%s]: ",
            ltm.tm_year + 1900, ltm.tm_mon + 1, ltm.tm_mday,
            ltm.tm_hour, ltm.tm_min, ltm.tm_sec,
            fileName, lineNo, m_printLevel[printLev].data());

        printf("%s", buf);

        FILE* fp;
        ::fopen_s(&fp, RUN_LOG_FILENAME, "a+");
        if (!fp) {
            return;
        }

        fwrite(buf, sizeof(char), strlen(buf), fp);
        fclose(fp);
    }

    int GetFileSize() {
        struct stat statbuf;
        stat(RUN_LOG_FILENAME, &statbuf);
        return statbuf.st_size;
    }

    void logFileSizeCtl() {
        if (GetFileSize() < LOG_FILE_MAX_SIZE)
            return;

        const char* newname = "RunLogBack.log";

        FILE* fp;
        fopen_s(&fp, newname, "r");
        if (fp != NULL) {
            fclose(fp);
            remove(newname);
        }
        rename(RUN_LOG_FILENAME, newname);
    }
};

LogPrinter::LogPrinterDef* LogPrinter::_PrinterDef = new LogPrinterDef;

const string LogPrinter::LogPrinterDef::m_printLevel[PRINT_LEV_NUM] = {
    "Debug",
    "Info",
    "Event",
    "Error",
    "Unknown",
};


LogPrinter::LogPrinter() {}

LogPrinter::~LogPrinter() { delete _PrinterDef; _PrinterDef = nullptr; }

/* 记录日志
* @param printLev  [日志信息等级]
* @param filename  [日志信息文件名]
* @param lineNo    [日志信息所在行]
* @param format    [日志信息列表格式]
*/
void __cdecl LogPrinter::LogPrint(PRINT_LEV printLev, const char* filename, int lineNo, const char* format, ...)    // __cdecl 是关键的返回值类型
{
    if (printLev < 0 || printLev >= PRINT_LEV_NUM)
        printLev = PRINT_LEV_UNKNOWN;

    _PrinterDef->logFileSizeCtl();

    string s_filiname;
    const char* temp = strrchr(filename, '\\');
    if (temp)
        s_filiname = temp + 1;
    else
        s_filiname = filename;
    _PrinterDef->prevInfoPrint(printLev, s_filiname.data(), lineNo);

    va_list args;
    va_start(args, format);
    vprintf(format, args);
    printf("\n");

    FILE* fp;
    ::fopen_s(&fp, RUN_LOG_FILENAME, "a+");
    if (!fp) {
        return;
    }
    vfprintf(fp, format, args);
    fwrite("\n", 1, 1, fp);
    fclose(fp);

    return;
}