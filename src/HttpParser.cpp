#include "../includes/HttpParser.hpp"

HttpParser::HttpParser()
    : state(REQUEST_LINE), expectedBodySize(0), errorCode(0)
{
    (void) state; // to silent -Werror for now
    (void) expectedBodySize;
    (void) errorCode;
}

HttpParser::~HttpParser() { }
