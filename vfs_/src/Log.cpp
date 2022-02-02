//
// Created by Ryan Skelton on 01/10/2020.
//

#include "../include/Log.h"

std::vector<std::string> split(const char* line, char sep) noexcept {
    std::vector<std::string> tokens;
    std::stringstream ss(line);
    std::string x;

    while ((getline(ss, x, sep))) {
        if (x != "")
            tokens.push_back(x);
    }
    return tokens;
}

Log *Log::logInstance = NULL;

Log::~Log()
{
    printf("Deleted Logger\n");
}

Log *Log::getInstance()
{
    if (logInstance == NULL)
        logInstance = new Log();
    return logInstance;
}

void Log::logger(Type type, int lineNumber, std::string log, std::string col)
{
    std::stringstream ss;
    switch (type)
    {
    case Log::WARNING:
        col = Log::colour.YELLOW;
        break;
    case Log::INFO:
        col = Log::colour.GREEN;
        break;
    case Log::SERVER:
        col = Log::colour.MAGENTA;
        break;
    case Log::DEBUG:
        col = Log::colour.BLUE;
        break;
    case Log::ERROR_:
        col = Log::colour.RED;
        break;
    }

    lineNumber = 0;

    ss << colour.RESET;
    ss << "[ " << col << typeMap[type] << colour.RESET << " ] ";
    ss << log;
    ss << colour.RESET;

    std::cout << ss.str() << "\n";

    if(type == Type::ERROR_)
        exit(EXIT_FAILURE);
}

std::string Log::logger_str(Type type, int lineNumber, std::string log, std::string col) {
    std::stringstream ss;
    switch (type)
    {
    case Log::WARNING:
        col = Log::colour.YELLOW;
        break;
    case Log::INFO:
        col = Log::colour.GREEN;
        break;
    case Log::SERVER:
        col = Log::colour.MAGENTA;
        break;
    case Log::DEBUG:
        col = Log::colour.BLUE;
        break;
    case Log::ERROR_:
        col = Log::colour.RED;
        break;
    }

    lineNumber = 0;

    ss << colour.RESET;
    ss << "[ " << col << typeMap[type] << colour.RESET << " ] ";
    ss << log;
    ss << colour.RESET;
    ss << "\n";

    if(type == Type::ERROR_)
        exit(EXIT_FAILURE);
    return ss.str();
}
