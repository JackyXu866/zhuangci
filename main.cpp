

#define _CRT_SECURE_NO_WARNINGS
#define CPPHTTPLIB_OPENSSL_SUPPORT
#ifndef MAIN_CPP
#define MAIN_CPP

#include <fstream>

#include "time.h"
#include "util.h"

//  读取配置文件，之后需迁移
std::shared_ptr<Database> readConfig(char *config) {
    std::wifstream file(config);
    if (!file.is_open()) {
        std::cout << "Error opening file " << config << std::endl;
        return nullptr;
    }
    file.imbue(std::locale("zh_CN.UTF-8"));

    std::shared_ptr<Database> db(new Database);

    std::wstring line;
    std::shared_ptr<Keyword> keyword = nullptr;

    int type = 0;  // 0: key, 1: adj, 2: prev, 3: similarWord
    while (std::getline(file, line)) {
        // match type
        if (line == L"key:") {
            type = 0;
        }
        else if (line == L"adj:") {
            type = 1;
        }
        else if (line == L"prev:") {
            type = 2;
        }
        else if (line == L"") {
            type = -1;
        }
        else {  // information
            replaceChineseNum(line);
            if (type == 0) {
                if (db->keywords.find(line) != db->keywords.end()) {
                    keyword = db->keywords[line];
                }
                else {
                    keyword = std::shared_ptr<Keyword>(new Keyword(line));
                    db->keywords[line] = keyword;
                }
                type = 3;
            }
            else if (type == 1) {
                // adj 配置: s:1 1 2（必须形容词 搜索方式 间隔）
                if (line.substr(0, 2) == L"s:") {
                    int tmp = line[2] - L'0';
                    keyword->mustAdj = (tmp == 1);
                    keyword->ADJ_SEARCH_TYPE = line[4] - L'0';
                    keyword->ADJ_GAP_SPACE = std::stoi(line.substr(6));

                    continue;
                }
                keyword->addAdjective(line);
            }
            else if (type == 2) {
                if (db->keywords.find(line) == db->keywords.end()) {
                    std::shared_ptr<Keyword> prevKeyword(new Keyword(line));
                    db->keywords[line] = prevKeyword;
                }
                keyword->addPrevKeyword(db->keywords[line]);
                db->keywords[line]->addNextKeyword(keyword);

                keyword->head = false;
                db->keywords[line]->tail = false;
            }
            else {
                keyword->addSimilarWord(line);
            }
        }
        line.clear();
    }

    file.close();
    return db;
}

// 筛选出不含前置需求（多轮需求）的关键词
void FilterDB(std::shared_ptr<Database> db) {
    db->headKeywords = std::vector<std::shared_ptr<Keyword>>();
    // db->tempKeywords = std::vector<std::shared_ptr<Keyword>>();

    for (std::pair<std::wstring, std::shared_ptr<Keyword>> p : db->keywords) {
        std::shared_ptr<Keyword> k = p.second;
        if (k->head) {
            db->headKeywords.push_back(k);
        }
    }
}

// 获取用户输入的时间有关信息，并根据现在本地时间更新新时间
struct tm *matchDate(std::wstring sentence) {
    // get current tm
    time_t rawtime;
    time(&rawtime);
    struct tm *timeinfo = localtime(&rawtime);

    // match date
    if (matchDay(sentence, timeinfo)) {
        // std::wcout << L"Matched day" << std::endl;
    }
    else if (matchWeek(sentence, timeinfo)) {
        // std::wcout << L"Matched week" << std::endl;
    }
    else if (matchDate(sentence, timeinfo)) {
        // std::wcout << L"Matched date" << std::endl;
    }

    return timeinfo;
}

// 匹配单个关键词并进一步更新具体信息
int matchKeyword(std::wstring sentence, std::shared_ptr<Keyword> k,
                 std::shared_ptr<Database> db) {
    std::pair<int, int> p = k->match(sentence);
    int find = p.first;
    int which = p.second;

    if (find != -1) {
        // 直接回答
        if (k->respond) {
            std::wstring r = (k->performAction(nullptr));
            std::wcout << r << std::endl;

            db->json_return = r;

            return 1;
        }

        std::shared_ptr<Description> description(new Description());
        bool adjFound = k->findAdj(sentence, find, description);
        // 在用户配置必须找到形容词的关键词做判定
        if (!adjFound && k->mustAdj) {
            return 0;
        }

        // 找出语句中出现的那个近义词
        description->word = k->getSimilarWord(which);
        struct tm *t = matchDate(sentence);

        // 更新tempKeywords，用于下一轮匹配 （多轮需求）
        db->tempKeywords = k->getNextKeywords();

        // 更正时间
        std::mktime(t);
        description->time = t;

        std::wstring r = (k->performAction(description));
        std::wcout << r << std::endl;

        db->json_return = r;

        return 1;
    }
    return 0;
}

// 循环匹配每个关键词
int matchKeywords(std::wstring sentence, std::shared_ptr<Database> db) {
    for (std::shared_ptr<Keyword> k : db->tempKeywords) {
        if (matchKeyword(sentence, k, db) == 1) return 1;
    }

    for (std::shared_ptr<Keyword> k : db->headKeywords) {
        if (matchKeyword(sentence, k, db) == 1) return 1;
    }

    // clear tempKeywords
    db->tempKeywords = std::vector<std::shared_ptr<Keyword>>();
    db->json_return = L"{}";
    return 0;
}

// 因含中文数据，需要转换编码并使用wstring（2字节）存储，与C#的string（2字节）相同
// argv[1] is the path to the config file
int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <config file>" << std::endl;
        return 1;
    }

    // 设置中文环境
    setlocale(LC_ALL, "zh_CN.UTF-8");

    // 读取配置文件，之后需要改成sql
    std::shared_ptr<Database> db = readConfig(argv[1]);
    readCSV_skill("skill.csv", db);
    FilterDB(db);

    std::wstring input = L"";
    std::wstring exit = L"exit";

    // 把输入流的编码转换成本地编码，不然windows下会乱码
    std::wcin.imbue(std::locale(""));

    // 构建http服务器
    httplib::Server svr;
    std::wcout << L"服务器已启动" << std::endl;

    // http post 请求，接收用户输入 (localhost:8080/input)-> body: 内容
    svr.Post(
        "/input", [&](const httplib::Request &req, httplib::Response &res) {
            std::string input = req.body;
            std::wstring winput =
                std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(
                    input);
            std::wcout << L"收到请求：" << winput << std::endl;

            // 退出
            if (winput == exit) {
                std::wcout << L"停止服务器" << std::endl;
                svr.stop();
            }

            replaceChineseNum(winput);
            matchKeywords(winput, db);
            std::string output =
                std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(
                    db->json_return);
            res.set_content(output, "application/json");
        });

    svr.listen("0.0.0.0", 8080);
    setlocale(LC_ALL, "C");

    return 0;
}
#endif