#include <algorithm>
#include <cctype>
#include <chrono>
#include <csetjmp>
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
// headerCallback method for getting the header response from the requested server
size_t getlyrics::HeaderCallback(char* buffer, size_t size, size_t nitems, void* userdata) {// setting the arguments for curl to handle the response
    size_t totalSize = size * nitems;// get the total size of the reponse header
    std::string* headers = static_cast<std::string*>(userdata);// static cast void type user data to the headers std::string* idk could of removed this line by making userdata std::string*
    headers->append(buffer, totalSize);// append the buffer which is the content of the header response to the headers std::string*
    return totalSize;//return total size as method return type
}
//same thing as the headerCallback method but this is for the response body
size_t getlyrics::WriteCallback(void* contents, size_t size, size_t chuncks, std::string* songLyrics){
    size_t totalSize = size * chuncks;
    songLyrics->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}
std::string getlyrics::httpGet(const std::string& url, const std::string& token_api){// this is the method that creates the get request for the two apis used the genius api and the spotify api
    CURL* curl = curl_easy_init();// create a curl handle in order to use the curl setopt functions
    std::string response;//create a response string

    if (curl){//check if curl pointer is not nullptr and handle is created successfully
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());//pass the URL from std::string to char* and set the curl funtion to send req to that url
        struct curl_slist* header = nullptr;// set the curl_slist struct to nullptr
        std::string headers = "Authorization: Bearer " + token_api;//set the header used for api calls
        header = curl_slist_append(header,headers.c_str());//append the headers to the header pointer
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);//set the header pointer ready to send in get req
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);// set the method of the write callback address since it is static the method can access it without initializing a class obj
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);// pass the address where to store the response data
        std::string headerData;// header data string to store
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, HeaderCallback);//same stuff as response data above
        curl_easy_setopt(curl, CURLOPT_HEADERDATA, &headerData);
        curl_easy_perform(curl);//send the get request
        long responseCode = 0;// get the response status code 0 for default
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);//get the response code value
        if (responseCode == 429){//if being rate limited
            std::smatch match; // match variable
            std::regex retry("Retry-After: (\\d+)");//use regex to find the value of the retry value in header
            if (std::regex_search(headerData, match, retry)){//search the headerData for the value and pass it to match varaible
                int retryAfter = std::stoi(match[1]);//convert the string value to int value
                std::cout << "httpGet rate limit " << match[1] << "\n";//print the rate limit message to console
                std::this_thread::sleep_for(std::chrono::seconds(retryAfter));//sleep for amount of seconds
                curl_easy_cleanup(curl);//clean up all pointers 
                curl_slist_free_all(header);//clean up header pointers
                return httpGet(url, token_api);//return the method again and try again after the rate limit time
            }
        }
        //clean up
        curl_easy_cleanup(curl);
        curl_slist_free_all(header);

    }
    //return response text
   return response; 
}

void getlyrics::scrapeLyrics(const std::string& artist){
    char* raw_query = curl_easy_escape(nullptr, (artist).c_str(), 0);// conver the artist name as a encoded url so every space is = to %20
    std::string query = raw_query;//convert char* to std::string
    curl_free(raw_query);//free memory from char* to prevent memory leak
    std::string url = "https://api.genius.com/search?q=" + query;//append the url with the query
    std::string json = httpGet(url, tokenApiGenius);//get the response data with the url and token
    auto jsonData = nlohmann::json::parse(json);//pass the response to json
    //create the variables for the hashmap
    std::string songURL;
    std::string songName;
    std::string songArtist;
    try{
        for (const auto& hit : jsonData["response"]["hits"]){//search the json for each hit
            std::string artistName = hit["result"]["primary_artist"]["name"];//append artist name
            songURL = hit["result"]["url"];//append hit url
            songName = hit["result"]["title"];// append song name
            songArtist = hit["result"]["artist_names"];//append artist(s) name 
            hashMapURL[artistName].push_back({songURL,songName, songArtist});//push back values into the std::vector<type struct>
        }
    }
    //check if json search was invalid
    catch(const std::exception& e){
        std::cerr << "error occurred when passing json check valid artist name"<< e.what();

    }

    std::string songLyrics;//song lyrics string
    std::string spotifyApiToken = httpPostToken(clientScrt, clienetID);//call the get spotify token curl request
    for (const auto& [song, name, properArtistName] : hashMapURL[artist]){//for each value in vector loop through each should be all same length since they get appended at the same time

        
        std::string htmlData = httpGet(song, tokenApiGenius);//get the html data from genius from the song url and the api
        songLyrics = htmlData;
        songLyrics = std::regex_replace(songLyrics, std::regex("<[^>]+>"), " ");
        songLyrics = std::regex_replace(songLyrics, std::regex(R"(&#x27;)"), "'");
        songLyrics = std::regex_replace(songLyrics, std::regex(R"(&quot;)"), "\"");
        songLyrics = std::regex_replace(songLyrics, std::regex(R"(&amp;)"), "&");
        songLyrics = std::regex_replace(songLyrics, std::regex(R"(&lt;)"), "<");
        songLyrics = std::regex_replace(songLyrics, std::regex(R"(&gt;)"), ">");
        songLyrics = std::regex_replace(songLyrics, std::regex(R"(\\n)"), " ");
        songLyrics = std::regex_replace(songLyrics, std::regex(R"(\\)"), "");
        std::replace(songLyrics.begin(), songLyrics.end(), '\n', ' ');
        songLyrics = std::regex_replace(songLyrics, std::regex("\""), "\"\"");
        songLyrics = std::regex_replace(songLyrics, std::regex(R"(\s{2,})"), " ");
        std::vector<std::string> startTags = {"[Intro]", "[Chorus:", "[Verse:", "[Intro:", "[Verse ", "[Chorus "};
        size_t startPos = std::string::npos;
        size_t endPos = songLyrics.find("Embed Cancel");
        bool hasFoundLyrics = false;

        for (const std::string& tag : startTags) {
            startPos = songLyrics.find(tag);
            if (startPos != std::string::npos && endPos != std::string::npos && endPos > startPos) {
                songLyrics = songLyrics.substr(startPos, (endPos - startPos) - 4);
                hasFoundLyrics = true;
                break;
            }
        }

        if (!hasFoundLyrics) {
            std::cout << "Could not find a suitable starting tag or 'Embed Cancel (Skipping)'.\n";
            continue;
        } 

        std:: string songCopy = song.substr(19);//make a copy of the song url but only print the end part of it removing https://genius/
        songCopy = std::regex_replace(songCopy, std::regex(R"(-)"), " ");//remove the - symbols with a space in the leftover part of the url
        std::string artistCopy = artist;//make a copy of the artist name to artist copy
        std::transform(artistCopy.begin(),artistCopy.end(), artistCopy.begin(), [](unsigned char c){return std::tolower(c);});//go through the artist and convert it to lower case
        std::transform(artistCopy.begin(), artistCopy.begin() + 1, artistCopy.begin(), [](unsigned char c) { return std::toupper(c);});//convert the first character in the artist name to capital
        //clean up some more text from song copy
        songCopy = std::regex_replace(songCopy, std::regex(R"(-)"), " ");
        songCopy = std::regex_replace(songCopy, std::regex(artistCopy), "");
        songCopy = std::regex_replace(songCopy, std::regex("\\blyrics\\b"), "");
        std::string songLyricsQuoted = "\"" + songLyrics + "\"";//double quote the songlyrics so it can be passed properly into the csv file
        addtocsv(songLyricsQuoted, properArtistName, name, spotifyApiToken);//call the addto csv method
        songLyrics = "";//rest the song lyrics back to empty string so it can loop to the next song
    }
    std::cout << artist + " completed successfully"<< "\n";//after all the hits are scraped print finished artist name

}

void getlyrics::addtocsv(const std::string& lyrics, const std::string& artist, const std::string& title, const std::string& token_api){
    //create column names for the csv
    std::string artistName = "Artist Name";
    std::string songTitle = "Song Title";
    std::string songLyrics = "Song Lyrics";
    std::string songMood = "Song Sentiment Tag";
    std::string filePath = "../../data/Song_DataSet.csv";
    std::string mood = getMood(token_api, artist, title);//call the get mood method returns string
    if (mood == "no track id found"){// if mood couldn't find the song valence in spotify api then dont add that song to the csv
        return;
    }
    std::string quotesartist = "\"" + artist + "\"";//quote artist name
    std::string quotestitle = "\"" + title + "\"";//quote title name for csv

    std::ifstream infile(filePath);//finds file path of file
    bool isEmpty = infile.peek() == std::ifstream::traits_type::eof();//return true if the file is empty
    std::ofstream csvFile(filePath, std::ios::app);//creates or opens the file
    if (csvFile.is_open()){//return true if is open
        if (isEmpty){//if is empty is true 
            csvFile << artistName << "," << songTitle << "," << songLyrics << "," << songMood << "\n";//add the titles to the empty files
        }
        csvFile << quotesartist << "," << quotestitle << "," << lyrics << "," << mood << "\n";//append the data in the row below
        csvFile.close();//close the file
    }
    
}
//generates the token to use for spotify api calls since spotify needs a new token generated every hr this method times the creation of the first token and generates a new one if its been over an hr or 30seconds before for a buffer
std::string getlyrics::httpPostToken(const std::string& clientScrt, const std::string& clientID){
    static std::string cachedToken;//store the token here using static so it can preserve values even after calls
    static std::chrono::steady_clock::time_point tokenTime;//initialized time point type 
    constexpr int tokenTTL = 3570; // subtracted 30 sec to give buffer to prevent invalid token during api calls uses constexpr instead of const for better compile performance
    std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();//set another time point but set it to the current time

    if (!cachedToken.empty()){ // checks if token is already initialized if so then return token if it isn't expired
        if (std::chrono::duration_cast<std::chrono::seconds>(now - tokenTime).count() < tokenTTL){//if it hasn't been an hr then return the cached token
            return cachedToken;
        }
    }
    //same logic as gethttp method but it sets the post setting 
    CURL* curl = curl_easy_init();
    std::string response;
    std::string url = "https://accounts.spotify.com/api/token";
    std::string payload = "grant_type=client_credentials&client_id=" +clientID + "&client_secret=" + clientScrt;//since its post set the payload to the server
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
    cachedToken = jsonRes["access_token"];//get the access token from response
    tokenTime = now;//set the token time initialized at the start to current time so it only gets updated if it passes the if statement above
    return cachedToken;//return the token
}
// this method uses the spotify to get the track id value of the song then it copies appends that value to the title of chosic url because it uses the same track id to find the song
// then it gets the valence from the website for that song the headers are very specific copied from my web browser as the api call the website makes needs these in order for cloudfare to authenticate the request
std::string getlyrics::getMood(const std::string& token_api, const std::string& artist, const std::string& title){
    //same logic for parsing url for the scrapeLyrics method but repeat twice for both title of song and artist of song to improve accuracy for search
    char* rawQuery = curl_easy_escape(nullptr, (title).c_str(), 0);
    std::string query = rawQuery;
    curl_free(rawQuery);
    char* raw_query1 = curl_easy_escape(nullptr, (artist).c_str(), 0);
    std::string query1 = raw_query1;
    curl_free(raw_query1);
    std::string url = "https://api.spotify.com/v1/search?q=track:" + query+"%20artist:"+query1 + "&type=track&limit=1";

    std::string response = httpGet(url, token_api);//get the response from spotify
    try {
        //looping through json to find track id 
        nlohmann::json jsonResponse = nlohmann::json::parse(response);
        if (jsonResponse["tracks"]["items"].empty()) return "no track id found";
        std::cout << jsonResponse["tracks"]["items"][0]["name"] << "\n";
        std::string trackID = jsonResponse["tracks"]["items"][0]["id"];
        std::string valenceURL = "https://www.chosic.com/api/tools/audio-features/" + trackID;
        std::string valenceRes = httpGetValence(valenceURL);//once track id is found append the url of it from chosic to httpGetValence
        nlohmann::json jsonValenceRese = nlohmann::json::parse(valenceRes);//parse response to json
        float valence = jsonValenceRese["valence"];//get the valence
        //sort the score of the valence to these values which are parsed to the mood column in the csv
        if (valence > 0.8) return "Very Happy";
        else if (valence > 0.6) return "Happy";
        else if (valence > 0.4) return "Neutral";
        else if (valence > 0.2) return "Sad";
        else return "Very Sad";
              
    }
    catch (...){
        return "error with passing json";
    }
    return "";
}
//sane logic as httpGet but uses new headers that are authenticated by the websites api
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
        //another rate limit check
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


