#ifndef UTILS_HPP
#define UTILS_HPP

#include "libs.hpp"

class Utils
{
    private:
        Utils();
    public:
        static  std::vector<std::string>    split(const std::string& str,
                                                const std::string& delimiter);
        static  bool    isAllUpper(const std::string& str);
        static  bool    isAllSpaces(const std::string& str);
        static  void    capitalizeWord(std::string& word);
        static  void    trim(std::string& str);
};

#endif
