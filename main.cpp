

#define _CRT_SECURE_NO_WARNINGS
#ifndef MAIN_CPP
#define MAIN_CPP

#include <fstream>

#include "util.h"
#include "time.h"

//  读取配置文件，之后需迁移
std::shared_ptr<Database> readConfig(char *config)
{
    std::wifstream file(config);
    if (!file.is_open())
    {
        std::cout << "Error opening file " << config << std::endl;
        return nullptr;
    }
    file.imbue(std::locale("zh_CN.UTF-8"));

    std::shared_ptr<Database> db(new Database);

    std::wstring line;
    std::shared_ptr<Keyword> keyword = nullptr;

    int type = 0; // 0: key, 1: adj, 2: prev, 3: similarWord
    while (std::getline(file, line))
    {
        // match type
        if (line == L"key:")
        {
            type = 0;
        }
        else if (line == L"adj:")
        {
            type = 1;
        }
        else if (line == L"prev:")
        {
            type = 2;
        }
        else if (line == L"")
        {
            type = -1;
        }
        else
        { // information
            replaceChineseNum(line);
            if (type == 0)
            {
                if (db->keywords.find(line) != db->keywords.end())
                {
                    keyword = db->keywords[line];
                    // std::wcout << L"找到已有关键词: " << line << std::endl;
                }
                else
                {
                    keyword = std::shared_ptr<Keyword>(new Keyword(line));
                    db->keywords[line] = keyword;
                }
                type = 3;
                // std::wcout << "Begin: " << line << std::endl;
            }
            else if (type == 1)
            {
                if (std::regex_match(line, adjConfig))
                {
                    int tmp = line[0] - L'0';
                    keyword->mustAdj = (tmp == 1);
                    keyword->ADJ_SEARCH_TYPE = line[2] - L'0';
                    keyword->ADJ_GAP_SPACE = std::stoi(line.substr(4));

                    // std::wcout << L"ADJ_SEARCH_TYPE: " << keyword->ADJ_SEARCH_TYPE << std::endl;
                    // std::wcout << L"ADJ_GAP_SPACE: " << keyword->ADJ_GAP_SPACE << std::endl;
                    continue;
                }
                keyword->addAdjective(line);
                // std::wcout << "Adj: " << line << std::endl;
            }
            else if (type == 2)
            {
                if (db->keywords.find(line) == db->keywords.end())
                {
                    std::shared_ptr<Keyword> prevKeyword(new Keyword(line));
                    db->keywords[line] = prevKeyword;
                }
                keyword->addPrevKeyword(db->keywords[line]);
                db->keywords[line]->addNextKeyword(keyword);
                // std::wcout << db->keywords[line]->getWord() << L"Add next keyword: " << keyword->getWord() << std::endl;

                keyword->head = false;
                db->keywords[line]->tail = false;
                // std::wcout << "Prev: " << line << std::endl;
            }
            else
            {
                keyword->addSimilarWord(line);
                // std::wcout << "Key: " << line << std::endl;
            }
        }
        line.clear();
    }

    file.close();
    return db;
}

// 筛选出不含前置需求（多轮需求）的关键词
void FilterDB(std::shared_ptr<Database> db)
{
    db->headKeywords = std::vector<std::shared_ptr<Keyword>>();
    // db->tempKeywords = std::vector<std::shared_ptr<Keyword>>();

    for (std::pair<std::wstring, std::shared_ptr<Keyword>> p : db->keywords)
    {
        std::shared_ptr<Keyword> k = p.second;
        if (k->head)
        {
            db->headKeywords.push_back(k);
            // std::wcout << L"Head keyword: " << k->getWord() << std::endl;
        }
    }
}

// 获取用户输入的时间有关信息，并根据现在本地时间更新新时间
struct tm *matchDate(std::wstring sentence)
{
    // get current tm
    time_t rawtime;
    time(&rawtime);
    struct tm *timeinfo = localtime(&rawtime);

    // match date
    if (matchDay(sentence, timeinfo))
    {
        // std::wcout << L"Matched day" << std::endl;
    }
    else if (matchWeek(sentence, timeinfo))
    {
        // std::wcout << L"Matched week" << std::endl;
    }
    else if (matchDate(sentence, timeinfo))
    {
        // std::wcout << L"Matched date" << std::endl;
    }

    return timeinfo;
}

// 匹配单个关键词并进一步更新具体信息
int matchKeyword(std::wstring sentence, std::shared_ptr<Keyword> k, std::shared_ptr<Database> db)
{
    std::pair<int, int> p = k->match(sentence);
    int find = p.first;
    int which = p.second;

    if (find != -1)
    {
        std::shared_ptr<Description> description(new Description());
        bool adjFound = k->findAdj(sentence, find, description);
        // 在用户配置必须找到形容词的关键词做判定
        if (!adjFound && k->mustAdj)
        {
            return 0;
        }

        // 找出语句中出现的那个近义词
        description->word = k->getSimilarWord(which);
        // std::wcout << L"Matched keyword: " << k->getWord() << std::endl;
        struct tm *t = matchDate(sentence);

        // 更新tempKeywords，用于下一轮匹配 （多轮需求）
        db->tempKeywords = k->getNextKeywords();
        // std::wcout << db->tempKeywords.size() << std::endl;

        // 更正时间
        std::mktime(t);
        description->time = t;
        // std::wcout << std::asctime(description->time) << std::endl;

        wchar_t* r = (k->performAction(description));
        std::wcout << r << std::endl;
        
        delete[] r;

        return 1;
    }
    return 0;
}

// 循环匹配每个关键词
int matchKeywords(std::wstring sentence, std::shared_ptr<Database> db)
{
    for (std::shared_ptr<Keyword> k : db->tempKeywords)
    {
        if (matchKeyword(sentence, k, db) == 1)
            return 1;
    }

    for (std::shared_ptr<Keyword> k : db->headKeywords)
    {
        if (matchKeyword(sentence, k, db) == 1)
            return 1;
    }

    // clear tempKeywords
    db->tempKeywords = std::vector<std::shared_ptr<Keyword>>();
    std::wcout << L"未匹配" << std::endl;
    return 0;
}

// 因含中文数据，需要转换编码并使用wstring（2字节）存储，与C#的string（2字节）相同
// argv[1] is the path to the config file
int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cout << "Usage: " << argv[0] << " <config file>" << std::endl;
        return 1;
    }

    // 设置中文环境
    setlocale(LC_ALL, "zh_CN.UTF-8");

    // 读取配置文件，之后需要改成sql
    std::shared_ptr<Database> db = readConfig(argv[1]);
    FilterDB(db);

    std::wstring input = L"";
    std::wstring exit = L"exit";
    // std::wcout.imbue(std::locale("", LC_CTYPE));

    // 把输入流的编码转换成本地编码，不然windows下会乱码
    std::wcin.imbue(std::locale(""));
    while (true)
    {
        std::wcout << L"请输入：";
        if(!(std::wcin >> input) || input == L"") continue;
        if (input == exit)
            break;
        replaceChineseNum(input);
        matchKeywords(input, db);

        input.clear();
    }

    setlocale(LC_ALL, "C");
    return 0;
}

#endif