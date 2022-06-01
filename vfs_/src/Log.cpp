#include "../include/Log.h"

Log *Log::logInstance = NULL;

Log::~Log() {}

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
