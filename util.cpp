#define _CRT_SECURE_NO_WARNINGS
#include "util.h"


/**
 * @brief 将中文数字替换为阿拉伯数字
 * 
 * @param sentence 待替换的句子
 * @return std::wstring 替换后的句子
 */
void replaceChineseNum(std::wstring &sentence)
{
    std::wsmatch match;
    std::wstring::const_iterator iterStart = sentence.begin();
    std::wstring::const_iterator iterEnd = sentence.end();
    std::wstring temp;
    int index = 0;
    while (std::regex_search(iterStart, iterEnd, match, cnNumMatch))
    {
        temp = match[0];
        index += match.position(0);
        // std::wcout << temp << " at " << index << " with size "
        // << temp.size() << std::endl;
        iterStart = match[0].second;

        std::wstring numStr = std::to_wstring(chineseNumToInt(temp));

        // replace the number
        sentence.replace(match[0].first, match[0].second, numStr.begin(), numStr.end());
    }

    // std::wcout << "sentence: " << sentence << std::endl;
}

/**
 * @brief 将中文数字转换为阿拉伯数字
 * 
 * @param num 中文数字
 * @return int 阿拉伯数字
 */
int chineseNumToInt(std::wstring &num)
{

    // replace all digits
    for (int i = 0; i < NUMBER_COUNT; i++)
    {
        std::wstring digit = chineseNumber[i];
        std::wregex digitMatch(digit);
        num = std::regex_replace(num, digitMatch, std::to_wstring(i));
    }

    // if no unit, e.g. 一二三, return 123
    if (!std::regex_search(num, cnUnit))
    {
        return std::stoi(num);
    }

    // remove all 0
    num = std::regex_replace(num, std::wregex(L"0"), L"");

    // replace all units
    int rt = 0;
    int currPos = 0;
    std::wstring::const_iterator iterStart = num.begin();
    std::wstring::const_iterator iterEnd = num.end();
    std::wsmatch match;
    while (std::regex_search(iterStart, iterEnd, match, cnUnit))
    {
        std::wstring unit = match[0];
        int pos = match.position(0);
        currPos += pos;
        std::wstring tdigit = num.substr(currPos - 1, 1);
        if (!std::regex_match(tdigit, std::wregex(L"[1-9]")))
        {
            tdigit = L"1";
        }
        int digit = std::stoi(tdigit);
        rt += (digit * chineseUnit[unit]);

        currPos += 1;
        iterStart = match[0].second;
    }
    // last digit
    std::wstring tdigit = num.substr(num.size() - 1, 1);
    if (std::regex_match(tdigit, std::wregex(L"[1-9]")))
    {
        rt += std::stoi(tdigit);
    }

    return rt;
}

int locateKey(std::wstring& sentence, std::vector<std::wstring>& vec, int p){
    int pos = -1;
    for(std::wstring& d : vec){
        pos = std::max((int)sentence.find(d, p), pos);
        if(pos != std::wstring::npos){
            break;
        }
    }
    return pos;
}

// @brief 从句子中提取不确定位数的数字
// @param sizeMax: 最大位数
// @param sentence: 句子
// @param pos: 根部（个位）位置
// @return int: 数字，-1表示没找到
int unsureInt(int sizeMax, std::wstring& sentence, int pos){
    if(pos < 0 || pos >= sentence.size()) return -1;

    int rt = 0;
    int size = 0;

    while(size < sizeMax){
        int p = pos-size;
        if(p < 0) break;
        wchar_t c = sentence[pos - size];
        if(c >= L'0' && c <= L'9'){
            rt += ((c - L'0') * std::pow(10, size));
        }
        // 没找到数字
        else if(size == 0) return -1;
        else break;

        size++;
    }
    return rt;
}

// pattern match day without regex
bool matchDay(std::wstring& sentence, struct tm* time){
    int p = 0;  // 当前位置，循环直到找到所有的时间词或者找不到为止
    // 先找定位词 天/日
    while(p < sentence.size()){
        int pos = locateKey(sentence, dayVec, p);
        if(pos == std::wstring::npos || pos == 0){
            break;
        }

        // 找表达时间词
        for(std::pair<wchar_t, int> p : dayTimeMap){
            if(sentence.find(p.first) != std::wstring::npos){
                time->tm_mday += p.second;
                return true;
            }
        }

        p = pos + 1;
    }
    return false;
}

// pattern match week without regex
bool matchWeek(std::wstring& sentence, struct tm* time){
    int p = 0;  // 当前位置，循环直到找到所有的时间词或者找不到为止
    // 先找定位词 周
    while(p < sentence.size()){
        int pos = locateKey(sentence, weekVec, p);
        if(pos == std::wstring::npos || pos == sentence.size()-1){
            break;
        }

        // 先找表达时间词
        wchar_t last = sentence[pos+1];
        int day = 0;
        if (last == L'天' || last == L'日')
        {
            day = 0;
        }
        else
        {
            day = last - L'0';
            // 错误的时间词
            if(day < 1 || day > 7){
                p = pos + 1;
                continue;
            }
        }
        int diff = day - time->tm_wday;
        time->tm_mday += diff;

        // 看看有没有前置词
        wchar_t pre = sentence[pos - 1];
        switch (pre)
        {
        case L'上':
            time->tm_mday -= 7;
            break;
        case L'下':
            time->tm_mday += 7;
            break;
        case L'这':
            break;
        default:
            break;
        }

        return true;
    }
    return false;

}

// xxxx年xx月xx日，只要找到日就行
bool matchDate(std::wstring& sentence, struct tm* time){
    int p = 0;
    // 先找 最小时间单位(天数)
    while(p < sentence.size()){
        int posDay = locateKey(sentence, dateVec, p);
        if(posDay == std::wstring::npos || posDay == 0){
            break;
        }

        // 确定天数
        int mday = unsureInt(2, sentence, posDay-1);

        // 验证天数
        if(mday < 1 || mday > 31){
            p = posDay + 1;
            continue;
        }
        time->tm_mday = mday;
        /*********************************************/

        // 确定月份
        int posMonth = posDay;
        if(mday >= 10) posMonth -= 3;
        else posMonth -= 2;

        if(posMonth < 0) return true;
        if(sentence[posMonth] != L'月'){
            return true;
        }

        int month = unsureInt(2, sentence, posMonth-1);
        if(month < 1 || month > 12){
            return true;
        }
        time->tm_mon = month - 1;
        /*********************************************/

        // 确定年份
        int posYear = posMonth;
        if(month >= 10) posYear -= 3;
        else posYear -= 2;

        if(posYear < 0) return true;
        if(sentence[posYear] != L'年'){
            return true;
        }

        int year = unsureInt(4, sentence, posYear-1);
        if(year < 0) return true;
        // 支持两位数年份
        if(year < 40) year += 2000;
        else if(year < 100) year += 1900;
        else if(year < 1900) return true;
        time->tm_year = year - 1900;

        return true;
    }

    return false;
}
