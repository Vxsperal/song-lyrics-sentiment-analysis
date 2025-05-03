#pragma once
#include <cstring>
#include <curl/curl.h>
#include <cstddef>
#include <string>
#include <nlohmann/json.hpp>
#include <unordered_map>
#include <vector>


class getlyrics{
    private:
        struct songinfo{
            std::string URL;
            std::string name;
            std::string artist;
        };
        std::unordered_map<std::string,std::vector<songinfo>> hashMapURL;
        std::string clientScrt = "";
        std::string clienetID = "";
        std::string tokenApiGenius = "";
        static size_t WriteCallback(void* contents, size_t size, size_t chunks, std::string* output);
        static size_t HeaderCallback(char* buffer, size_t size, size_t nitems, void* userdata);
        std::string httpGet(const std::string& url, const std::string& token_api);
        std::string httpPostToken(const std::string& clientScrt, const std::string& clienetID);
        std::string getMood(const std::string& token_api, const std::string& artist, const std::string& title);
        std::string httpGetValence(const std::string& url);
     
    public:
        void addtocsv(const std::string& lyrics, const std::string& artist, const std::string& title, const std::string& token_api);
        void scrapeLyrics(const std::string& artist);
};
