#include <fstream>
#include <regex>
#include <vector>
#include <unordered_map>
#include <iostream>
#include "getlyrics.hpp"
#include "curl/easy.h"
#include <exception>
#include <nlohmann/json.hpp>
#include <curl/curl.h>
#include <cstddef>
#include <string>

getlyrics::getlyrics(const std::string& token): token_api(token){}

size_t getlyrics::WriteCallback(void* contents, size_t size, size_t chuncks, std::string* songLyrics){
    size_t totalSize = size * chuncks;
    songLyrics->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

std::string getlyrics::httpGet(const std::string& url){
    CURL* curl = curl_easy_init();
    std::string response;

    if (curl){
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        struct curl_slist* header = nullptr;
        std::string headers = "Authorization: Bearer " + token_api;
        header = curl_slist_append(header,headers.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        curl_slist_free_all(header);

    }
   return response; 
}

void getlyrics::scrapeLyrics(const std::string& artist, const std::string& title){
    std::string query = curl_easy_escape(nullptr, (artist).c_str(), 0);
    std::string url = "https://api.genius.com/search?q=" + query;
    std::string json = httpGet(url);
    auto jsonData = nlohmann::json::parse(json);
    std::string songURL;
    try{
        for (const auto& hit : jsonData["response"]["hits"]){
            std::string artistName = hit["result"]["primary_artist"]["name"];
            songURL = hit["result"]["url"];
            hashMapURL[artistName].push_back(songURL);
        }
    }
    catch(const std::exception& e){
        std::cerr << "error occurred when passing json check valid artist name"<< e.what();

    }

    std::vector<std::string>& songs = hashMapURL[artist];
    std::string songLyrics;
    for (const auto& song : songs){

        
        std::string htmlData = httpGet(song);
        std::regex htmlPattern(R"(<span class="ReferentFragment-desktop__Highlight-sc-[^"]*">([\s\S]*?)<\/span>)");
        auto lyricBegin = std::sregex_iterator(htmlData.begin(), htmlData.end(), htmlPattern);
        auto lyricEnd = std::sregex_iterator();
        for (std::sregex_iterator i = lyricBegin; i!= lyricEnd; i++){
            std::smatch match = *i;
            songLyrics += match[1].str() + " ";
        }
        songLyrics = std::regex_replace(songLyrics, std::regex(R"(&#x27;)"), "'"); // apostrophe
        songLyrics = std::regex_replace(songLyrics, std::regex(R"(&quot;)"), "\"");
        songLyrics = std::regex_replace(songLyrics, std::regex(R"(&amp;)"), "&");
        songLyrics = std::regex_replace(songLyrics, std::regex(R"(&lt;)"), "<");
        songLyrics = std::regex_replace(songLyrics, std::regex(R"(&gt;)"), ">");
        songLyrics = std::regex_replace(songLyrics, std::regex(R"(\s{2,})"), " ");
        songLyrics = std::regex_replace(songLyrics, std::regex(R"(<i>)"), " ");
        songLyrics = std::regex_replace(songLyrics, std::regex(R"(</i>)"), " ");
        songLyrics = std::regex_replace(songLyrics, std::regex(R"(<br/>)"), " ");
        addtocsv(songLyrics, artist, song.substr(19));
        songLyrics = "";
    }






}

void getlyrics::addtocsv(const std::string& lyrics, const std::string& artist, const std::string& title){
    std::string artistName = "Artist Name";
    std::string songTitle = "Song Title";
    std::string songLyrics = "Song Lyrics";
    std::string songMood = "Song Sentiment Tag";
    std::string filePath = "../../data/Song_DataSet.csv";
    std::ifstream infile(filePath);
    std::string mood;
    bool isEmpty = infile.peek() == std::ifstream::traits_type::eof();
    std::ofstream csvFile(filePath, std::ios::app);
    if (csvFile.is_open()){
        if (isEmpty){
            csvFile << artistName << "," << songTitle << "," << songLyrics << "," << songMood << "\n";
        }
        csvFile << artist << "," << title << "," << lyrics << "," << mood << "\n";
        csvFile.close();
    }
    
}







