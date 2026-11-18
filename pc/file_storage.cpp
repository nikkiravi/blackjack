#include "file_storage.h"
#include <cstdio>

FileStorage::FileStorage(const std::string& filePath) : path(filePath) { }

int FileStorage::loadBankroll(int fallback){
    FILE* fptr = std::fopen(path.c_str(), "r");
    if(fptr == nullptr){
        return fallback;
    }

    int currentBankroll = 0;
    int readSuccess = std::fscanf(fptr, "%d", &currentBankroll);
    if(readSuccess != 1){
        std::fclose(fptr);
        return fallback;
    }

    std::fclose(fptr);
    return currentBankroll;
}

void FileStorage::saveBankroll(int currentBankroll){
    FILE* fptr = std::fopen(path.c_str(), "w");
    if(fptr == nullptr){ return; }

    std::fprintf(fptr, "%d\n", currentBankroll);
    std::fclose(fptr);
}