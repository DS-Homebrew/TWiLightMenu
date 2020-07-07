#include "widescreenconfig.h"
#include "dsimenusettings.h"
#include "filecopy.h"
#include "systemdetails.h"

#include "sr_data_srllastran.h"

WidescreenConfig &WidescreenConfig::enable(bool enable)
{
  _useWidescreen = enable;
  return *this;
}

WidescreenConfig &WidescreenConfig::isHomebrew(bool homebrew)
{
  _isHomebrew = homebrew;
  return *this;
}

WidescreenConfig::WidescreenConfig(const std::string &filename)
    : _isHomebrew(false), _useWidescreen(false)
{
  std::string patchPath =
      "sd:/_nds/TWiLightMenu/widescreen/" + filename + ".bin";
  if ((access(patchPath.c_str(), F_OK) == 0))
  {
    _widescreenPatch = patchPath;
  }
  else
  {
    _widescreenPatch = std::string();
  }
}

WidescreenConfig &WidescreenConfig::gamePatch(const char *gametid,
                                              u16 gameCrc)
{
  if (!_widescreenPatch.empty())
    return *this;
  char wideBinPath[256];
  snprintf(wideBinPath, sizeof(wideBinPath),
           "sd:/_nds/TWiLightMenu/widescreen/%s-%X.bin", gametid, gameCrc);
  if (access(wideBinPath, F_OK) == 0)
  {
    _widescreenPatch = std::string(wideBinPath);
  }
  return *this;
}

const std::string WidescreenConfig::apply()
{
  remove("/_nds/nds-bootstrap/wideCheatData.bin");

  if (sys().arm7SCFGLocked() || ms().consoleModel < 2 || !_useWidescreen ||
      (access("sd:/_nds/TWiLightMenu/TwlBg/Widescreen.cxi", F_OK) != 0))
  {
    return "";
  }

  if (_isHomebrew)
  {
    if (!ms().homebrewHasWide)
      return "";
    // Prepare for reboot into 16:10 TWL_FIRM
    mkdir("sd:/luma", 0777);
    mkdir("sd:/luma/sysmodules", 0777);
    if ((access("sd:/luma/sysmodules/TwlBg.cxi", F_OK) == 0) &&
        (rename("sd:/luma/sysmodules/TwlBg.cxi",
                "sd:/luma/sysmodules/TwlBg_bak.cxi") != 0))
    {
      return "Failed to backup custom TwlBg.";
    }
    if (fcopy("sd:/_nds/TWiLightMenu/TwlBg/Widescreen.cxi",
              "sd:/luma/sysmodules/TwlBg.cxi") == 0)
    {
      irqDisable(IRQ_VBLANK); // Fix the throwback to 3DS HOME Menu bug
      memcpy((u32 *)0x02000300, sr_data_srllastran, 0x020);
      fifoSendValue32(FIFO_USER_02, 1); // Reboot in 16:10 widescreen
      swiWaitForVBlank();
    }
    else
    {
      return "Failed to reboot TwlBg in widescreen.";
    }
    return "";
  }

  if (!_widescreenPatch.empty())
  {
    std::string resultText;
    mkdir("/_nds", 0777);
    mkdir("/_nds/nds-bootstrap", 0777);

    if (fcopy(_widescreenPatch.c_str(), "/_nds/nds-bootstrap/wideCheatData.bin") == 0)
    {
      // Ensure luma directories
      mkdir("sd:/luma", 0777);
      mkdir("sd:/luma/sysmodules", 0777);

      // Backup existing TwlBg.cxi
      if ((access("sd:/luma/sysmodules/TwlBg.cxi", F_OK) == 0) && (rename("sd:/luma/sysmodules/TwlBg.cxi", "sd:/luma/sysmodules/TwlBg_bak.cxi") != 0))
      {
        return "Failed to backup custom TwlBg.";
      }

      if (fcopy("sd:/_nds/TWiLightMenu/TwlBg/Widescreen.cxi", "sd:/luma/sysmodules/TwlBg.cxi") == 0)
      {
        irqDisable(IRQ_VBLANK); // Fix the throwback to 3DS HOME Menu bug
        memcpy((u32 *)0x02000300, sr_data_srllastran, 0x020);
        fifoSendValue32(FIFO_USER_02, 1); // Reboot in 16:10 widescreen
        swiWaitForVBlank();
        return "";
      }
      else
      {
        rename("sd:/luma/sysmodules/TwlBg_bak.cxi", "sd:/luma/sysmodules/TwlBg.cxi");
        return "Failed to copy Widescreen TwlBg.";
      }
    }
    else
    {
      return "Failed to copy widescreen patch.";
    }
  }
  return "";
}

WidescreenConfig::WidescreenConfig()
    : _widescreenPatch(std::string()), _isHomebrew(false),
      _useWidescreen(false) {}