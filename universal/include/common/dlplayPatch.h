#ifndef DLPLAY_PATCH_H
#define DLPLAY_PATCH_H

// RSA signature check patch for DSi DS Download Play (HNDA v256), ported from FlashMe
// https://gbatemp.net/threads/rsa-patch-for-dsi-download-play.538078/

// Returns the path to boot: a patched copy when the patch applies, otherwise srcPath
const char* dlplayGetBootPath(const char* srcPath, const char* patchedPath);

#endif // DLPLAY_PATCH_H
