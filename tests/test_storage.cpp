// tests/test_storage.cpp: tests for saving/loading the bankroll (FakeStorage and the PC's FileStorage)
#include "check.h"
#include "storage.h"
#include "file_storage.h"
#include <cstdio>

// every Storage must behave the same, whichever kind it is (called through the interface)
static void checkSaveThenLoad(Storage& storage) {
    storage.saveBankroll(340);
    CHECK(storage.loadBankroll(200) == 340);
    storage.saveBankroll(2);                       // later saves replace earlier ones
    CHECK(storage.loadBankroll(200) == 2);
    storage.saveBankroll(0);                       // 0 is a real saved value, not "nothing saved"
    CHECK(storage.loadBankroll(200) == 0);
}

void test_fake_storage() {
    {   // nothing saved yet -> the fallback
        FakeStorage s;
        CHECK(s.loadBankroll(200) == 200);
        CHECK(s.loadBankroll(50) == 50);
    }
    {   // save then load, through the Storage interface
        FakeStorage s;
        checkSaveThenLoad(s);
    }
}

// writes `text` into the file at `path`, replacing anything there (test helper)
static void writeFile(const char* path, const char* text) {
    FILE* f = std::fopen(path, "w");
    if (f != nullptr) {
        std::fputs(text, f);
        std::fclose(f);
    }
}

void test_file_storage() {
    // the test build runs from the project root, and make creates build/ before running tests
    const char* path = "build/test_bankroll.txt";
    std::remove(path);                             // start clean, in case an earlier run left it

    {   // no file yet (first run) -> the fallback
        FileStorage s(path);
        CHECK(s.loadBankroll(200) == 200);
    }
    {   // save then load, through the Storage interface
        FileStorage s(path);
        checkSaveThenLoad(s);
    }
    {   // the number survives a "restart": a brand-new FileStorage reads the same file
        FileStorage first(path);
        first.saveBankroll(512);
        FileStorage second(path);
        CHECK(second.loadBankroll(200) == 512);
    }
    {   // empty file -> the fallback (fscanf returns EOF)
        writeFile(path, "");
        FileStorage s(path);
        CHECK(s.loadBankroll(200) == 200);
    }
    {   // garbage in the file -> the fallback (fscanf reads 0 numbers)
        writeFile(path, "hello\n");
        FileStorage s(path);
        CHECK(s.loadBankroll(200) == 200);
    }
    {   // saving somewhere that can't be written doesn't crash
        FileStorage s("build/no_such_folder/bankroll.txt");
        s.saveBankroll(5);
        CHECK(s.loadBankroll(200) == 200);
    }

    std::remove(path);                             // don't leave test files behind
}
