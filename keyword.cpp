#define _CRT_SECURE_NO_WARNINGS

/**
 * @brief 关键词类，用于匹配句子中的关键词并执行相应的动作
 * 
 */
#include "keyword.h"

/**
 * @brief 构造函数，初始化关键词及相关属性
 * 
 * @param word 关键词
 */
Keyword::Keyword(std::wstring word) : word(word)
{
    similarWords = std::vector<std::wstring>();
    similarWords.push_back(word);
    adjectiveList = std::vector<std::wstring>();
    maxAdjLen = 0;
    action = nullptr;
    ADJ_SEARCH_TYPE = 0;
    ADJ_GAP_SPACE = 3;

    prevKeywords = std::vector<std::shared_ptr<Keyword>>();
    nextKeywords = std::vector<std::shared_ptr<Keyword>>();
    head = true;
    tail = true;
    mustAdj = false;
    repeat = 1;
}

/**
 * @brief 添加形容词
 * 
 * @param adjective 形容词
 */
void Keyword::addAdjective(std::wstring adjective)
{
    adjectiveList.push_back(adjective);
    maxAdjLen = std::max(maxAdjLen, adjective.length());
}

/**
 * @brief 根据关键词的位置，寻找形容词，具体寻找方式参考配置文件
 * 
 * @param sentence 句子
 * @param pos 关键词位置
 * @param description 存储详细信息
 * @return true 找到形容词
 * @return false 未找到形容词
 */
bool Keyword::findAdj(std::wstring &sentence, int pos, std::shared_ptr<Description> description)
{
    int begPos, adjLen;
    // 根据ADJ_SEARCH_TYPE调整搜索范围
    switch (ADJ_SEARCH_TYPE)
    {
    case 2:
        begPos = pos - maxAdjLen - ADJ_GAP_SPACE;
        adjLen = (maxAdjLen + ADJ_GAP_SPACE) * 2 + word.length();
        break;
    case 0:
        begPos = pos - maxAdjLen - ADJ_GAP_SPACE;
        adjLen = maxAdjLen + ADJ_GAP_SPACE;
        break;
    case 1:
        begPos = pos + word.length();
        adjLen = maxAdjLen + ADJ_GAP_SPACE;
        break;
    default:
        begPos = pos - maxAdjLen - ADJ_GAP_SPACE;
        adjLen = (maxAdjLen + ADJ_GAP_SPACE) * 2 + word.length();
        break;
    }
    // 如果超出范围，调整搜索范围
    if (begPos < 0)
    {
        adjLen += begPos;
        begPos = 0;
    }
    if ((size_t)(begPos + adjLen) > sentence.length())
    {
        adjLen -= begPos + adjLen - sentence.length();
    }

    std::wstring tmp = sentence.substr(begPos, adjLen);

    for (std::wstring adjective : adjectiveList)
    {
        std::wregex adjectiveMatch(adjective);
        if (std::regex_search(tmp, adjectiveMatch))
        {
            // std::wcout << L"Matched adjective: " << adjective << std::endl;
            description->adjective = adjective;
            return true;
        }
    }

    return false;
}

/**
 * @brief 找到句子里的关键词，返回关键词的位置和关键词在similarWords里的位置
 * 
 * @param sentence 句子
 * @return std::pair<int, int> 如果找到，返回pair<位置, similar word(int)>，否则返回<-1, -1>
 */
std::pair<int, int> Keyword::match(std::wstring &sentence)
{
    std::wsmatch match;
    for (int i = 0; i < similarWords.size(); i++)
    {
        std::wstring similarWord = similarWords[i];
        std::wregex similarWordMatch(similarWord);
        if (std::regex_search(sentence, match, similarWordMatch))
        {
            // std::wcout << L"Matched keyword: " << similarWord << std::endl;

            int pos = match.position(0);

            return std::make_pair(pos, i);
        }
    }
    return std::make_pair(-1, -1);
}

/**
 * @brief 执行动作，动作可以在外部定义并通过类的action指针传入
 * 
 * @param input 描述
 * @return int 返回值
 */
int Keyword::performAction(std::shared_ptr<Description> input) const
{
    std::wcout << L"技能名称: " << word << std::endl;
    std::wcout << L"具体词语: " << input->word << std::endl;
    std::wcout << L"技能时间: " << std::asctime(input->time);
    if (input->adjective != L"")
        std::wcout << L"技能形容/类别: " << input->adjective << std::endl;
    else
        std::wcout << L"技能形容/类别: " << L"无" << std::endl;
    if (head != true)
        std::wcout << L"经过前置判定" << std::endl;
    std::wcout << std::endl;

    if (action != nullptr)
        action(input);

    return 0;
}
