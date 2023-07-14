#include "string"


#include <iostream>
#include <locale>
#include <string>
#include <codecvt>

int main(){
    // std::locale::global(std::locale("zh_CN.UTF-8"));
    // std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;

    // std::basic_string<wchar_t> str = L"你好";
    // wchar_t *p(str.data());
    // std::wcout << p << std::endl;

    // std::wstring str2 = L"你不好";
    // wchar_t *p2(str2.data());
    // std::wcout << p2 << std::endl;
    // std::wstring str3(p2);
    // std::wcout << str3 << std::endl;

    std::string a = "aabb你好";
    std::cout << a.substr(0, 3) << std::endl;
    std::cout << a.substr(4, 3) << std::endl;

    return 0;
}