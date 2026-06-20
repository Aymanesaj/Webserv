#include "../includes/Utils.hpp"

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

std::string Utils::getCurrentDate( void )
{
    time_t      now = time(0);
    struct tm   tm_now;
    char        buf[100];
    gmtime_r(&now, &tm_now);
    strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", &tm_now);
    return std::string(buf);
}

time_t  Utils::getCurrentTime( void )
{
    time_t      now = time(0);
    gmtime_r(&now, NULL);
    return now;
}

std::string Utils::to_string_c98(const int& value)
{
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

void    Utils::replace(std::string& str, const std::string& old, const std::string& new_str)
{
    size_t pos = str.find(old);
    if (pos != std::string::npos)
        str.replace(pos, old.size(), new_str);
}

bool Utils::isFileExists(const std::string& path)
{
    struct stat buffer;
    return (stat(path.c_str(), &buffer) == 0);
}

bool Utils::is_Directory(const std::string& path)
{
    struct stat info;
    if (stat(path.c_str(), &info) != 0)
        return false;
    return (info.st_mode & S_IFDIR) != 0;
}

bool Utils::is_Writable(const std::string& path)
{
    return (access(path.c_str(), W_OK) == 0);
}

/* 
* Extracts one line (ending in \r\n).
* If the buffer doesn't have \r\n, it reads more from the fd until it does.
*/
bool Utils::extractLine(std::vector<char>& buffer, int fd, std::string& line) {
    line.clear();
    const char crlf[] = {'\r', '\n'};
    
    while (true) {
        // Look for \r\n in buffer
        std::vector<char>::iterator it = std::search(buffer.begin(), buffer.end(), crlf, crlf + 2);
        
        if (it != buffer.end()) {
            line.assign(buffer.begin(), it); // Assign data to string (excluding \r\n)
            buffer.erase(buffer.begin(), it + 2);
            return true;
        }
        
        char tmp[8192];
        ssize_t bytes = read(fd, tmp, sizeof(tmp));
        if (bytes <= 0) {
            if (!buffer.empty()) {
                line.assign(buffer.begin(), buffer.end());
                buffer.clear();
                return true;
            }
            return false;
        }
        buffer.insert(buffer.end(), tmp, tmp + bytes);
    }
}

std::string Utils::getExtension(const std::string& ContentType)
{
    if (ContentType == "text/html")
        return ".html";
    else if (ContentType == "text/plain")
        return ".txt";
    else if (ContentType == "image/jpeg")
        return ".jpg";
    else if (ContentType == "image/png")
        return ".png";
    else if (ContentType == "video/mp4")
        return ".mp4";
    else if (ContentType == "application/pdf")
        return ".pdf";
    else
        return "";
}
