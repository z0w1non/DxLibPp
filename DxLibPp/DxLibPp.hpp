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

template<typename T> struct Iterator {
    using value_type = T;
    virtual ~Iterator() {}
    virtual bool HasNext() const = 0;
    virtual value_type Next() = 0;
    virtual void Remove() = 0;
};

template<typename ErasableContainer>
struct ConcreteIterator : Iterator<typename ErasableContainer::value_type> {
    using value_type = typename ErasableContainer::value_type;
    ConcreteIterator(const std::shared_ptr<ErasableContainer> & erasable_container)
        : begin{std::begin(*erasable_container)}
        , end{std::end(*erasable_container)}
        , erasable_container{erasable_container} {}
    virtual ~ConcreteIterator() {}
    virtual bool HasNext() const override { return begin != end; }

    virtual value_type Next() override {
        if (!HasNext())
            throw std::runtime_error("Iterator has no next.");
        prev = begin++;
        return **prev;
    }

    virtual void Remove() override {
        if (!prev)
            throw std::runtime_error("Iterator has no previous.");
        erasable_container->erase(*prev);
    }

private:
    typename ErasableContainer::iterator begin, end;
    std::optional<typename ErasableContainer::iterator> prev;
    std::shared_ptr<ErasableContainer> erasable_container;
};

template<typename ErasableContainer>
auto CreateIterator(const std::shared_ptr<ErasableContainer> & container)
    -> std::shared_ptr<Iterator<typename ErasableContainer::value_type>>
{
    return std::make_shared<ConcreteIterator<ErasableContainer>>(container);
}

template<typename T>
struct AbstractPosition {
    using value_type = T;
    virtual ~AbstractPosition() {}
    virtual value_type GetX() const = 0;
    virtual value_type GetY() const = 0;
    virtual void SetX(value_type x) = 0;
    virtual void SetY(value_type y) = 0;
};

template<typename T>
struct BasicPoint : AbstractPosition<T> {
    using value_type = T;
    BasicPoint() {}
    BasicPoint(value_type x, value_type y) : x{x}, y{y} {}
    BasicPoint(const BasicPoint & p) : x{p.x}, y{p.y} {}
    virtual ~BasicPoint() {}
    BasicPoint & operator =(const BasicPoint & p) { x = p.x; y = p.y; }
    BasicPoint & operator +=(const BasicPoint & p) { x += p.x; y += p.y; return *this; }
    BasicPoint operator +(const BasicPoint & p) { return BasicPoint{*this} += p; }
    BasicPoint & operator -=(const BasicPoint & p) { x -= p.x; y -= p.y; return *this; }
    BasicPoint operator -(const BasicPoint & p) { return BasicPoint{*this} -= p; }
    template<typename U> operator BasicPoint<U>() const { return BasicPoint<U>{static_cast<U>(x), static_cast<U>(y)}; }
    virtual value_type GetX() const override { return x; }
    virtual value_type GetY() const override { return y; }
    virtual void SetX(value_type x) override { this->x = x; }
    virtual void SetY(value_type y) override { this->y = y; }
    value_type x{}, y{};
};
using Point = BasicPoint<double>;

template<typename T>
struct AbstractDimension {
    using value_type = T;
    ~AbstractDimension() {}
    virtual value_type GetWidth() const = 0;
    virtual value_type GetHeight() const = 0;
    virtual void SetWidth(value_type width) = 0;
    virtual void SetHeight(value_type height) = 0;
};

template<typename T>
struct BasicDimension : AbstractDimension<T> {
    using value_type = T;
    BasicDimension() {}
    BasicDimension(value_type width, value_type height) : width{width}, height{height} {}
    BasicDimension(const BasicDimension & obj) = default;
    virtual ~BasicDimension() {}
    BasicDimension & operator=(const BasicDimension & obj) = default;
    virtual value_type GetWidth() const override { return width; }
    virtual value_type GetHeight() const override { return height; }
    virtual void SetWidth(value_type width) override { this->width = width; }
    virtual void SetHeight(value_type height) override { this->height = height; }

private:
    value_type width{}, height{};
};
using Dimension = BasicDimension<double>;

template<typename T>
struct BasicRect : AbstractPosition<T>, AbstractDimension<T> {
    using value_type = T;
    BasicRect() {}
    BasicRect(value_type x, value_type y, value_type width, value_type height) : x{x}, y{y}, width{width}, height{height} {}
    BasicRect(const BasicRect &) = default;
    BasicRect & operator =(const BasicRect & r) = default;
    bool intersects(const BasicRect & r) const { return x <= r.x + r.width && y <= r.y + r.height && r.x <= x + width && r.y <= y + height; }
    BasicPoint<value_type> center() const { return BasicPoint<value_type>{x + width / 2, y + height / 2}; }
    virtual value_type GetX() const override { return x; }
    virtual value_type GetY() const override { return y; }
    virtual value_type GetWidth() const override { return width; }
    virtual value_type GetHeight() const override { return height; }

private:
    value_type x{}, y{}, width{}, height{};
};
using Rect = BasicRect<double>;

struct Drawable {
    virtual ~Drawable() {}
    virtual void Draw() const = 0;
};

struct Updatable {
    virtual ~Updatable() {}
    virtual void Update() = 0;
};

template<typename T>
struct AbstractRotatable {
    using value_type = T;
    virtual ~AbstractRotatable() {}
    virtual value_type GetTheta() const = 0;
    virtual void SetTheta(value_type theta) = 0;
};

template<typename T>
struct BasicPosition : AbstractPosition<T> {
    using value_type = T;
    double GetX() const override { return x; }
    double GetY() const override { return y; }
    void SetX(double x) override { this->x = x; }
    void SetY(double y) override { this->y = y; }

protected:
    value_type x{}, y{}, width{}, height{};
};
using Position = BasicPosition<double>;

template<typename T>
struct BasicRotatable : AbstractRotatable<T> {
    using value_type = T;
    value_type GetTheta() const override { return theta; }
    void SetTheta(value_type theta) override { this->theta = theta; }

protected:
    value_type theta{};
};
using Rotatable = BasicRotatable<double>;

struct Object
    : Drawable
    , Updatable
    , AbstractRotatable<double>
    , AbstractPosition<double>
    , AbstractDimension<double>
{
    virtual void Draw() const override {}
    virtual void Update() override {}
};

struct Graph : Object {
    Graph();
    Graph(std::string_view path);
    Graph(const Graph & img);
    virtual ~Graph();
    Graph & operator =(const Graph & g);
    virtual void Draw() const override;
    virtual double GetX() const override { return x; }
    virtual double GetY() const override { return y; }
    virtual void SetX(double x) override { this->x = x; }
    virtual void SetY(double y) override { this->y = y; }
    virtual double GetWidth() const override { return width; }
    virtual double GetHeight() const override { return height; }
    virtual void SetWidth(double width) override { this->width = width; }
    virtual void SetHeight(double heihgt) override { this->height = height; }
    virtual double GetTheta() const override { return theta; }
    virtual void SetTheta(double theta) override { this->theta = theta; }
    virtual void Load(std::string_view path);
    static std::shared_ptr<Iterator<std::shared_ptr<Graph>>> LoadDivGraph(
        std::string_view path,
        std::size_t number,
        std::size_t column_number, std::size_t row_number,
        std::size_t column_width, std::size_t row_height
    );

private:
    double x{}, y{}, width{}, height{}, theta{};
    struct impl_t;
    std::unique_ptr<impl_t> impl;
};

struct Animation : Object {
    using frame = std::pair<std::shared_ptr<Graph>, std::size_t>;
    Animation(const std::shared_ptr<Iterator<std::shared_ptr<frame>>> & frame_iterator) {
        while (frame_iterator->HasNext())
            frames->push_back(frame_iterator->Next());
        if (frames->empty()) throw std::logic_error("Animation must be not empty.");
    }
    std::shared_ptr<Graph> GetCurrentGraph() const { return frames->at(index)->first; }
    virtual void Draw() const override { GetCurrentGraph()->Draw(); }
    virtual double GetWidth() const override { return frames->front()->first->GetWidth(); }
    virtual double GetHeight() const override { return frames->front()->first->GetHeight(); }
    virtual void Update() override { if (++count > frames->at(index)->second) { count = 0; ++index %= frames->size(); } }
private:
    std::shared_ptr<std::vector<std::shared_ptr<frame>>> frames{
        std::make_shared<std::vector<std::shared_ptr<frame>>>()
    };
    std::size_t count{};
    std::size_t index{};
};

struct Font : Object {
    Font();
    Font(std::string_view path, int size = -1);
    Font(const Font & fnt);
    virtual ~Font();
    Font & operator =(const Font & fnt);
    virtual std::string_view GetText() const { return text; }
    virtual void SetText(std::string_view text) { this->text = text; }
    virtual double GetX() const override { return x; }
    virtual double GetY() const override { return y; }
    virtual void SetX(double x) override { this->x = x; }
    virtual void SetY(double y) override { this->y = y; }
    virtual double GetWidth() const override;
    virtual double GetHeight() const override;
    virtual void SetWidth(double width) {}
    virtual void SetHeight(double height) {}
    virtual double GetTheta() const override;
    virtual void SetTheta(double theta) override;
    virtual void Load(std::string_view path, int size = -1);
    virtual void Draw() const override;
    virtual void Update() override;

private:
    std::string text;
    double x{}, y{}, theta{};
    struct impl_t;
    std::unique_ptr<impl_t> impl;
};

template<typename T>
struct As {
    As();
    void Attach(typename std::list<T>::reverse_iterator iter);
    void Detach();
    ~As();

private:
    std::optional<typename std::list<T>::Iterator> optiter;
};

struct Global {
    template<typename T> static std::list<std::shared_ptr<T>> & list();
    static std::vector<std::function<void()>> GetAttachmentResuests();
    template<typename T> static void RequestAttachment(As<T> & obj);
    static void ResolveAttachment();
    template<typename T, typename ... Args> static std::shared_ptr<T> Create(Args && ... args);
};

template<typename T> As<T>::As() { Global::RequestAttachment<T>(*this); }
template<typename T> void As<T>::Attach(typename std::list<T>::reverse_iterator iter) { optiter = iter; }
template<typename T> void As<T>::Detach() { if (optiter) Global::list<T>().erase(*optiter); optiter = std::nullopt; }
template<typename T> As<T>::~As() { Detach(); }

template<typename T> std::list<std::shared_ptr<T>> & Global::list()
    { static std::list<std::shared_ptr<T>> lst; return lst; }

template<typename T> void Global::RequestAttachment(As<T> & obj)
    { GetAttachmentResuests().push_back([&](){ obj.Attach(list<T>().rbegin()); }); }

template<typename T, typename ... Args> std::shared_ptr<T> Global::Create(Args && ... args) {
    auto ptr = std::make_shared<T>(std::forward<Args>(args) ...);
    list<T>().push_back(ptr);
    ResolveAttachment();
    return ptr;
}

struct SystemInitializer;
struct System {
    friend struct SystemInitializer;
    static bool Update();
};

static struct SystemInitializer {
    SystemInitializer();
    ~SystemInitializer();
} system_initializer;

struct Screen {
    static int GetWidth();
    static int GetHeight();
};

struct Sound {
    Sound();
    Sound(std::string_view path);
    Sound(const Sound & obj);
    virtual ~Sound();
    Sound & operator =(const Sound & obj);
    virtual void Play(int play_type, bool top_position_flag = true);
    virtual void Stop();
    virtual bool Check() const;
    virtual void Load(std::string_view path);

    enum {
        NORMAL,
        BACK,
        LOOP
    };

private:
    struct impl_t;
    std::unique_ptr<impl_t> impl;
};

struct Key {
    friend struct System;
    static bool CheckHit(int key_code);
    static int GetTimer(int key_code);

    static const int INPUT_BACK;
    static const int INPUT_TAB;
    static const int INPUT_RETURN;

    static const int INPUT_LSHIFT;
    static const int INPUT_RSHIFT;
    static const int INPUT_LCONTROL;
    static const int INPUT_RCONTROL;
    static const int INPUT_ESCAPE;
    static const int INPUT_SPACE;
    static const int INPUT_PGUP;
    static const int INPUT_PGDN;
    static const int INPUT_END;
    static const int INPUT_HOME;
    static const int INPUT_LEFT;
    static const int INPUT_UP;
    static const int INPUT_RIGHT;
    static const int INPUT_DOWN;
    static const int INPUT_INSERT;
    static const int INPUT_DELETE;

    static const int INPUT_MINUS;
    static const int INPUT_YEN;
    static const int INPUT_PREVTRACK;
    static const int INPUT_PERIOD;
    static const int INPUT_SLASH;
    static const int INPUT_LALT;
    static const int INPUT_RALT;
    static const int INPUT_SCROLL;
    static const int INPUT_SEMICOLON;
    static const int INPUT_COLON;
    static const int INPUT_LBRACKET;
    static const int INPUT_RBRACKET;
    static const int INPUT_AT;
    static const int INPUT_BACKSLASH;
    static const int INPUT_COMMA;
    static const int INPUT_CAPSLOCK;
    static const int INPUT_PAUSE;

    static const int INPUT_NUMPAD0;
    static const int INPUT_NUMPAD1;
    static const int INPUT_NUMPAD2;
    static const int INPUT_NUMPAD3;
    static const int INPUT_NUMPAD4;
    static const int INPUT_NUMPAD5;
    static const int INPUT_NUMPAD6;
    static const int INPUT_NUMPAD7;
    static const int INPUT_NUMPAD8;
    static const int INPUT_NUMPAD9;
    static const int INPUT_MULTIPLY;
    static const int INPUT_ADD;
    static const int INPUT_SUBTRACT;
    static const int INPUT_DECIMAL;
    static const int INPUT_DIVIDE;
    static const int INPUT_NUMPADENTER;

    static const int INPUT_F1;
    static const int INPUT_F2;
    static const int INPUT_F3;
    static const int INPUT_F4;
    static const int INPUT_F5;
    static const int INPUT_F6;
    static const int INPUT_F7;
    static const int INPUT_F8;
    static const int INPUT_F9;
    static const int INPUT_F10;
    static const int INPUT_F11;
    static const int INPUT_F12;

    static const int INPUT_A;
    static const int INPUT_B;
    static const int INPUT_C;
    static const int INPUT_D;
    static const int INPUT_E;
    static const int INPUT_F;
    static const int INPUT_G;
    static const int INPUT_H;
    static const int INPUT_I;
    static const int INPUT_J;
    static const int INPUT_K;
    static const int INPUT_L;
    static const int INPUT_M;
    static const int INPUT_N;
    static const int INPUT_O;
    static const int INPUT_P;
    static const int INPUT_Q;
    static const int INPUT_R;
    static const int INPUT_S;
    static const int INPUT_T;
    static const int INPUT_U;
    static const int INPUT_V;
    static const int INPUT_W;
    static const int INPUT_X;
    static const int INPUT_Y;
    static const int INPUT_Z;
    static const int INPUT_0;
    static const int INPUT_1;
    static const int INPUT_2;
    static const int INPUT_3;
    static const int INPUT_4;
    static const int INPUT_5;
    static const int INPUT_6;
    static const int INPUT_7;
    static const int INPUT_8;
    static const int INPUT_9;
};

struct TiledMap : Object {
    TiledMap()
        : graph_indexes{std::make_shared<typename decltype(graph_indexes)::element_type>()}
        , graphs{std::make_shared<typename decltype(graphs)::element_type>()}
    {}

    TiledMap(std::size_t column_number, std::size_t row_number, double column_width, double row_height)
        : column_number{column_number}
        , row_number{row_number}
        , column_width{column_width}
        , row_height{row_height}
        , graph_indexes{std::make_shared<typename decltype(graph_indexes)::element_type>()}
        , graphs{std::make_shared<typename decltype(graphs)::element_type>()}
    {}

    virtual std::size_t GetColumnNumber() const { return column_number; }
    virtual void SetColumnNumber(std::size_t column_number) {this->column_number = column_number; }
    virtual std::size_t GetRowNumber() const { return row_number; }
    virtual void set_row_number(std::size_t row_number) { this->row_number = row_number; }
    virtual double GetColumnWidth() const { return column_width; }
    virtual void SetColumnWidth(double column_width) { this->column_width = column_width; }
    virtual double GetRowHeight() const { return this->row_height; }
    virtual void SetRowHeight(double row_height) { this->row_height = row_height; }
    virtual std::size_t GetGraphIndex(std::size_t x, std::size_t y) const { return graph_indexes->at(GetColumnNumber() * y + x); }
    virtual void SetGraphIndex(std::size_t x, std::size_t y, std::size_t graph_index) { graph_indexes->at(GetColumnNumber() * y + x) = graph_index; }
    virtual std::shared_ptr<Graph> GetGraph(std::size_t index) const { return graphs->at(index); }
    virtual void SetGraph(std::size_t index, std::shared_ptr<Graph> g) { graphs->at(index) = g; }

    virtual void SetGraphs(std::shared_ptr<Iterator<std::shared_ptr<Graph>>> graph_iterator) {
        graphs->clear();
        while (graph_iterator->HasNext())
            graphs->push_back(graph_iterator->Next());
    }

    virtual void Update() override {
        for (auto Graph : *graphs)
            Graph->Update();
    }

    virtual void Draw() const override {
        for (std::size_t row = 0; row < GetRowNumber(); ++row) {
            for (std::size_t column = 0; column < GetColumnNumber(); ++column) {
                std::size_t graph_index = GetGraphIndex(row, column);
                if (graph_index != EMPTY) {
                    auto g = std::make_shared<Graph>(*GetGraph(graph_index));
                    g->SetX(GetColumnWidth() * column);
                    g->SetY(GetRowHeight() * row);
                    g->SetWidth(GetColumnWidth());
                    g->SetHeight(GetRowHeight());
                    g->Draw();
                }
            }
        }
    }

    static constexpr std::size_t EMPTY = static_cast<std::size_t>(-1);

private:
    double column_width{}, row_height{};
    std::size_t column_number{}, row_number{};
    std::shared_ptr<std::vector<std::size_t>> graph_indexes;
    std::shared_ptr<std::vector<std::shared_ptr<Graph>>> graphs;
};

} //namespace DxLibPp

#endif
