#pragma once

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace image {

template <typename T = uint32_t> class basic_bitmap
{
    class const_split_iterator
    {
    public:
        const_split_iterator(const basic_bitmap& bm, int32_t w, int32_t h,
                             int32_t xpos = 0, int32_t ypos = 0)
            : bm(bm), width(w), height(h), xpos(xpos), ypos(ypos)
        {}
        const_split_iterator(const const_split_iterator& rhs)
            : bm(rhs.bm), width(rhs.width), height(rhs.height), xpos(rhs.xpos),
              ypos(rhs.ypos)
        {}

        bool operator!=(const const_split_iterator& other) const
        {
            return xpos != other.xpos || ypos != other.ypos;
        }

        basic_bitmap operator*() const
        {
            return bm.cut(xpos, ypos, width, height);
        }

        const const_split_iterator& operator++()
        {
            xpos += width;
            if (xpos > bm.width() - width) {
                xpos = 0;
                ypos += height;
                if (ypos > bm.height() - height) {
                    xpos = ypos = -1;
                }
            }
            return *this;
        }

    private:
        const basic_bitmap& bm;
        int32_t width;
        int32_t height;
        int32_t xpos;
        int32_t ypos;
    };

    class splitter
    {
    public:
        splitter(const basic_bitmap& bm, int32_t w, int32_t h)
            : bm(bm), width(w), height(h)
        {}
        const_split_iterator begin()
        {
            return const_split_iterator(bm, width, height);
        }

        const_split_iterator end()
        {
            return const_split_iterator(bm, width, height, -1, -1);
        }

    private:
        const basic_bitmap& bm;
        int32_t width;
        int32_t height;
    };

public:
    splitter split(int32_t w, int32_t h) const
    {
        return splitter(*this, w, h);
    }

    basic_bitmap() : w(0), h(0)
    {}

    basic_bitmap(int32_t width, int32_t height,
                 std::function<T(int32_t, int32_t)> const& f)
    {
        w = width;
        h = height;
        pixels = std::make_shared<std::vector<T>>(width * height);
        for (int32_t yy = 0; yy < h; yy++)
            for (int32_t xx = 0; xx < w; xx++)
                (*pixels)[xx + w * yy] = f(xx, yy);
    }

    basic_bitmap(int32_t width, int32_t height) : w(width), h(height)
    {
        pixels = std::make_shared<std::vector<T>>(width * height);
    }

    basic_bitmap(int32_t width, int32_t height, const std::vector<T>& data)
        : w(width), h(height)
    {
        pixels = std::make_shared<std::vector<T>>(width * height);
        memcpy(&(*pixels)[0], &data[0], sizeof(T) * width * height);
    }

    basic_bitmap(int32_t width, int32_t height, const T& color)
        : w(width), h(height)
    {
        pixels = std::make_shared<std::vector<T>>(width * height);
        std::fill(pixels->begin(), pixels->end(), color);
    }

    basic_bitmap(int32_t width, int32_t height, const T* px)
        : w(width), h(height)
    {
        pixels = std::make_shared<std::vector<T>>(width * height);
        memcpy(&(*pixels)[0], px, sizeof(T) * width * height);
    }

    basic_bitmap(int32_t width, int32_t height, int32_t n, const uint8_t* px)
        : w(width), h(height)
    {
        pixels = std::make_shared<std::vector<T>>(width * height);
        for (int64_t i = 0; i < w * h; i++)
            (*pixels)[i] = 0xff000000 | px[i * 3] | (px[i * 3 + 1] << 8) |
                           (px[i * 3 + 2] << 16);
    }

    //! Render another bitmap into this one
    void put(int32_t x, int32_t y, const basic_bitmap& bm)
    {
        T* target = bm.data();
        for (int32_t yy = y; yy < bm.h + y; yy++)
            for (int32_t xx = x; xx < bm.w + x; xx++) {
                (*pixels)[yy * w + x] = *target++;
            }
    }

    basic_bitmap clone()
    {
        return basic_bitmap(w, h, &(*pixels)[0]);
    }

    T& operator[](const int64_t& i)
    {
        return (*pixels)[i];
    }

    T operator[](const int64_t& i) const
    {
        return (*pixels)[i];
    }

    typename std::vector<T>::iterator begin()
    {
        return pixels->begin();
    };

    typename std::vector<T>::iterator end()
    {
        return pixels->end();
    };

    //! Clear bitmap to given color
    void clear(uint32_t color = 0)
    {
        T* p = &(*pixels)[0];
        if (sizeof(T) == 1 || color == 0)
            memset(p, 0, w * h * sizeof(T));
        else {
            for (int32_t i = 0; i < w * h; i++)
                p[i] = color;
        }
    }

    //! Return a cut out of the bitmap
    basic_bitmap cut(int32_t x, int32_t y, int32_t ww, int32_t hh) const
    {
        basic_bitmap dest(ww, hh);
        T* p = &(*pixels)[0];
        for (int32_t yy = 0; yy < hh; yy++)
            for (int32_t xx = 0; xx < ww; xx++) {
                dest[xx + yy * ww] = p[xx + x + (yy + y) * this->w];
            }
        return dest;
    }

    T* data()
    {
        return &(*pixels)[0];
    }

    basic_bitmap flip()
    {
        basic_bitmap result(w, h);
        auto* target = result.data();
        int32_t l = sizeof(T) * w;
        for (int32_t y = 0; y < h; y++)
            std::memcpy(&(*pixels)[(h - y - 1) * w], &(*target)[y * w], l);
        return result;
    }

    int32_t width() const
    {
        return w;
    }
    int32_t height() const
    {
        return h;
    }
    int64_t size() const
    {
        return w * h;
    }

private:
    std::shared_ptr<std::vector<T>> pixels;
    int32_t w;
    int32_t h;
};

typedef basic_bitmap<uint32_t> bitmap;
typedef basic_bitmap<uint8_t> bitmap8;

} // namespace image

