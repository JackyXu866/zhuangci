#ifndef UTIL_H
#define UTIL_H
#include <unordered_map>
#include <iostream>
#include <string>
#include <ctime>
#include <locale>
#include <cmath>
#include <vector>
#include <memory>
#include <codecvt>

#include "keyword.h"
#include "database.h"
#include "httplib.h"

// 数字相关
static std::unordered_map<wchar_t, int> chineseUnit = {
    {L'十', 10}, {L'百', 100}, {L'千', 1000}, {L'万', 10000}};

static std::vector<wchar_t> cnNum = {
    L'零', L'一', L'二', L'三', L'四',
    L'五', L'六', L'七', L'八', L'九'};
static std::vector<wchar_t> cnNumAll = {
    L'零', L'一', L'二', L'三', L'四',
    L'五', L'六', L'七', L'八', L'九', L'十', L'百', L'千', L'万'};
static std::vector<wchar_t> cnUnit = {
    L'十', L'百', L'千', L'万'};

// 日期相关
static std::unordered_map<wchar_t, int> dayTimeMap = {
    {L'今', 0}, {L'明', 1}, {L'昨', -1}, {L'后', 2}, {L'前', -2}};
static std::vector<std::wstring> dayVec = {L"天", L"日"};
static std::vector<std::wstring> weekVec = {L"周", L"星期", L"礼拜"};
static std::vector<std::wstring> dateVec = {L"日", L"号"};

// 地点 [旗|县|省|市|区]

bool isChineseNum(wchar_t c);

void replaceChineseNum(std::wstring &sentence);
int chineseNumToInt(std::wstring &num);

int locateKey(std::wstring &sentence, std::vector<std::wstring> &vec, int p);
int unsureInt(int sizeMax, std::wstring &sentence, int pos);

bool matchDay(std::wstring &sentence, struct tm *time);
bool matchWeek(std::wstring &sentence, struct tm *time);
bool matchDate(std::wstring &sentence, struct tm *time);

#endif // UTIL_H
