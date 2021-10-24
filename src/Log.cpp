//
// Created by Ryan Skelton on 01/10/2020.
//

#include "Log.h"

Log *Log::logInstance = NULL;

Log::~Log()
{
    delete logInstance;
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
    case Log::MESSAGE:
        col = Log::colour.MAGENTA;
        break;
    case Log::DEBUG:
        col = Log::colour.BLUE;
        break;
    case Log::ERROR:
        col = Log::colour.RED;
        break;
    }

    lineNumber = 0;

    ss << colour.RESET;
    ss << "[ " << col << typeMap[type] << colour.RESET << " ] ";
    ss << log;
    ss << colour.RESET;

    std::cout << ss.str() << "\n";

    if(type == Type::ERROR)
        exit(EXIT_FAILURE);
}
