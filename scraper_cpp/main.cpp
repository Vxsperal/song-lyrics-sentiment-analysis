#include <nlohmann/json.hpp>
#include <iostream>
#include <curl/curl.h>
#include "getlyrics.hpp"
using json = nlohmann::json;
int main(){
    std::string token = ""; //remeber to remove when uploading to github
    getlyrics main(token);
    main.scrapeLyrics("Juice WRLD", "Lucid Dreams");
    return 0;
}
