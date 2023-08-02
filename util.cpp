#define _CRT_SECURE_NO_WARNINGS
#include "util.h"

// 是否为中文数字
bool isChineseNum(wchar_t c) {
    for (int i = 0; i < cnNumAll.size(); i++) {
        if (c == cnNumAll[i]) {
            return true;
        }
    }

    return false;
}

// 中文数字转阿拉伯数字，如果是其他的则返回原字符
wchar_t convertChineseNum(wchar_t c) {
    for (int i = 0; i < cnNum.size(); i++) {
        if (c == cnNum[i]) return (L'0' + i);
    }

    return c;
}

/**
 * @brief 将中文数字替换为阿拉伯数字
 *
 * @param sentence 待替换的句子
 * @return std::wstring 替换后的句子
 */
void replaceChineseNum(std::wstring &sentence) {
    int p = 0;
    // 查找含有中文数字的位置
    while (p < sentence.size()) {
        // 找到一组连续的中文数字
        int beg = p, end = p;
        while (end < sentence.size() && isChineseNum(sentence[end])) {
            end++;
        }
        // 没有中文数字
        if (beg == end) {
            p++;
            continue;
        }
        // 替换
        std::wstring num = sentence.substr(beg, end - beg);
        int n = chineseNumToInt(num);
        sentence.replace(beg, end - beg, std::to_wstring(n));
        p = beg + std::to_wstring(n).size();
    }
}

/**
 * @brief 将中文数字转换为阿拉伯数字
 *
 * @param num 中文数字
 * @return int 阿拉伯数字
 */
int chineseNumToInt(std::wstring &num) {
    // 替换所有数字字符
    for (int i = 0; i < num.size(); i++) {
        num[i] = convertChineseNum(num[i]);
    }

    // 如果是纯数字, e.g. 一二三, return 123
    int pureNum = true;
    for (int i = 0; i < num.size(); i++) {
        for (int j = 0; j < cnUnit.size(); j++) {
            if (num[i] == cnUnit[j]) {
                pureNum = false;
                break;
            }
        }
    }
    if (pureNum) return std::stoi(num);

    // 删除所有 0
    for (int i = 0; i < num.size(); i++) {
        if (num[i] == L'0') {
            num.erase(i, 1);
            i--;
        }
    }

    // 处理有单位的数字
    int rt = 0;
    int prevNum = 1;
    for (int i = 0; i < num.size(); i++) {
        // 是数字
        if (num[i] - L'0' >= 1 && num[i] - L'0' <= 9) {
            prevNum = num[i] - L'0';
        }
        else {  // 是单位
            rt += (prevNum * chineseUnit[num[i]]);
        }
    }
    // 处理最后一个数字
    if (num[num.size() - 1] - L'0' >= 1 && num[num.size() - 1] - L'0' <= 9) {
        rt += (num[num.size() - 1] - L'0');
    }

    return rt;
}

// 找到一个vector中任意词的位置
int locateKey(std::wstring &sentence, std::vector<std::wstring> &vec, int p) {
    int pos = -1;
    for (std::wstring &d : vec) {
        pos = std::max((int)sentence.find(d, p), pos);
        if (pos != std::wstring::npos) {
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
int unsureInt(int sizeMax, std::wstring &sentence, int pos) {
    if (pos < 0 || pos >= sentence.size()) return -1;

    int rt = 0;
    int size = 0;

    while (size < sizeMax) {
        int p = pos - size;
        if (p < 0) break;
        wchar_t c = sentence[pos - size];
        if (c >= L'0' && c <= L'9') {
            rt += ((c - L'0') * std::pow(10, size));
        }
        // 没找到数字
        else if (size == 0)
            return -1;
        else
            break;

        size++;
    }
    return rt;
}

// pattern match day without regex
bool matchDay(std::wstring &sentence, struct tm *time) {
    int p = 0;  // 当前位置，循环直到找到所有的时间词或者找不到为止
    // 先找定位词 天/日
    while (p < sentence.size()) {
        int pos = locateKey(sentence, dayVec, p);
        if (pos == std::wstring::npos || pos == 0) {
            break;
        }

        // 找表达时间词
        for (std::pair<wchar_t, int> p : dayTimeMap) {
            if (sentence.find(p.first) != std::wstring::npos) {
                time->tm_mday += p.second;
                return true;
            }
        }

        p = pos + 1;
    }
    return false;
}

// pattern match week without regex
bool matchWeek(std::wstring &sentence, struct tm *time) {
    int p = 0;  // 当前位置，循环直到找到所有的时间词或者找不到为止
    // 先找定位词 周
    while (p < sentence.size()) {
        int pos = locateKey(sentence, weekVec, p);
        if (pos == std::wstring::npos || pos == sentence.size() - 1) {
            break;
        }

        // 先找表达时间词
        wchar_t last = sentence[pos + 1];
        int day = 0;
        if (last == L'天' || last == L'日') {
            day = 0;
        }
        else {
            day = last - L'0';
            // 错误的时间词
            if (day < 1 || day > 7) {
                p = pos + 1;
                continue;
            }
        }
        int diff = day - time->tm_wday;
        time->tm_mday += diff;

        // 看看有没有前置词
        wchar_t pre = sentence[pos - 1];
        switch (pre) {
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
bool matchDate(std::wstring &sentence, struct tm *time) {
    int p = 0;
    // 先找 最小时间单位(天数)
    while (p < sentence.size()) {
        int posDay = locateKey(sentence, dateVec, p);
        if (posDay == std::wstring::npos || posDay == 0) {
            break;
        }

        // 确定天数
        int mday = unsureInt(2, sentence, posDay - 1);

        // 验证天数
        if (mday < 1 || mday > 31) {
            p = posDay + 1;
            continue;
        }
        time->tm_mday = mday;
        /*********************************************/

        // 确定月份
        int posMonth = posDay;
        if (mday >= 10)
            posMonth -= 3;
        else
            posMonth -= 2;

        if (posMonth < 0) return true;
        if (sentence[posMonth] != L'月') {
            return true;
        }

        int month = unsureInt(2, sentence, posMonth - 1);
        if (month < 1 || month > 12) {
            return true;
        }
        time->tm_mon = month - 1;
        /*********************************************/

        // 确定年份
        int posYear = posMonth;
        if (month >= 10)
            posYear -= 3;
        else
            posYear -= 2;

        if (posYear < 0) return true;
        if (sentence[posYear] != L'年') {
            return true;
        }

        int year = unsureInt(4, sentence, posYear - 1);
        if (year < 0) return true;
        // 支持两位数年份
        if (year < 40)
            year += 2000;
        else if (year < 100)
            year += 1900;
        else if (year < 1900)
            return true;
        time->tm_year = year - 1900;

        return true;
    }

    return false;
}

void readCSV_skill(const char *path, std::shared_ptr<Database> db) {
    // TODO: windows不支持中文路径，需要在windows环境使用`_wfopen`
    FILE *file = std::fopen(path, "rb,ccs=UTF-8");
    if (!file) {
        std::cout << "Error opening file " << path << std::endl;
        return;
    }

    wint_t c;
    bool multiline = false;  // check open and close \"
    int numCell = 0;         // number of cell in one line (x/2)
    std::wstring temp = L"";
    bool isComplete = false;  // true if one segment is complete
    std::shared_ptr<Keyword> keyword = nullptr;
    while (c = std::fgetwc(file) != EOF) {
        wchar_t ch = (wchar_t)c;
        std::wcout << c;
        switch (ch) {
            case ',':  // end of cell
                // cell text contains ,
                if (multiline) {
                    temp += ch;
                    break;
                }

                isComplete = true;
                break;
            case '\"':  // start or end of multiline
                // TODO: 文本带有" 会出错 ("""aaa""")
                multiline = !multiline;
                isComplete = !multiline;
                break;
            case '\n':  // end of line
                isComplete = true;

                break;
            default:  // normal char
                temp += ch;
                break;
        }

        if (temp == L"" || !isComplete) continue;
        // TODO: add to database
        switch (numCell) {
            case 0:  // skill name
                if (db->keywords.find(temp) != db->keywords.end()) {
                    keyword = db->keywords[temp];
                }
                else {
                    keyword =
                        std::shared_ptr<Keyword>(new Keyword(temp, false));
                    db->keywords[temp] = keyword;
                }
                std::wcout << L"读取技能: " << temp << L"\n";
                break;
            case 1:  // similar words
                keyword->addSimilarWord(temp);
                break;
            case 2:  // response
                keyword->addResponse(temp);
                break;
            default:
                break;
        }
        temp = L"";

        // completed one cell
        if (!multiline) numCell++;
        if (!multiline && numCell > 2) numCell = 0;
    }
}

int main() {
    setlocale(LC_ALL, "zh_CN.UTF-8");
    std::shared_ptr<Database> db = std::shared_ptr<Database>(new Database());
    readCSV_skill("./test.csv", db);
    std::wcout << L"\n读取完成\n";

    return 0;
}