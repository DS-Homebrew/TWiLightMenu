#pragma once

#include <string>
#include <string_view>
#include <strings.h>

namespace twlm {

// Select the folder just left after the parent's normal listing/sort finishes.
// Keep one reusable filename, not paths/history or extra filesystem requests.
class ParentDirectorySelection {
    std::string folder;

public:
    void remember(std::string_view currentPath) {
        while (!currentPath.empty() && currentPath.back() == '/')
            currentPath.remove_suffix(1);
        const size_t slash = currentPath.find_last_of('/');
        if (slash == std::string_view::npos) {
            folder.clear(); // A volume root (sd:/ or fat:/) has no parent tile.
        } else {
            folder.assign(currentPath.data() + slash + 1, currentPath.size() - slash - 1);
        }
    }

    template <typename Entries>
    int restore(const Entries &entries) {
        if (folder.empty()) return -1;
        int selected = -1;
        for (size_t i = 0; i < entries.size(); ++i) {
            if (entries[i].isDirectory && strcasecmp(entries[i].name.c_str(), folder.c_str()) == 0) {
                selected = static_cast<int>(i);
                break;
            }
        }
        folder.clear(); // Consume once; later reloads keep their current cursor.
        return selected;
    }
};

} // namespace twlm
