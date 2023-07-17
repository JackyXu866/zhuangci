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
#include <cmath>

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

static std::vector<wchar_t> cnNum = {
    L'零', L'一', L'二', L'三', L'四',
    L'五', L'六', L'七', L'八', L'九'};
static std::vector<wchar_t> cnNumAll = {
    L'零', L'一', L'二', L'三', L'四',
    L'五', L'六', L'七', L'八', L'九', L'十', L'百', L'千', L'万'};

// 日期相关
static std::unordered_map<wchar_t, int> dayTimeMap = {
    {L'今', 0}, {L'明', 1}, {L'昨', -1}, {L'后', 2}, {L'前', -2}};
static std::vector<std::wstring> dayVec = {L"天", L"日"};
static std::vector<std::wstring> weekVec = {L"周", L"星期", L"礼拜"};
static std::vector<std::wstring> dateVec = {L"日", L"号"};


// 配置相关
static std::wregex adjConfig(L"[0-1]\\s[0-2]\\s[0-9]+");

// 地点
static std::wregex location(L"(\u4e00-\u9fa5)+[旗|县|省|市|区]");

void replaceChineseNum(std::wstring &sentence);
int chineseNumToInt(std::wstring &num);

int locateKey(std::wstring& sentence, std::vector<std::wstring>& vec, int p);
int unsureInt(int sizeMax, std::wstring& sentence, int pos);

bool matchDay(std::wstring &sentence, struct tm *time);
bool matchWeek(std::wstring& sentence, struct tm* time);
bool matchDate(std::wstring& sentence, struct tm* time);

// actions
// class Description;
// int weatherAction(std::shared_ptr<Description> description);

#endif // UTIL_H
