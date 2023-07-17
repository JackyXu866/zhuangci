// Database to store matching words

#ifndef DATABASE_H
#define DATABASE_H

#include "util.h"
#include <map>

class Keyword;

class Database
{
public:
    Database() { keywords = std::unordered_map<std::wstring, std::shared_ptr<Keyword>>(); };
    std::unordered_map<std::wstring, std::shared_ptr<Keyword>> keywords;
    std::vector<std::shared_ptr<Keyword>> headKeywords; // always query
    std::vector<std::shared_ptr<Keyword>> tempKeywords; // 多轮启动

    std::wstring json_return;
};

#endif // DATABASE_H