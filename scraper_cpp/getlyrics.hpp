#pragma once
#include <curl/curl.h>
#include <cstddef>
#include <string>
#include <nlohmann/json.hpp>
#include <unordered_map>
#include <vector>


class getlyrics{
    private:
        std::unordered_map<std::string,std::vector<std::string>> hashMapURL;
        std::string token_api;
        static size_t WriteCallback(void* contents, size_t size, size_t chunks, std::string* output);
        std::string httpGet(const std::string& url);
    
    public:
        getlyrics(const std::string& token);
        void addtocsv(const std::string& lyrics, const std::string& artist, const std::string& title);
        void scrapeLyrics(const std::string& artist, const std::string& title);
};
