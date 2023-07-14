#define _CRT_SECURE_NO_WARNINGS
#include "util.h"

// 以下3个函数用于将中文时间转换为tm结构体

/**
 * @brief 将中文时间中的日期信息转换为tm结构体中的日信息
 * 
 * @param dayStr 中文时间中的日期信息
 * @param time tm结构体指针
 */
void getDay(std::wstring &dayStr, struct tm *time)
{
    wchar_t prev = dayStr[0];
    switch (prev)
    {
    case L'今':
        time->tm_mday = time->tm_mday;
        break;
    case L'明':
        time->tm_mday = time->tm_mday + 1;
        break;
    case L'昨':
        time->tm_mday = time->tm_mday - 1;
        break;
    case L'后':
        time->tm_mday = time->tm_mday + 2;
        break;
    case L'前':
        time->tm_mday = time->tm_mday - 2;
        break;

    default:
        break;
    }
}

/**
 * @brief 将中文时间中的星期信息转换为tm结构体中的日信息
 * 
 * @param weekStr 中文时间中的星期信息
 * @param time tm结构体指针
 */
void getWeek(std::wstring &weekStr, struct tm *time)
{
    // match prefix
    std::wregex prefix(L"^(上|下|这)[\u4E00-\u9FA5A-Za-z0-9]*");
    if (std::regex_match(weekStr, prefix))
    {
        wchar_t prev = weekStr[0];
        switch (prev)
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
    }

    // match actual day
    wchar_t last = weekStr[weekStr.size() - 1];
    int day = 0;
    if (last == L'天' || last == L'日')
    {
        day = 0;
    }
    else
    {
        day = last - L'0';
    }
    int diff = day - time->tm_wday;
    time->tm_mday += diff;
}

/**
 * @brief 将中文时间中的年月日信息转换为tm结构体中的年月日信息
 * 
 * @param dateStr 中文时间中的年月日信息
 * @param time tm结构体指针
 */
void getDate(std::wstring &dateStr, struct tm *time)
{
    // 年
    int find = dateStr.find(L'年');
    int tmp = 0;
    if ((size_t)find != std::wstring::npos)
    {
        std::wstring yearStr = dateStr.substr(0, find);
        tmp = std::stoi(yearStr);
        if (tmp < 1900)
            return;
        time->tm_year = tmp - 1900;
        dateStr = dateStr.substr(find + 1);
    }

    // 月
    find = dateStr.find(L'月');
    if ((size_t)find != std::wstring::npos)
    {
        std::wstring monthStr = dateStr.substr(0, find);
        tmp = std::stoi(monthStr);
        if (tmp < 1 || tmp > 12)
            return;
        time->tm_mon = tmp - 1;
        dateStr = dateStr.substr(find + 1);
    }

    // 日
    std::wstring dayStr = dateStr.substr(0, dateStr.size() - 1);
    tmp = std::stoi(dayStr);
    if (tmp < 1 || tmp > 31)
        return;
    time->tm_mday = tmp;
}


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
        // index += temp.size();
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


// /**
//  * @brief 使用高德地图API获取天气信息
//  * 
//  * @param description 描述信息
//  * @return int 返回1表示成功
//  */
// int weatherAction(std::shared_ptr<Description> description)
// {
//     std::string key = "071ba2a731fad6093c444925d44a82d5";
//     std::string url = "http://restapi.amap.com/v3/weather/weatherInfo?";
//     std::string city = "110101";
//     std::string extensions = "base";

//     http::Request request(url + "key=" + key + "&city=" + city + "&extensions=" + extensions);
//     const http::Response response = request.send("GET");
//     std::string beforeW = std::string(response.body.begin(), response.body.end());
//     std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
//     std::wstring res = converter.from_bytes(beforeW);

//     std::wcout << res << std::endl
//                << std::endl;

//     return 1;
// }


// int main(){
//     setlocale(LC_ALL, "zh_CN.UTF-8");
//     std::wstring test = L"我要买一百个苹果，三个橘子，五个梨子，还有一千个香蕉。abcabc";
//     // set locale otherwise chinese character will not be printed
//     std::wcout << replaceChineseNum(test) << std::endl;
//     std::wcout << "aaabbbccc" << std::endl;

//     // std::wstring test = L"八万零十";
//     // std::wcout << chineseNumToInt(test) << std::endl;

//     return 0;
// }