#include "Log.h"

Log *Log::logInstance = NULL;

Log::~Log() {
    delete logInstance;
}

Log* Log::getInstance() {
    if(logInstance == NULL)
        logInstance = new Log();
    return logInstance;
}

void Log::logger(Type type, int lineNumber, std::string log, std::string col) {
    Logging.lock();
    std::stringstream ss;
    switch(type) {
        case Log::WARNING: col = Log::colour.YELLOW; break;
        case Log::INFO: col = Log::colour.WHITE; break;
        case Log::MESSAGE: col = Log::colour.BLUE; break;
        case Log::DEBUG: col = Log::colour.MAGENTA; break;
        case Log::ERROR: col = Log::colour.RED; break;
        case Log::SERVER: col = Log::colour.GREEN; break;
    }

    ss << col << "[" << "LINE:" << lineNumber << "] ";
    ss << "[" << "<<" << typeMap[type] << ">>" << "] ";
    ss << "[" << TIME << "] ";
    ss << "[" << log << "]";
    ss << Log::colour.RESET;


    auto a = split(log,' ');
    if(notPriority.find(a[a.size()-1]) == notPriority.end()) {
        std::cout << ss.str() << "\n";
    }
    writeLog(ss.str());
    Logging.unlock();

    if(type == Log::ERROR)
        exit(EXIT_FAILURE);
}

void Log::writeLog(std::string log) {
    std::string newStr = "";
    std::ofstream file(DIR+FILE_+DATE+EXT, std::ios::app);
    for (int i = 7; i < (int)log.size() - 6; i++)
        newStr += log[i];
    file << newStr << "\r\n";
}

std::string Log::getDate() {
    time_t now = time(0);
    std::string res = (char *) (ctime(&now));
    res = res.substr(0, 10);
    for (int i = 0; i < (int) res.size(); i++)
        if (res[i] == ' ') {
            res[i] = '_';
        }
    return res;
}

std::string Log::getTime() {
    time_t now = time(0);
    std::string res = (char *)(ctime(&now));
    res = res.substr(11,8);
    return res;
}

std::vector<std::string> Log::split(std::string &str, char separator) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string x;
    while(getline(ss, x, separator))
        if(x != "")
            tokens.emplace_back(x);
        return tokens;
}