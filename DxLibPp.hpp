#ifndef DXLIBPP_HPP
#define DXLIBPP_HPP

#include <string>
#include <memory>

template<typename T>
struct abstract_position {
    using value_type = T;
    virtual ~abstract_position() {}
    virtual value_type get_x() const = 0;
    virtual value_type get_y() const = 0;
    virtual void set_x(value_type x) = 0;
    virtual void set_y(value_type y) = 0;
};

template<typename T>
struct basic_point : abstract_position<T> {
    using value_type = T;
    basic_point() {}
    basic_point(value_type x, value_type y) : x{x}, y{y} {}
    basic_point(const basic_point & p) : x{p.x}, y{p.y} {}
    virtual ~basic_point() {}
    basic_point & operator =(const basic_point & p) { x = p.x; y = p.y; }
    basic_point & operator +=(const basic_point & p) { x += p.x; y += p.y; return *this; }
    basic_point operator +(const basic_point & p) { return basic_point{*this} += p; }
    basic_point & operator -=(const basic_point & p) { x -= p.x; y -= p.y; return *this; }
    basic_point operator -(const basic_point & p) { return basic_point{*this} -= p; }
    template<typename U> operator basic_point<U>() const { return basic_point<U>{static_cast<U>(x), static_cast<U>(y)}; }
    virtual value_type get_x() const override { return x; }
    virtual value_type get_y() const override { return y; }
    virtual void set_x(value_type x) override { this->x = x; }
    virtual void set_y(value_type y) override { this->y = y; }
    value_type x{}, y{};
};
using point = basic_point<double>;

template<typename T>
struct abstract_dimension {
    using value_type = T;
    ~abstract_dimension() {}
    virtual value_type get_width() const = 0;
    virtual value_type get_height() const = 0;
    virtual void set_width(value_type width) = 0;
    virtual void set_height(value_type height) = 0;
};

template<typename T>
struct basic_dimension : abstract_dimension<T> {
    using value_type = T;
    basic_dimension() {}
    basic_dimension(value_type width, value_type height) : width{width}, height{height} {}
    basic_dimension(const basic_dimension & obj) = default;
    virtual ~basic_dimension() {}
    basic_dimension & operator=(const basic_dimension & obj) = default;
    virtual value_type get_width() const override { return width; }
    virtual value_type get_height() const override { return height; }
    virtual void set_width(value_type width) override { this->width = width; }
    virtual void set_height(value_type height) override { this->height = height; }

private:
    value_type width{}, height{};
};
using dimension = basic_dimension<double>;

template<typename T>
struct basic_rect : abstract_position<T>, abstract_dimension<T> {
    using value_type = T;
    basic_rect() {}
    basic_rect(value_type x, value_type y, value_type width, value_type height) : x{x}, y{y}, width{width}, height{height} {}
    basic_rect(const basic_rect &) = default;
    basic_rect & operator =(const basic_rect & r) = default;
    bool intersects(const basic_rect & r) const { return x <= r.x + r.width && y <= r.y + r.height && r.x <= x + width && r.y <= y + height; }
    basic_point<value_type> center() const { return basic_point<value_type>{x + width / 2, y + height / 2}; }
    virtual value_type get_x() const override { return x; }
    virtual value_type get_y() const override { return y; }
    virtual value_type get_width() const override { return width; }
    virtual value_type get_height() const override { return height; }

private:
    value_type x{}, y{}, width{}, height{};
};
using rect = basic_rect<double>;

struct drawable {
    virtual ~drawable() {}
    virtual void draw() const = 0;
};

struct updatable {
    virtual ~updatable() {}
    virtual void update() = 0;
};

template<typename T>
struct abstract_rotatable {
    using value_type = T;
    virtual ~abstract_rotatable() {}
    virtual value_type get_theta() const = 0;
    virtual void set_theta(value_type theta) = 0;
};

struct object : drawable, updatable, abstract_rotatable<double>, abstract_position<double>, abstract_dimension<double> {};

struct image : object {
    image(std::string_view path);
    image(const image & img);
    virtual ~image();
    image & operator =(const image & img);
    virtual void draw() const override;
    virtual double get_x() const override { return x; }
    virtual double get_y() const override { return y; }
    virtual void set_x(double x) override { this->x = x; }
    virtual void set_y(double y) override { this->y = y; }
    virtual double get_width() const override { return width; }
    virtual double get_height() const override { return height; }
    virtual void set_width(double width) override { this->width = width; }
    virtual void set_height(double heihgt) override { this->height = height; }
    virtual double get_theta() const override { return theta; }
    virtual void set_theta(double theta) override { this->theta = theta; }

private:
    double x{}, y{}, width{}, height{}, theta{};
    class impl_t;
    std::unique_ptr<impl_t> impl;
};

// struct animation : drawable, updatable {
//     using frame = std::pair<image, int>;
//     template<typename Iterator> animation(Iterator begin, Iterator end) : frames{begin, end}
//         { if (frames.empty()) throw std::logic_error("animation must be not empty."); }
//     virtual draw() const override { frames[index].first.draw(x, y, width, height); }
//     virtual double get_width() const override { return frames.front().first.get_width(); }
//     virtual double get_height() const override { return frames.front().first.get_height(); }
//     virtual update() override { if (++count > frames[index].second) ++index %= frames.size(); }
//
// private:
//     std::vector<frame> frames;
//     std::size_t count{};
//     std::size_t index{};
// };

struct font : object {
    font();
    font(std::string_view path, int size = -1);
    font(const font & fnt);
    virtual ~font();
    font & operator =(const font & fnt);
    virtual std::string_view get_text() const { return text; }
    virtual void set_text(std::string_view text) { this->text = text; }
    virtual double get_x() const override { return x; }
    virtual double get_y() const override { return y; }
    virtual void set_x(double x) override { this->x = x; }
    virtual void set_y(double y) override { this->y = y; }
    virtual double get_width() const override;
    virtual double get_height() const override;
    virtual void set_width(double width) {}
    virtual void set_height(double height) {}
    virtual void draw() const;

private:
    std::string text;
    double x{}, y{};
    class impl_t;
    std::unique_ptr<impl_t> impl;
};

struct system_initializer_t;
struct system {
    friend class system_initializer_t;
    static bool update();
};

static struct system_initializer_t {
    system_initializer_t();
    ~system_initializer_t();
} system_initializer;

#endif
