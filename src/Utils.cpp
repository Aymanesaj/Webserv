#include "Utils.hpp"

std::vector<std::string> Utils::split(const std::string& str,
                                      const std::string& delimiter)
{
    std::vector<std::string> result;
    if (delimiter.empty())
        return result;
    std::string token;
    size_t start = 0;
    size_t end;
    while ((end = str.find(delimiter, start)) != std::string::npos)
    {
        token = str.substr(start, end - start);
        if (!token.empty() && !isAllSpaces(token))
            result.push_back(token);
        start = end + delimiter.length();
    }
    token = str.substr(start);
    if (!token.empty() && !isAllSpaces(token))
        result.push_back(token);
    return result;
}

bool    Utils::isAllUpper(const std::string& str)
{
    for (size_t i = 0; i < str.size(); i++)
    {
        if (std::islower(str[i]))
            return false;
    }
    return true;
}

bool Utils::isAllSpaces(const std::string& str)
{
    for (size_t i = 0; i < str.size(); i++)
	    if (!std::isspace(static_cast<unsigned char>(str[i])))
		    return false;
    return true;
}

void    Utils::trim(std::string& str)
{
    if (str.empty())
        return ;
    int i = 0;
    while (str[i] && std::isspace(str[i])) ++i;
    if (!str[i]){
        str = "";
        return ;
    }
    int j = str.length() - 1;
    while (j > 0 && std::isspace(str[j])) --j;
    str = str.substr(i, j - i + 1);
}

void    Utils::capitalizeWord(std::string& word)
{
    if (word.empty())
        return ;
    word[0] = std::toupper(word[0]);
    for (size_t i = 1; i < word.length(); i++)
        word[i] = std::tolower(word[i]);
}
