#include <string>

static std::string normalize_filename(std::string name) {
    // "///my/images/shime1.png" ==> "my_images_shime1.png"
    size_t i=0;
    while (i < name.size() && name[i] == '/')
        ++i;
    name = name.substr(i);
    i = 0;
    while ((i = name.find('/', i)) != std::string::npos)
        name[i++] = '_';
    return to_lower(name);
}
