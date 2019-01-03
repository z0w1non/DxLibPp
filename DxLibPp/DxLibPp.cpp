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
DEFINE_THROW_FUNCTION(LoadSoundMem)
DEFINE_THROW_FUNCTION(PlaySoundMem)
DEFINE_THROW_FUNCTION(CheckSoundMem)
DEFINE_THROW_FUNCTION(StopSoundMem)
DEFINE_THROW_FUNCTION(DeleteSoundMem)
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

DxLibPp::SystemInitializer::SystemInitializer() {
    if (system_initializer_counter++ == 0) {
        SetOutApplicationLogValidFlag_s(FALSE);
        ChangeWindowMode_s(TRUE);
        DxLib_Init_s();
        SetDrawScreen_s(DX_SCREEN_BACK);
    }
}

DxLibPp::SystemInitializer::~SystemInitializer() {
    if (--system_initializer_counter == 0)
        DxLib_End();
}

struct DxLibPp::Graph::impl_t {
    std::shared_ptr<int> handle{new int{-1}, &delete_handle };
    Dimension get_dimension() const {
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

DxLibPp::Graph::Graph()
    : impl{std::make_unique<impl_t>()}
{}

DxLibPp::Graph::Graph(std::string_view path)
    : impl{std::make_unique<impl_t>()}
{
    Load(path);
}

DxLibPp::Graph::Graph(const Graph & g)
    : x{g.x}
    , y{g.y}
    , width{g.width}
    , height{g.height}
    , theta{g.theta}
    , impl{std::make_unique<Graph::impl_t>(*g.impl)}
{}

DxLibPp::Graph::~Graph() {}

DxLibPp::Graph & DxLibPp::Graph::operator =(const Graph & g) {
    x = g.x;
    y = g.y;
    width = g.width;
    height = g.height;
    theta = g.theta;
    impl = std::make_unique<Graph::impl_t>(*g.impl);
    return *this;
}

void DxLibPp::Graph::Draw() const {
    Dimension d = impl->get_dimension();
    int center_x = static_cast<int>(GetX() + GetWidth() / 2);
    int center_y = static_cast<int>(GetY() + GetHeight() / 2);
    double extend_x_rate = GetWidth() / d.GetWidth();
    double extend_y_rate = GetHeight() / d.GetHeight();
    DrawRotaGraph3_s(
        center_x, center_y,
        center_x, center_y,
        extend_x_rate, extend_y_rate,
        GetTheta(), *impl->handle,
        TRUE, FALSE
    );
}

void DxLibPp::Graph::Load(std::string_view path) {
    int handle = LoadGraph_s(std::string{path}.c_str());
    impl->handle = std::shared_ptr<int>(new int{handle}, &impl_t::delete_handle);
    Dimension d = impl->get_dimension();
    width = d.GetWidth();
    height = d.GetHeight();
}

std::shared_ptr<DxLibPp::Iterator<std::shared_ptr<DxLibPp::Graph>>> DxLibPp::Graph::LoadDivGraph(
    std::string_view path,
    std::size_t number,
    std::size_t column_number, std::size_t row_number,
    std::size_t column_width, std::size_t row_height
) {
    std::vector<int> handles(number);
    LoadDivGraph_s(std::string(path).c_str(), number, column_number, row_number, column_width, row_height, handles.data());
    auto graphs = std::make_shared<std::vector<std::shared_ptr<Graph>>>();
    for (std::size_t i = 0; i < handles.size(); ++i) {
        auto g = std::make_shared<Graph>();
        g->impl->handle = std::shared_ptr<int>{new int{handles.at(i)}, impl_t::delete_handle};
        graphs->push_back(g);
    }
    return CreateIterator(graphs);
}

struct DxLibPp::Font::impl_t {
    std::shared_ptr<int> handle{new int{-1}, &delete_handle};
    static void delete_handle(int * ptr) {
        if (*ptr != -1)
            DeleteFontToHandle_s(*ptr);
        delete ptr;
    }
};

DxLibPp::Font::Font()
    : impl{std::make_unique<impl_t>()}
{
    int handle = CreateFontToHandle_s(nullptr, -1, -1, DX_FONTTYPE_ANTIALIASING);
    impl->handle = std::shared_ptr<int>(new int{ handle }, &impl_t::delete_handle);
}

DxLibPp::Font::Font(std::string_view path, int size)
    : impl{std::make_unique<impl_t>()}
{
    Load(path, size);
}

DxLibPp::Font::Font(const Font & fnt)
    : text{fnt.text}
    , x{fnt.x}
    , y{fnt.y}
    , theta{fnt.theta}
    , impl{std::make_unique<Font::impl_t>(*fnt.impl)}
{}

DxLibPp::Font::~Font() {}

DxLibPp::Font & DxLibPp::Font::operator =(const Font & fnt) {
    text = fnt.text;
    x = fnt.x;
    y = fnt.y;
    theta = fnt.theta;
    impl = std::make_unique<Font::impl_t>(*fnt.impl);
    return *this;
}

double DxLibPp::Font::GetHeight() const {
    int height;
    GetFontStateToHandle_s(nullptr, &height, nullptr, *impl->handle);
    return static_cast<double>(height);
}

double DxLibPp::Font::GetWidth() const {
    return GetDrawStringWidthToHandle_s(text.data(), static_cast<int>(text.length()), *impl->handle);
}

double DxLibPp::Font::GetTheta() const {
    return theta;
}

void DxLibPp::Font::SetTheta(double theta) {
    theta = theta;
}

void DxLibPp::Font::Load(std::string_view path, int size) {
    int handle = CreateFontToHandle_s(path.data(), size, -1, DX_FONTTYPE_ANTIALIASING);
    impl->handle = std::shared_ptr<int>(new int{ handle }, &impl_t::delete_handle);
}

void DxLibPp::Font::Draw() const {
    DrawStringToHandle_s(static_cast<int>(x), static_cast<int>(y), text.data(), GetColor(255, 255, 255), *impl->handle); //TODO
}

void DxLibPp::Font::Update() {}

std::vector<std::function<void()>> DxLibPp::Global::GetAttachmentResuests() {
    static std::vector<std::function<void()>> requests; return requests;
}

void DxLibPp::Global::ResolveAttachment() {
    for (auto & request : GetAttachmentResuests()) request();
}

bool DxLibPp::System::Update() {
    return ScreenFlip() != -1 && ProcessMessage() != -1 && ClearDrawScreen() != -1;
}

int DxLibPp::Screen::GetWidth() {
    int width{}, height{}, color_bit_depth{};
    GetScreenState_s(&width, &height, &color_bit_depth);
    return width;
}

int DxLibPp::Screen::GetHeight() {
    int width{}, height{}, color_bit_depth{};
    GetScreenState_s(&width, &height, &color_bit_depth);
    return height;
}

struct DxLibPp::Sound::impl_t {
    std::shared_ptr<int> handle{new int{-1}, &delete_handle};
    static void delete_handle(int * ptr) {
        if (*ptr != -1)
            DeleteSoundMem_s(*ptr);
        delete ptr;
    }
};

DxLibPp::Sound::Sound()
    : impl{std::make_unique<impl_t>()}
{}

DxLibPp::Sound::Sound(std::string_view path) {
    Load(path);
}

DxLibPp::Sound::Sound(const Sound & obj)
    : impl{std::make_unique<impl_t>(*obj.impl)}
{}

DxLibPp::Sound::~Sound() {}

DxLibPp::Sound & DxLibPp::Sound::operator =(const Sound & obj) {
    *impl = *obj.impl;
    return *this;
}

void DxLibPp::Sound::Play(int play_type, bool top_position_flag) {
    PlaySoundMem_s(*impl->handle, play_type, top_position_flag ? TRUE : FALSE);
}

void DxLibPp::Sound::Stop() {
    StopSoundMem_s(*impl->handle);
}

void DxLibPp::Sound::Load(std::string_view path) {
    int handle = LoadSoundMem_s(std::string{path}.c_str());
    impl->handle = std::shared_ptr<int>(new int{handle}, &impl_t::delete_handle);
}

bool DxLibPp::Sound::Check() const {
    return CheckSoundMem_s(*impl->handle) ? true : false;
}
