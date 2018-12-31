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

system_initializer_t::system_initializer_t() {
    if (system_initializer_counter++ == 0) {
        ChangeWindowMode_s(TRUE);
        DxLib_Init_s();
        SetDrawScreen_s(DX_SCREEN_BACK);
    }
}

system_initializer_t::~system_initializer_t() {
    if (--system_initializer_counter == 0)
        DxLib_End();
}

struct image::impl_t {
    int handle;
    dimension get_dimension() const {
        int width{}, height{};
        GetGraphSize_s(handle, &width, &height);
        return {static_cast<double>(width), static_cast<double>(height)};
    }
};

image::image(std::string_view path) {
    impl->handle = LoadGraph_s(path.data());
    dimension d = impl->get_dimension();
    this->width = d.get_width();
    this->height = d.get_height();
}

image::image(const image & img)
    : x{img.x}
    , y{img.y}
    , width{img.width}
    , height{img.height}
    , theta{img.theta}
    , impl{std::make_unique<image::impl_t>(*img.impl)}
{
    *impl = *img.impl;
}

image::~image() {
    DeleteGraph(impl->handle);
}

image & image::operator =(const image & img) {
    x = img.x;
    y = img.y;
    width = img.width;
    height = img.height;
    theta = img.theta;
    impl = std::make_unique<image::impl_t>(*img.impl);
    return *this;
}

void image::draw() const {
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

struct font::impl_t {
    int handle;
};

font::font() {
    impl->handle = CreateFontToHandle_s(nullptr, -1, -1, DX_FONTTYPE_ANTIALIASING);
}

font::font(std::string_view path, int size) {
    impl->handle = CreateFontToHandle_s(path.data(), size, -1, DX_FONTTYPE_ANTIALIASING);
    GetFontStateToHandle_s(std::string{path}.data(), &size, nullptr, impl->handle);
}

font::font(const font & fnt)
    : text{fnt.text}
    , x{fnt.x}
    , y{fnt.y}
    , impl{std::make_unique<font::impl_t>(*fnt.impl)}
{}

font::~font() {
    DeleteFontToHandle(impl->handle);
}

font & font::operator =(const font & fnt) {
    text = fnt.text;
    x = fnt.x;
    y = fnt.y;
    impl = std::make_unique<font::impl_t>(*fnt.impl);
    return *this;
}

double font::get_width() const {
    return GetDrawStringWidthToHandle_s(text.c_str(), text.length(), impl->handle);
}

void font::draw() const {
    DrawStringToHandle_s(x, y, text.data(), GetColor(0, 0, 0), impl->handle); //TODO
}

bool system::update() {
    return ScreenFlip() != -1 && ProcessMessage() != -1 && ClearDrawScreen() != -1;
}

int main();

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    return main();
}
