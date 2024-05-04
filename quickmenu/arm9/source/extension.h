#ifndef EXTENSION_H
#define EXTENSION_H
#include <initializer_list>
#include <string>

inline bool extension(std::string_view filename, const std::initializer_list<std::string_view>& extensions) {
	for(const auto& extension : extensions)
	{
		if(filename.size() >= extension.size() && strcasecmp(filename.data() + (filename.size() - extension.size()), extension.data()) == 0)
			return true;
	}
	return false;
}

#endif