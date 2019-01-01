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
DEFINE_THROW_FUNCTION(GetGraphSize)
// DEFINE_THROW_FUNCTION(DeleteGraph)
// DEFINE_THROW_FUNCTION(ScreenFlip)
// DEFINE_THROW_FUNCTION(ProcessMessage)
// DEFINE_THROW_FUNCTION(ClearDrawScreen)
DEFINE_THROW_FUNCTION(CreateFontToHandle)
DEFINE_THROW_FUNCTION(GetDrawStringWidthToHandle)
DEFINE_THROW_FUNCTION(GetFontStateToHandle)
// DEFINE_THROW_FUNCTION(DeleteFontToHandle)
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
        ChangeWindowMode_s(TRUE);
        DxLib_Init_s();
        SetDrawScreen_s(DX_SCREEN_BACK);
    }
}

DxLibPp::system_initializer_t::~system_initializer_t() {
    if (--system_initializer_counter == 0)
        DxLib_End();
}

struct DxLibPp::image::impl_t {
    int handle{-1};
    dimension get_dimension() const {
        int width{}, height{};
        GetGraphSize_s(handle, &width, &height);
        return {static_cast<double>(width), static_cast<double>(height)};
    }
};

DxLibPp::image::image(std::string_view path) {
    impl->handle = LoadGraph_s(path.data());
    dimension d = impl->get_dimension();
    this->width = d.get_width();
    this->height = d.get_height();
}

DxLibPp::image::image(const image & img)
    : x{img.x}
    , y{img.y}
    , width{img.width}
    , height{img.height}
    , theta{img.theta}
    , impl{std::make_unique<image::impl_t>(*img.impl)}
{
    *impl = *img.impl;
}

DxLibPp::image::~image() {
    DeleteGraph(impl->handle);
}

DxLibPp::image & DxLibPp::image::operator =(const image & img) {
    x = img.x;
    y = img.y;
    width = img.width;
    height = img.height;
    theta = img.theta;
    impl = std::make_unique<image::impl_t>(*img.impl);
    return *this;
}

void DxLibPp::image::draw() const {
    dimension d = impl->get_dimension();
    int center_x = static_cast<int>(get_x() + get_width() / 2);
    int center_y = static_cast<int>(get_x() + get_height() / 2);
    double extend_x_rate = get_width() / d.get_width();
    double extend_y_rate = get_height() / d.get_height();
    DrawRotaGraph3_s(
        center_x, center_y,
        center_x, center_y,
        extend_x_rate, extend_y_rate,
        get_theta(), impl->handle,
        TRUE, FALSE
    );
}

struct DxLibPp::font::impl_t {
    int handle{-1};
    double theta{};
};

DxLibPp::font::font() {
    impl->handle = CreateFontToHandle_s(nullptr, -1, -1, DX_FONTTYPE_ANTIALIASING);
}

DxLibPp::font::font(std::string_view path, int size) {
    impl->handle = CreateFontToHandle_s(path.data(), size, -1, DX_FONTTYPE_ANTIALIASING);
}

DxLibPp::font::font(const font & fnt)
    : text{fnt.text}
    , x{fnt.x}
    , y{fnt.y}
    , impl{std::make_unique<font::impl_t>(*fnt.impl)}
{}

DxLibPp::font::~font() {
    DeleteFontToHandle(impl->handle);
}

DxLibPp::font & DxLibPp::font::operator =(const font & fnt) {
    text = fnt.text;
    x = fnt.x;
    y = fnt.y;
    impl = std::make_unique<font::impl_t>(*fnt.impl);
    return *this;
}

double DxLibPp::font::get_height() const {
    int height;
    GetFontStateToHandle_s(nullptr, &height, nullptr, impl->handle);
    return static_cast<double>(height);
}

double DxLibPp::font::get_width() const {
    return GetDrawStringWidthToHandle_s(text.data(), text.length(), impl->handle);
}

double DxLibPp::font::get_theta() const {
    return impl->theta;
}

void DxLibPp::font::set_theta(double theta) {
    impl->theta = theta;
}

void DxLibPp::font::draw() const {
    DrawStringToHandle_s(x, y, text.data(), GetColor(0, 0, 0), impl->handle); //TODO
}

void DxLibPp::font::update() {}

bool DxLibPp::system::update() {
    return ScreenFlip() != -1 && ProcessMessage() != -1 && ClearDrawScreen() != -1;
}

int main();

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    return main();
}
