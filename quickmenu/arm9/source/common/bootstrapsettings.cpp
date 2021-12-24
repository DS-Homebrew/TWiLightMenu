
#include "myDSiMode.h"
#include "common/inifile.h"
#include "common/bootstrappaths.h"
#include "bootstrapsettings.h"
#include <string.h>

BootstrapSettings::BootstrapSettings()
{
    b4dsMode = 0;
}

void BootstrapSettings::loadSettings()
{
    CIniFile bootstrapini(BOOTSTRAP_INI);

	if (dsiFeatures()) {
		b4dsMode = bootstrapini.GetInt("NDS-BOOTSTRAP", "B4DS_MODE", b4dsMode);
	}
}
