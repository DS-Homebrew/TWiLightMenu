#pragma once
#ifndef __PER_GAME_SETTINGS_H__
#define __PER_GAME_SETTINGS_H__
#include <string>
class PerGameSettings
{
  public:
    enum TLanguage
    {
        ELangDefault = -2,
        ELangSystem = -1,
        ELangJapanese = 0,
        ELangEnglish = 1,
        ELangFrench = 2,
        ELangGerman = 3,
        ELangItalian = 4,
        ELangSpanish = 5,
        ELangChineseS = 6,
        ELangKorean = 7
    };

    enum TDefaultBool
    {
        EDefault = -1,
        EFalse = 0,
        ETrue = 1,
    };

  public:
    PerGameSettings(const std::string &romFileName);
    ~PerGameSettings() {}

    void loadSettings();
    void saveSettings();
	bool checkIfShowAPMsg();
	void dontShowAPMsgAgain();
	std::string getSavExtension(int number);
	std::string getImgExtension(int number);

  public:
    TDefaultBool directBoot;
    TDefaultBool dsiMode;
    TLanguage language;
	int saveNo;
	int ramDiskNo;
    TDefaultBool boostCpu;
    TDefaultBool boostVram;
    TDefaultBool heapShrink;
    TDefaultBool bootstrapFile;
    TDefaultBool wideScreen;

  private:
    std::string _iniPath;
};

#endif