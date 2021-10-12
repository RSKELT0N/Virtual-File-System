#ifndef _LOG_H_
#define _LOG_H_

#include <iostream>
#include <sstream>
#include <ctime>
#include <mutex>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#define DATE std::string((Log::getInstance()->getDate()))
#define TIME std::string((Log::getInstance()->getTime()))
#define DIR std::string("/Users/ryanskelton/Desktop/server_/")
#define FILE_ std::string("Logs_")
#define EXT std::string(".txt")
#define LOG(__type__, __msg__) (Log::getInstance()->logger(__type__, __LINE__, __msg__, ""))

class Log {
public:
    enum Type {
        WARNING,
        INFO,
        MESSAGE,
        DEBUG,
        ERROR,
        SERVER
    };

public:
    ~Log();
    static Log *getInstance();
    Log(const Log &) = delete;
    void operator=(const Log &) = delete;
    void logger(Type, int, std::string, std::string);
    std::string getDate();
    std::string getTime();
    std::unordered_set<std::string> notPriority = {"/list"};
    std::vector<std::string> split(std::string &str, char separator);
private:
    Log() = default;
    static Log *logInstance;
    void writeLog(std::string);
    std::mutex Logging;

    std::unordered_map<Log::Type, std::string> typeMap = {
            {std::make_pair(Type::WARNING, "WARNING")},
            {std::make_pair(Type::INFO, "INFO")},
            {std::make_pair(Type::MESSAGE, "MESSAGE")},
            {std::make_pair(Type::DEBUG, "DEBUG")},
            {std::make_pair(Type::ERROR, "ERROR")},
            {std::make_pair(Type::SERVER, "SERVER")}
    };

    struct Colours {
        const std::string RED = "\033[1;31m";
        const std::string GREEN = "\033[1;92m";
        const std::string YELLOW = "\033[1;93m";
        const std::string BLUE = "\033[1;34m";
        const std::string MAGENTA = "\033[1;95m";
        const std::string CYAN = "\033[1;96m";
        const std::string WHITE = "\033[1;37m";
        const std::string RESET = "\033[1;0m";
    } colour;
};


#endif //LOG_LOG_H