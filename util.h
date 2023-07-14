#ifndef UTIL_H
#define UTIL_H
#include <unordered_map>
#include <iostream>
#include <string>
#include <ctime>
#include <regex>
#include <locale>
#include <codecvt>
#include <memory>

#include "include/HTTPRequest.hpp"

#include "keyword.h"
#include "database.h"

#define NUMBER_COUNT 10
#define UNIT_COUNT 4
// #define ADJ_GAP_SPACE 3

// 零一二三四五六七八九十百
static std::wstring chineseNumber[NUMBER_COUNT] = {
    L"\u96f6", L"\u4e00", L"\u4e8c", L"\u4e09", L"\u56db",
    L"\u4e94", L"\u516d", L"\u4e03", L"\u516b", L"\u4e5d"};

// 十百千万
static std::unordered_map<std::wstring, int> chineseUnit = {
    {L"\u5341", 10}, {L"\u767e", 100}, {L"\u5343", 1000}, {L"\u4e07", 10000}};

// L"[零一二三四五六七八九十百千万]+"
static std::wregex cnNumMatch(L"[\u96f6\u4e00\u4e8c\u4e09\u56db\u4e94\u516d\u4e03\u516b\u4e5d\u5341\u767e\u5343\u4e07]+");
static std::wregex nonValid(L"[^\u4e00-\u9fa5\u0030-\u0039\u0041-\u005a\u0061-\u007a]");
static std::wregex cnDigit(L"[\u96f6\u4e00\u4e8c\u4e09\u56db\u4e94\u516d\u4e03\u516b\u4e5d]");
static std::wregex cnUnit(L"[\u5341\u767e\u5343\u4e07]");

// 日期相关
static std::wregex day(L"[今|明|昨|后|前]天");
static std::wregex week(L"(上|下|这)?(周|星期|礼拜)([1-6]|天|日)");
static std::wregex date(L"((\\d{4})年)?((\\d{1,2})月)?(\\d{1,2})(日|号)");

// 配置相关
static std::wregex adjConfig(L"[0-1]\\s[0-2]\\s[0-9]+");

// 地点
static std::wregex location(L"(\u4e00-\u9fa5)+[旗|县|省|市|区]");

void getDay(std::wstring &dayStr, struct tm *time);
void getWeek(std::wstring &weekStr, struct tm *time);
void getDate(std::wstring &dateStr, struct tm *time);

std::wstring replaceChineseNum(std::wstring &sentence);
int chineseNumToInt(std::wstring &num);

// actions
class Description;
int weatherAction(std::shared_ptr<Description> description);

#endif // UTIL_H
