#include "DxLibPp.hpp"
#include "DxLib.h"

namespace {
    int system_initializer_counter = 0;

#define DEFINE_THROW_FUNCTION(function_name) \
    template<typename ... Args> \
    static auto function_name##_s(Args && ... args) { \
        auto result = function_name(args ...); \
        if (result == -1) \
            throw std::runtime_error(#function_name " failed."); \
        return result; \
    }
DEFINE_THROW_FUNCTION(DxLib_Init)
// DEFINE_THROW_FUNCTION(DxLib_End)
DEFINE_THROW_FUNCTION(ChangeWindowMode)
DEFINE_THROW_FUNCTION(SetDrawScreen)
DEFINE_THROW_FUNCTION(LoadGraph)
DEFINE_THROW_FUNCTION(LoadDivGraph)
DEFINE_THROW_FUNCTION(GetGraphSize)
DEFINE_THROW_FUNCTION(DeleteGraph)
// DEFINE_THROW_FUNCTION(ScreenFlip)
// DEFINE_THROW_FUNCTION(ProcessMessage)
// DEFINE_THROW_FUNCTION(ClearDrawScreen)
DEFINE_THROW_FUNCTION(CreateFontToHandle)
DEFINE_THROW_FUNCTION(GetDrawStringWidthToHandle)
DEFINE_THROW_FUNCTION(GetFontStateToHandle)
DEFINE_THROW_FUNCTION(DeleteFontToHandle)
DEFINE_THROW_FUNCTION(SetOutApplicationLogValidFlag)
DEFINE_THROW_FUNCTION(GetScreenState)
#undef DEFINE_THROW_FUNCTION

#define DEFINE_NOTHROW_FUNCTION(function_name) \
    template<typename ... Args> \
    static auto function_name##_s(Args && ... args) { \
        return function_name(args ...); \
    }
DEFINE_NOTHROW_FUNCTION(DrawRotaGraph3)
DEFINE_NOTHROW_FUNCTION(DrawStringToHandle)
#undef DEFINE_NOTHROW_FUNCTION

}

DxLibPp::system_initializer_t::system_initializer_t() {
    if (system_initializer_counter++ == 0) {
        SetOutApplicationLogValidFlag_s(FALSE);
        ChangeWindowMode_s(TRUE);
        DxLib_Init_s();
        SetDrawScreen_s(DX_SCREEN_BACK);
    }
}

DxLibPp::system_initializer_t::~system_initializer_t() {
    if (--system_initializer_counter == 0)
        DxLib_End();
}

struct DxLibPp::graph::impl_t {
    std::shared_ptr<int> handle{new int{-1}, &delete_handle };
    dimension get_dimension() const {
        int width{}, height{};
        GetGraphSize_s(*handle, &width, &height);
        return {static_cast<double>(width), static_cast<double>(height)};
    }
    static void delete_handle(int * ptr) {
        if (*ptr != -1)
            DeleteGraph_s(*ptr);
        delete ptr;
    }
};

DxLibPp::graph::graph()
    : impl{std::make_unique<impl_t>()}
{}

DxLibPp::graph::graph(std::string_view path)
    : impl{std::make_unique<impl_t>()}
{
    load(path);
}

DxLibPp::graph::graph(const graph & g)
    : x{g.x}
    , y{g.y}
    , width{g.width}
    , height{g.height}
    , theta{g.theta}
    , impl{std::make_unique<graph::impl_t>(*g.impl)}
{}

DxLibPp::graph::~graph() {}

DxLibPp::graph & DxLibPp::graph::operator =(const graph & g) {
    x = g.x;
    y = g.y;
    width = g.width;
    height = g.height;
    theta = g.theta;
    impl = std::make_unique<graph::impl_t>(*g.impl);
    return *this;
}

void DxLibPp::graph::draw() const {
    dimension d = impl->get_dimension();
    int center_x = static_cast<int>(get_x() + get_width() / 2);
    int center_y = static_cast<int>(get_x() + get_height() / 2);
    double extend_x_rate = get_width() / d.get_width();
    double extend_y_rate = get_height() / d.get_height();
    DrawRotaGraph3_s(
        center_x, center_y,
        center_x, center_y,
        extend_x_rate, extend_y_rate,
        get_theta(), *impl->handle,
        TRUE, FALSE
    );
}

void DxLibPp::graph::load(std::string_view path) {
    int handle = LoadGraph_s(std::string{path}.c_str());
    impl->handle = std::shared_ptr<int>(new int{handle}, &impl_t::delete_handle);
    dimension d = impl->get_dimension();
    width = d.get_width();
    height = d.get_height();
}

std::shared_ptr<DxLibPp::iterator<std::shared_ptr<DxLibPp::graph>>> DxLibPp::graph::load_div_graph(
    std::string_view path,
    std::size_t number,
    std::size_t column_number, std::size_t row_number,
    std::size_t column_width, std::size_t row_height
) {
    std::vector<int> handles(number);
    LoadDivGraph_s(std::string(path).c_str(), number, column_number, row_number, column_width, row_height, handles.data());
    auto graphs = std::make_shared<std::vector<std::shared_ptr<graph>>>();
    for (std::size_t i = 0; i < handles.size(); ++i) {
        auto g = std::make_shared<graph>();
        g->impl->handle = std::shared_ptr<int>{new int{handles.at(i)}, impl_t::delete_handle};
        graphs->push_back(g);
    }
    return make_iterator(graphs);
}

struct DxLibPp::font::impl_t {
    std::shared_ptr<int> handle{new int{-1}, &delete_handle};
    static void delete_handle(int * ptr) {
        if (*ptr != -1)
            DeleteFontToHandle_s(*ptr);
        delete ptr;
    }
};

DxLibPp::font::font()
    : impl{std::make_unique<impl_t>()}
{
    int handle = CreateFontToHandle_s(nullptr, -1, -1, DX_FONTTYPE_ANTIALIASING);
    impl->handle = std::shared_ptr<int>(new int{ handle }, &impl_t::delete_handle);
}

DxLibPp::font::font(std::string_view path, int size)
    : impl{std::make_unique<impl_t>()}
{
    load(path, size);
}

DxLibPp::font::font(const font & fnt)
    : text{fnt.text}
    , x{fnt.x}
    , y{fnt.y}
    , theta{fnt.theta}
    , impl{std::make_unique<font::impl_t>(*fnt.impl)}
{}

DxLibPp::font::~font() {}

DxLibPp::font & DxLibPp::font::operator =(const font & fnt) {
    text = fnt.text;
    x = fnt.x;
    y = fnt.y;
    theta = fnt.theta;
    impl = std::make_unique<font::impl_t>(*fnt.impl);
    return *this;
}

double DxLibPp::font::get_height() const {
    int height;
    GetFontStateToHandle_s(nullptr, &height, nullptr, *impl->handle);
    return static_cast<double>(height);
}

double DxLibPp::font::get_width() const {
    return GetDrawStringWidthToHandle_s(text.data(), static_cast<int>(text.length()), *impl->handle);
}

double DxLibPp::font::get_theta() const {
    return theta;
}

void DxLibPp::font::set_theta(double theta) {
    theta = theta;
}

void DxLibPp::font::load(std::string_view path, int size) {
    int handle = CreateFontToHandle_s(path.data(), size, -1, DX_FONTTYPE_ANTIALIASING);
    impl->handle = std::shared_ptr<int>(new int{ handle }, &impl_t::delete_handle);
}

void DxLibPp::font::draw() const {
    DrawStringToHandle_s(static_cast<int>(x), static_cast<int>(y), text.data(), GetColor(255, 255, 255), *impl->handle); //TODO
}

void DxLibPp::font::update() {}

std::vector<std::function<void()>> DxLibPp::global::get_attachment_resuests() {
    static std::vector<std::function<void()>> requests; return requests;
}

void DxLibPp::global::resolve_attachment() {
    for (auto & request : get_attachment_resuests()) request();
}

bool DxLibPp::system::update() {
    return ScreenFlip() != -1 && ProcessMessage() != -1 && ClearDrawScreen() != -1;
}

int DxLibPp::screen::get_width() {
    int width{}, height{}, color_bit_depth{};
    GetScreenState_s(&width, &height, &color_bit_depth);
    return width;
}

int DxLibPp::screen::get_height() {
    int width{}, height{}, color_bit_depth{};
    GetScreenState_s(&width, &height, &color_bit_depth);
    return height;
}
