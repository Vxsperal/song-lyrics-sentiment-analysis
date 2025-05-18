#include <fstream>  
#include <nlohmann/json.hpp>
#include <iostream>
#include <curl/curl.h>
#include "getlyrics.hpp"
using json = nlohmann::json;
int main(){
    getlyrics main;
    std::string line;
    
    // Open the text file one directory up
    std::ifstream file("../artist.txt");  // Adjust this path if needed

    // Check if the file was opened successfully
    if (!file.is_open()) {
         
        std::cerr << "Failed to open file!" << std::endl;
        return 1;  // Exit with error code
    }

    // Read each line from the file
    while (std::getline(file, line)) {
        // Each line is an artist name, pass it to scrapeLyrics
        main.scrapeLyrics(line);
    }

    // Close the file
    file.close();
    
    return 0;   
}
