#include "storage.h"

int FakeStorage::loadBankroll(int fallback){
    return hasSaved ? saved : fallback;
}

void FakeStorage::saveBankroll(int currentBankroll){
    saved = currentBankroll;
    hasSaved = true;
}