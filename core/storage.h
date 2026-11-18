// core/storage.h: saving the bankroll between power-offs (interface, so each platform brings its own storage)
#pragma once

struct Storage {
    virtual ~Storage() = default;

    
    virtual int loadBankroll(int fallback) = 0; // Loading the bankroll when the gadget is switched on
    virtual void saveBankroll(int currentBankroll) = 0; // Save game state
};

struct FakeStorage : public Storage {
    int saved = 0; // the remembered bankroll
    bool hasSaved = false; // has anyone called save yet

    int loadBankroll(int fallback) override;
    void saveBankroll(int currentBankroll) override;
};