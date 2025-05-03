#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstring>
#include <fstream>
#include <regex>
#include <thread>
#include <unordered_map>
#include <iostream>
#include "getlyrics.hpp"
#include "curl/easy.h"
#include "nlohmann/json_fwd.hpp"
#include <exception>
#include <nlohmann/json.hpp>
#include <curl/curl.h>
#include <cstddef>
#include <string>

size_t getlyrics::HeaderCallback(char* buffer, size_t size, size_t nitems, void* userdata) {
    size_t totalSize = size * nitems;
    std::string* headers = static_cast<std::string*>(userdata);
    headers->append(buffer, totalSize);
    return totalSize;
}
size_t getlyrics::WriteCallback(void* contents, size_t size, size_t chuncks, std::string* songLyrics){
    size_t totalSize = size * chuncks;
    songLyrics->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}
std::string getlyrics::httpGet(const std::string& url, const std::string& token_api){
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
        std::string headerData;
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, HeaderCallback);
        curl_easy_setopt(curl, CURLOPT_HEADERDATA, &headerData);
        curl_easy_perform(curl);
        long responseCode = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);
        if (responseCode == 429){
            std::smatch match; 
            std::regex retry("Retry-After: (\\d+)");
            if (std::regex_search(headerData, match, retry)){
                int retryAfter = std::stoi(match[1]);
                std::cout << "httpGet rate limit " << match[1] << "\n";
                std::this_thread::sleep_for(std::chrono::seconds(retryAfter));
                curl_easy_cleanup(curl);
                curl_slist_free_all(header);
                return httpGet(url, token_api);
            }
        }
        curl_easy_cleanup(curl);
        curl_slist_free_all(header);

    }
   return response; 
}

void getlyrics::scrapeLyrics(const std::string& artist){
    char* raw_query = curl_easy_escape(nullptr, (artist).c_str(), 0);
    std::string query = raw_query;
    curl_free(raw_query);
    std::string url = "https://api.genius.com/search?q=" + query;
    std::string json = httpGet(url, tokenApiGenius);
    auto jsonData = nlohmann::json::parse(json);
    std::string songURL;
    std::string songName;
    std::string songArtist;
    try{
        for (const auto& hit : jsonData["response"]["hits"]){
            std::string artistName = hit["result"]["primary_artist"]["name"];
            songURL = hit["result"]["url"];
            songName = hit["result"]["title"];
            songArtist = hit["result"]["artist_names"];
            hashMapURL[artistName].push_back({songURL,songName, songArtist});
        }
    }
    catch(const std::exception& e){
        std::cerr << "error occurred when passing json check valid artist name"<< e.what();

    }

    std::string songLyrics;
    std::string spotifyApiToken = httpPostToken(clientScrt, clienetID);
    for (const auto& [song, name, properArtistName] : hashMapURL[artist]){

        
        std::string htmlData = httpGet(song, tokenApiGenius);
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
        songLyrics = std::regex_replace(songLyrics, std::regex("\""), "\"\"");
        std:: string songCopy = song.substr(19);
        songCopy = std::regex_replace(songCopy, std::regex(R"(-)"), " ");
        std::string artistCopy = artist;
        std::transform(artistCopy.begin(),artistCopy.end(), artistCopy.begin(), [](unsigned char c){return std::tolower(c);});
        std::transform(artistCopy.begin(), artistCopy.begin() + 1, artistCopy.begin(), [](unsigned char c) { return std::toupper(c);});
        songCopy = std::regex_replace(songCopy, std::regex(R"(-)"), " ");
        songCopy = std::regex_replace(songCopy, std::regex(artistCopy), "");
        songCopy = std::regex_replace(songCopy, std::regex("\\blyrics\\b"), "");
        std::string songLyricsQuoted = "\"" + songLyrics + "\"";
        addtocsv(songLyricsQuoted, properArtistName, name, spotifyApiToken);
        songLyrics = "";
    }
    std::cout << artist + " completed successfully"<< "\n";

}

void getlyrics::addtocsv(const std::string& lyrics, const std::string& artist, const std::string& title, const std::string& token_api){
    std::string artistName = "Artist Name";
    std::string songTitle = "Song Title";
    std::string songLyrics = "Song Lyrics";
    std::string songMood = "Song Sentiment Tag";
    std::string filePath = "../../data/Song_DataSet.csv";
    std::string mood = getMood(token_api, artist, title);
    if (mood == "no track id found"){
        return;
    }
    std::string quotesartist = "\"" + artist + "\"";
    std::string quotestitle = "\"" + title + "\"";

    std::ifstream infile(filePath);
    bool isEmpty = infile.peek() == std::ifstream::traits_type::eof();
    std::ofstream csvFile(filePath, std::ios::app);
    if (csvFile.is_open()){
        if (isEmpty){
            csvFile << artistName << "," << songTitle << "," << songLyrics << "," << songMood << "\n";
        }
        csvFile << quotesartist << "," << quotestitle << "," << lyrics << "," << mood << "\n";
        csvFile.close();
    }
    
}

std::string getlyrics::httpPostToken(const std::string& clientScrt, const std::string& clientID){
    static std::string cachedToken;
    static std::chrono::steady_clock::time_point tokenTime;
    constexpr int tokenTTL = 3570; // subtracted 30 sec to give buffer to prevent invalid token during api callsF
    std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();

    if (!cachedToken.empty()){ // checks if token is already initialized if so then return token if it isn't expired
        if (std::chrono::duration_cast<std::chrono::seconds>(now - tokenTime).count() < tokenTTL){
            return cachedToken;
        }
    }

    CURL* curl = curl_easy_init();
    std::string response;
    std::string url = "https://accounts.spotify.com/api/token";
    std::string payload = "grant_type=client_credentials&client_id=" +clientID + "&client_secret=" + clientScrt;
    if (curl){
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        struct curl_slist* header = nullptr;
        std::string headers = "Content-Type: application/x-www-form-urlencoded";
        header = curl_slist_append(header,headers.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        curl_slist_free_all(header);

    }
    nlohmann::json jsonRes = nlohmann::json::parse(response);
    cachedToken = jsonRes["access_token"];
    tokenTime = now;
    return cachedToken;
}

std::string getlyrics::getMood(const std::string& token_api, const std::string& artist, const std::string& title){
    char* rawQuery = curl_easy_escape(nullptr, (title).c_str(), 0);
    std::string query = rawQuery;
    curl_free(rawQuery);
    char* raw_query1 = curl_easy_escape(nullptr, (artist).c_str(), 0);
    std::string query1 = raw_query1;
    curl_free(raw_query1);
    std::string url = "https://api.spotify.com/v1/search?q=track:" + query+"%20artist:"+query1 + "&type=track&limit=1";

    std::string response = httpGet(url, token_api);
    try {
        nlohmann::json jsonResponse = nlohmann::json::parse(response);
        if (jsonResponse["tracks"]["items"].empty()) return "no track id found";
        std::cout << jsonResponse["tracks"]["items"][0]["name"] << "\n";
        std::string trackID = jsonResponse["tracks"]["items"][0]["id"];
        std::string valenceURL = "https://www.chosic.com/api/tools/audio-features/" + trackID;
        std::string valenceRes = httpGetValence(valenceURL);
        nlohmann::json jsonValenceRese = nlohmann::json::parse(valenceRes);
        float valence = jsonValenceRese["valence"];

        if (valence > 0.9) return "Extremely Happy";
        else if (valence > 0.8) return "Very Happy";
        else if (valence > 0.7) return "Pretty Happy";
        else if (valence > 0.6) return "Slightly Happy";
        else if (valence > 0.5) return "Neutral-Happy";
        else if (valence == 0.5) return "Neutral";
        else if (valence > 0.4) return "Neutral-Sad";
        else if (valence > 0.3) return "Slightly Sad";
        else if (valence > 0.2) return "Pretty Sad";
        else if (valence > 0.1) return "Very Sad";
        else return "Extremely Sad";
               
    }
    catch (...){
        return "error with passing json";
    }
    return "";
}
std::string getlyrics::httpGetValence(const std::string& url){
    CURL* curl = curl_easy_init();
    std::string response;

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:137.0) Gecko/20100101 Firefox/137.0");
        headers = curl_slist_append(headers, "Accept: application/json, text/javascript, */*; q=0.01");
        headers = curl_slist_append(headers, "Accept-Language: en-US,en;q=0.5");
        headers = curl_slist_append(headers, "Accept-Encoding: gzip, deflate, br, zstd");
        headers = curl_slist_append(headers, "Connection: keep-alive");
        headers = curl_slist_append(headers, "Cookie: pll_language=en");
        headers = curl_slist_append(headers, "Sec-Fetch-Dest: empty");
        headers = curl_slist_append(headers, "Sec-Fetch-Mode: no-cors");
        headers = curl_slist_append(headers, "Sec-Fetch-Site: same-origin");
        headers = curl_slist_append(headers, "X-WP-Nonce: 0951b68167");
        headers = curl_slist_append(headers, "app: genre_finder");
        headers = curl_slist_append(headers, "X-Requested-With: XMLHttpRequest");
        headers = curl_slist_append(headers, "Pragma: no-cache");
        headers = curl_slist_append(headers, "Cache-Control: no-cache");
        headers = curl_slist_append(headers, "Priority: u=4");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_REFERER, "https://www.chosic.com/music-genre-finder/?track=285pBltuF7vW8TeWk8hdRR");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        std::string headerData;
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, HeaderCallback);
        curl_easy_setopt(curl, CURLOPT_HEADERDATA, &headerData);
        curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "");
        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << "\n";
        }
        long responseCode = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);
        if (responseCode == 429){
            std::smatch match; 
            std::regex retry("Retry-After: (\\d+)");
            if (std::regex_search(headerData, match, retry)){
                int retryAfter = std::stoi(match[1]);
                std::cout << "valence rate limit " << match[1] << "\n";
                std::this_thread::sleep_for(std::chrono::seconds(retryAfter));
                curl_easy_cleanup(curl);
                curl_slist_free_all(headers);
                return httpGetValence(url);
            }
        }

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);

        }
    return response;
    
}


