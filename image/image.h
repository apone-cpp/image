#pragma once

#include "bitmap.h"

#include <stdexcept>
#include <vector>

namespace image {

class image_exception : public std::exception
{
public:
    image_exception(const std::string& msg) : msg(msg) {}
    virtual const char* what() const throw() { return msg.c_str(); }

private:
    std::string msg;
};

std::vector<bitmap> load_gifs(const std::string& filename);
bitmap load_image(const std::string& file_name);
void save_png(bitmap bitmap, const std::string& path);

} // namespace image

namespace utils {
inline bool load_data(image::bitmap& target, std::string const& file_name)
{
    try {
        target = image::load_image(file_name);
    } catch (image::image_exception& ie) {
        return false;
    }
    return true;
}

inline void save_data(image::bitmap const& source, std::string const& file_name)
{
    image::save_png(source, file_name);
}

} // namespace utils
