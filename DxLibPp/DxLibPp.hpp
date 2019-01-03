#ifndef DXLIBPP_HPP
#define DXLIBPP_HPP

#include <string>
#include <memory>
#include <list>
#include <vector>
#include <utility>
#include <optional>
#include <functional>
#include <type_traits>
#include <algorithm>

#ifdef _MSC_VER
#    pragma comment(linker, "/subsystem:windows /ENTRY:mainCRTStartup")
#endif

namespace DxLibPp {

template<typename T> struct iterator {
    using value_type = T;
    virtual ~iterator() {}
    virtual bool has_next() const = 0;
    virtual value_type next() = 0;
    virtual void remove() = 0;
};

template<typename ErasableContainer>
struct concrete_iterator : iterator<typename ErasableContainer::value_type> {
    using value_type = typename ErasableContainer::value_type;
    concrete_iterator(const std::shared_ptr<ErasableContainer> & erasable_container)
        : begin{std::begin(*erasable_container)}
        , end{std::end(*erasable_container)}
        , erasable_container{erasable_container} {}
    virtual ~concrete_iterator() {}
    virtual bool has_next() const override { return begin != end; }

    virtual value_type next() override {
        if (!has_next())
            throw std::runtime_error("iterator has no next.");
        prev = begin++;
        return **prev;
    }

    virtual void remove() override {
        if (!prev)
            throw std::runtime_error("iterator has no previous.");
        erasable_container->erase(*prev);
    }

private:
    typename ErasableContainer::iterator begin, end;
    std::optional<typename ErasableContainer::iterator> prev;
    std::shared_ptr<ErasableContainer> erasable_container;
};

template<typename ErasableContainer>
auto make_iterator(const std::shared_ptr<ErasableContainer> & container)
    -> std::shared_ptr<iterator<typename ErasableContainer::value_type>>
{
    return std::make_shared<concrete_iterator<ErasableContainer>>(container);
}

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

template<typename T>
struct basic_position : abstract_position<T> {
    using value_type = T;
    double get_x() const override { return x; }
    double get_y() const override { return y; }
    void set_x(double x) override { this->x = x; }
    void set_y(double y) override { this->y = y; }

protected:
    value_type x{}, y{}, width{}, height{};
};
using position = basic_position<double>;

template<typename T>
struct basic_rotatable : abstract_rotatable<T> {
    using value_type = T;
    value_type get_theta() const override { return theta; }
    void set_theta(value_type theta) override { this->theta = theta; }

protected:
    value_type theta{};
};
using rotatable = basic_rotatable<double>;

struct object
    : drawable
    , updatable
    , abstract_rotatable<double>
    , abstract_position<double>
    , abstract_dimension<double>
{
    virtual void draw() const override {}
    virtual void update() override {}
};

struct graph : object {
    graph(std::string_view path);
    graph(const graph & img);
    virtual ~graph();
    graph & operator =(const graph & g);
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
    struct impl_t;
    std::unique_ptr<impl_t> impl;
};

// struct animation : drawable, updatable {
//     using frame = std::pair<graph, int>;
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
    virtual double get_theta() const override;
    virtual void set_theta(double theta) override;
    virtual void draw() const override;
    virtual void update() override;

private:
    std::string text;
    double x{}, y{}, theta{};
    struct impl_t;
    std::unique_ptr<impl_t> impl;
};

template<typename T>
struct as {
    as();
    void attach(typename std::list<T>::reverse_iterator iter);
    void detach();
    ~as();

private:
    std::optional<typename std::list<T>::iterator> optiter;
};

struct global {
    template<typename T> static std::list<std::shared_ptr<T>> & list();
    template<typename T, typename Function> static void for_each(Function && function);
    static std::vector<std::function<void()>> get_attachment_resuests();
    template<typename T> static void request_attachment(as<T> & obj);
    static void resolve_attachment();
    template<typename T, typename ... Args> static std::shared_ptr<T> create(Args && ... args);
};

template<typename T> as<T>::as() { global::request_attachment<T>(*this); }
template<typename T> void as<T>::attach(typename std::list<T>::reverse_iterator iter) { optiter = iter; }
template<typename T> void as<T>::detach() { if (optiter) global::list<T>().erase(*optiter); optiter = std::nullopt; }
template<typename T> as<T>::~as() { detach(); }

template<typename T> std::list<std::shared_ptr<T>> & global::list()
    { static std::list<std::shared_ptr<T>> lst; return lst; }

template<typename T, typename Function> void global::for_each(Function && function)
    { for (auto && element : list<T>()) function(element); }

template<typename T> void global::request_attachment(as<T> & obj)
    { get_attachment_resuests().push_back([&](){ obj.attach(list<T>().rbegin()); }); }

template<typename T, typename ... Args> std::shared_ptr<T> global::create(Args && ... args) {
    auto ptr = std::make_shared<T>(std::forward<Args>(args) ...);
    list<T>().push_back(ptr);
    resolve_attachment();
    return ptr;
}

struct system_initializer_t;
struct system {
    friend struct system_initializer_t;
    static bool update();
};

static struct system_initializer_t {
    system_initializer_t();
    ~system_initializer_t();
} system_initializer;

struct screen {
    static int get_width();
    static int get_height();
};

struct tiled_map : object, position, dimension, rotatable {
    tiled_map()
        : graph_indexes{std::make_shared<typename decltype(graph_indexes)::element_type>()}
        , graphs{std::make_shared<typename decltype(graphs)::element_type>()}
    {}

    tiled_map(std::size_t column_number, std::size_t row_number, double column_width, double row_height)
        : column_number{column_number}, row_number{row_number}, column_width{column_width}, row_height{row_height}
        , graph_indexes{std::make_shared<typename decltype(graph_indexes)::element_type>()}
        , graphs{std::make_shared<typename decltype(graphs)::element_type>()}
    {}

    virtual std::size_t get_column_number() const { return column_number; }
    virtual void set_column_number(std::size_t column_number) {this->column_number = column_number; }
    virtual std::size_t get_row_number() const { return row_number; }
    virtual void set_row_number(std::size_t row_number) { this->row_number = row_number; }
    virtual double get_column_width() const { return column_width; }
    virtual void set_column_width(double column_width) { this->column_width = column_width; }
    virtual double get_row_height() const { return this->row_height; }
    virtual void set_row_height(double row_height) { this->row_height = row_height; }
    virtual std::size_t get_graph_index(std::size_t x, std::size_t y) const { return graph_indexes->at(get_column_number() * y + x); }
    virtual void set_graph_index(std::size_t x, std::size_t y, std::size_t graph_index) { graph_indexes->at(get_column_number() * y + x) = graph_index; }
    virtual std::shared_ptr<graph> get_graph(std::size_t index) const { return graphs->at(index); }
    virtual void set_graph(std::size_t index, std::shared_ptr<graph> g) { graphs->at(index) = g; }

    template<typename Iterator> void set_graphs(Iterator begin, Iterator end) {
        graphs->clear();
        while (begin != end) {
            graphs->push_back(*begin);
            ++begin;
        }
    }

    virtual void update() override {
        for (auto graph : *graphs)
            graph->update();
    }

    virtual void draw() const override {
        for (std::size_t row = 0; row < get_row_number(); ++row) {
            for (std::size_t column = 0; column < get_column_number(); ++column) {
                std::size_t graph_index = get_graph_index(row, column);
                if (graph_index != empty) {
                    auto g = std::make_shared<graph>(*get_graph(graph_index));
                    g->set_x(get_column_width() * column);
                    g->set_y(get_row_height() * row);
                    g->set_width(get_column_width());
                    g->set_height(get_row_height());
                    g->draw();
                }
            }
        }
    }

    static constexpr std::size_t empty = static_cast<std::size_t>(-1);

private:
    double column_width{}, row_height{};
    std::size_t column_number{}, row_number{};
    std::shared_ptr<std::vector<std::size_t>> graph_indexes;
    std::shared_ptr<std::vector<std::shared_ptr<graph>>> graphs;
};

} //namespace DxLibPp

#endif
