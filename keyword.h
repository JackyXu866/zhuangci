// class to store keywords

#ifndef KEYWORD_H
#define KEYWORD_H

#include <vector>
#include <string>
#include <functional>
#include <utility>

#include "util.h"

// 关键词的详细信息，用来返回给接口
class Description
{
public:
    Description() { adjective = L""; };
    std::wstring adjective;
    struct tm *time;
    std::wstring word;
};

// tm 是 C++ 里的时间结构体，用来存储时间
/**
 * struct tm {
 *      int tm_sec;    // 秒，正常范围从 0 到 59，但允许至 61
 *      int tm_min;    // 分，范围从 0 到 59
 *      int tm_hour;   // 小时，范围从 0 到 23
 *      int tm_mday;   // 一月中的第几天，范围从 1 到 31
 *      int tm_mon;    // 月，范围从 0 到 11
 *      int tm_year;   // 自 1900 年起的年数
 *      int tm_wday;   // 一周中的第几天，范围从 0 到 6，从星期日算起
 *      int tm_yday;   // 一年中的第几天，范围从 0 到 365，从 1 月 1 日算起
 *      int tm_isdst;  // 夏令时
 * };
 */

// 关键词类
class Keyword
{
public:
    Keyword(std::wstring word);

    // 修改器 (Modifiers)
    void addSimilarWord(std::wstring similarWord);
    void addAdjective(std::wstring adjective);
    void addPrevKeyword(std::shared_ptr<Keyword> prevKeyword);
    void addNextKeyword(std::shared_ptr<Keyword> nextKeyword);

    // 访问器 (Accessors)
    std::wstring getWord() const;
    std::vector<std::wstring> getSimilarWords() const;
    std::wstring getSimilarWord(int index) const;
    std::vector<std::wstring> getAdjectiveList() const;
    std::vector<std::shared_ptr<Keyword>> getPrevKeywords() const;
    std::vector<std::shared_ptr<Keyword>> getNextKeywords();

    /**
     * @brief 根据关键词的位置，寻找形容词，具体寻找方式参考配置文件
     *
     * @param sentence 句子
     * @param pos 关键词位置
     * @param description 存储详细信息
     * @return true 找到形容词
     * @return false 未找到形容词
     */
    bool findAdj(std::wstring &sentence, int pos, std::shared_ptr<Description> description);

    /**
     * @brief 找到句子里的关键词，返回关键词的位置和关键词在similarWords里的位置
     *
     * @param sentence 句子
     * @return std::pair<int, int> 如果找到，返回pair<位置, similar word(int)>，否则返回<-1, -1>
     */
    std::pair<int, int> match(std::wstring &sentence);
    /**
     * @brief 执行动作，动作可以在外部定义并通过类的action指针传入
     *
     * @param input 描述
     * @return wchar_t* JSON格式的返回值
     */
    wchar_t *performAction(std::shared_ptr<Description> input) const;

    int ADJ_GAP_SPACE; // 形容词与关键词之间的最大间隔
    // 形容词搜索方式，0为从关键词位置开始向前搜索，1为向后搜索，2为双向搜索
    int ADJ_SEARCH_TYPE;
    bool head, tail, mustAdj; // 是否为头，尾，必须有形容词

private:
    std::wstring word;                       // 关键词（技能名称）
    std::vector<std::wstring> similarWords;  // 与关键词相似的词（含关键词本身）
    std::vector<std::wstring> adjectiveList; // 形容词列表
    size_t maxAdjLen;                        // 形容词最大长度
    // std::function<int(std::shared_ptr<Description>)> action; // action to be performed when keyword is detected, takes in adjective

    // 多轮
    // 匹配逻辑：在从数据库读取时获取每个关键词前置及后置关键词
    // 在匹配时，如果当前关键词匹配成功，则将待匹配关键词列表（tempKeywords）等于当前关键词的后置关键词列表(nextKeywords)
    // 如果没有匹配到任意关键词，则将tempKeywords清空
    // 匹配到没有后置的关键词，因为nextKeywords本就是空的，所以tempKeywords也会更新成空的
    // 没有前置的关键词，即head，每轮均会匹配，因为他们加入了headKeywords
    std::vector<std::shared_ptr<Keyword>> prevKeywords; // 前置关键词
    std::vector<std::shared_ptr<Keyword>> nextKeywords; // 后置关键词
    int repeat;                                         // 重复次数（还没用到）
};

#endif // KEYWORD_H