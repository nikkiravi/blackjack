// pc/file_storage.h: the PC's version of Storage; keeps the bankroll in a small text file between runs
#pragma once
#include "storage.h"
#include <string>

struct FileStorage : public Storage {
    std::string path;
    FileStorage(const std::string& filePath);
    
    int loadBankroll(int fallback) override;
    void saveBankroll(int currentBankroll) override;
};