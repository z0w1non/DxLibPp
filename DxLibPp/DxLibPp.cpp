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
DEFINE_THROW_FUNCTION(GetHitKeyStateAll)
#undef DEFINE_THROW_FUNCTION

#define DEFINE_NOTHROW_FUNCTION(function_name) \
    template<typename ... Args> \
    static auto function_name##_s(Args && ... args) { \
        return function_name(args ...); \
    }
DEFINE_NOTHROW_FUNCTION(DrawRotaGraph3)
DEFINE_NOTHROW_FUNCTION(DrawStringToHandle)
#undef DEFINE_NOTHROW_FUNCTION

static char key_state[256];
static int key_timer[256];
static void init_key_timer() {
    for (std::size_t i = 0; i < std::size(key_timer); ++i)
        key_timer[i] = 0;
}
static void update_key_state() {
    GetHitKeyStateAll_s(key_state);
    for (std::size_t i = 0; i < std::size(key_state); ++i) {
        if (key_state[i])
            ++key_timer[i];
        else
            key_timer[i] = 0;
    }
}

}

DxLibPp::SystemInitializer::SystemInitializer() {
    if (system_initializer_counter++ == 0) {
        SetOutApplicationLogValidFlag_s(FALSE);
        ChangeWindowMode_s(TRUE);
        DxLib_Init_s();
        SetDrawScreen_s(DX_SCREEN_BACK);
        init_key_timer();
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

DxLibPp::Iterator<DxLibPp::Graph &> DxLibPp::Graph::LoadDivGraph(
    std::string_view path,
    std::size_t number,
    std::size_t column_number, std::size_t row_number,
    std::size_t column_width, std::size_t row_height
) {
    std::vector<int> handles(number);
    LoadDivGraph_s(std::string(path).c_str(), number, column_number, row_number, column_width, row_height, handles.data());
    auto graphs = std::make_shared<std::vector<Graph>>();
    for (std::size_t i = 0; i < handles.size(); ++i) {
        Graph g;
        g.impl->handle = std::shared_ptr<int>{new int{handles.at(i)}, impl_t::delete_handle};
        graphs->push_back(g);
    }
    return GetIterator(graphs);
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
    update_key_state();
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

bool DxLibPp::Key::CheckHit(int key_code) {
    if (key_code < 0 || key_code >= 256)
        throw std::runtime_error("key_code must be [0, 255].");
    return key_state[key_code] ? true : false;
}

int DxLibPp::Key::GetTimer(int key_code) {
    return key_timer[key_code];
}

const int DxLibPp::Key::INPUT_BACK = KEY_INPUT_BACK;
const int DxLibPp::Key::INPUT_TAB = KEY_INPUT_TAB;
const int DxLibPp::Key::INPUT_RETURN = KEY_INPUT_RETURN;

const int DxLibPp::Key::INPUT_LSHIFT = KEY_INPUT_LSHIFT;
const int DxLibPp::Key::INPUT_RSHIFT = KEY_INPUT_RSHIFT;
const int DxLibPp::Key::INPUT_LCONTROL = KEY_INPUT_LCONTROL;
const int DxLibPp::Key::INPUT_RCONTROL = KEY_INPUT_RCONTROL;
const int DxLibPp::Key::INPUT_ESCAPE = KEY_INPUT_ESCAPE;
const int DxLibPp::Key::INPUT_SPACE = KEY_INPUT_SPACE;
const int DxLibPp::Key::INPUT_PGUP = KEY_INPUT_PGUP;
const int DxLibPp::Key::INPUT_PGDN = KEY_INPUT_PGDN;
const int DxLibPp::Key::INPUT_END = KEY_INPUT_END;
const int DxLibPp::Key::INPUT_HOME = KEY_INPUT_HOME;
const int DxLibPp::Key::INPUT_LEFT = KEY_INPUT_LEFT;
const int DxLibPp::Key::INPUT_UP = KEY_INPUT_UP;
const int DxLibPp::Key::INPUT_RIGHT = KEY_INPUT_RIGHT;
const int DxLibPp::Key::INPUT_DOWN = KEY_INPUT_DOWN;
const int DxLibPp::Key::INPUT_INSERT = KEY_INPUT_INSERT;
const int DxLibPp::Key::INPUT_DELETE = KEY_INPUT_DELETE;

const int DxLibPp::Key::INPUT_MINUS = KEY_INPUT_MINUS;
const int DxLibPp::Key::INPUT_YEN = KEY_INPUT_YEN;
const int DxLibPp::Key::INPUT_PREVTRACK = KEY_INPUT_PREVTRACK;
const int DxLibPp::Key::INPUT_PERIOD = KEY_INPUT_PERIOD;
const int DxLibPp::Key::INPUT_SLASH = KEY_INPUT_SLASH;
const int DxLibPp::Key::INPUT_LALT = KEY_INPUT_LALT;
const int DxLibPp::Key::INPUT_RALT = KEY_INPUT_RALT;
const int DxLibPp::Key::INPUT_SCROLL = KEY_INPUT_SCROLL;
const int DxLibPp::Key::INPUT_SEMICOLON = KEY_INPUT_SEMICOLON;
const int DxLibPp::Key::INPUT_COLON = KEY_INPUT_COLON;
const int DxLibPp::Key::INPUT_LBRACKET = KEY_INPUT_LBRACKET;
const int DxLibPp::Key::INPUT_RBRACKET = KEY_INPUT_RBRACKET;
const int DxLibPp::Key::INPUT_AT = KEY_INPUT_AT;
const int DxLibPp::Key::INPUT_BACKSLASH = KEY_INPUT_BACKSLASH;
const int DxLibPp::Key::INPUT_COMMA = KEY_INPUT_COMMA;
const int DxLibPp::Key::INPUT_CAPSLOCK = KEY_INPUT_CAPSLOCK;
const int DxLibPp::Key::INPUT_PAUSE = KEY_INPUT_PAUSE;

const int DxLibPp::Key::INPUT_NUMPAD0 = KEY_INPUT_NUMPAD0;
const int DxLibPp::Key::INPUT_NUMPAD1 = KEY_INPUT_NUMPAD1;
const int DxLibPp::Key::INPUT_NUMPAD2 = KEY_INPUT_NUMPAD2;
const int DxLibPp::Key::INPUT_NUMPAD3 = KEY_INPUT_NUMPAD3;
const int DxLibPp::Key::INPUT_NUMPAD4 = KEY_INPUT_NUMPAD4;
const int DxLibPp::Key::INPUT_NUMPAD5 = KEY_INPUT_NUMPAD5;
const int DxLibPp::Key::INPUT_NUMPAD6 = KEY_INPUT_NUMPAD6;
const int DxLibPp::Key::INPUT_NUMPAD7 = KEY_INPUT_NUMPAD7;
const int DxLibPp::Key::INPUT_NUMPAD8 = KEY_INPUT_NUMPAD8;
const int DxLibPp::Key::INPUT_NUMPAD9 = KEY_INPUT_NUMPAD9;
const int DxLibPp::Key::INPUT_MULTIPLY = KEY_INPUT_MULTIPLY;
const int DxLibPp::Key::INPUT_ADD = KEY_INPUT_ADD;
const int DxLibPp::Key::INPUT_SUBTRACT = KEY_INPUT_SUBTRACT;
const int DxLibPp::Key::INPUT_DECIMAL = KEY_INPUT_DECIMAL;
const int DxLibPp::Key::INPUT_DIVIDE = KEY_INPUT_DIVIDE;
const int DxLibPp::Key::INPUT_NUMPADENTER = KEY_INPUT_NUMPADENTER;

const int DxLibPp::Key::INPUT_F1 = KEY_INPUT_F1;
const int DxLibPp::Key::INPUT_F2 = KEY_INPUT_F2;
const int DxLibPp::Key::INPUT_F3 = KEY_INPUT_F3;
const int DxLibPp::Key::INPUT_F4 = KEY_INPUT_F4;
const int DxLibPp::Key::INPUT_F5 = KEY_INPUT_F5;
const int DxLibPp::Key::INPUT_F6 = KEY_INPUT_F6;
const int DxLibPp::Key::INPUT_F7 = KEY_INPUT_F7;
const int DxLibPp::Key::INPUT_F8 = KEY_INPUT_F8;
const int DxLibPp::Key::INPUT_F9 = KEY_INPUT_F9;
const int DxLibPp::Key::INPUT_F10 = KEY_INPUT_F10;
const int DxLibPp::Key::INPUT_F11 = KEY_INPUT_F11;
const int DxLibPp::Key::INPUT_F12 = KEY_INPUT_F12;

const int DxLibPp::Key::INPUT_A = KEY_INPUT_A;
const int DxLibPp::Key::INPUT_B = KEY_INPUT_B;
const int DxLibPp::Key::INPUT_C = KEY_INPUT_C;
const int DxLibPp::Key::INPUT_D = KEY_INPUT_D;
const int DxLibPp::Key::INPUT_E = KEY_INPUT_E;
const int DxLibPp::Key::INPUT_F = KEY_INPUT_F;
const int DxLibPp::Key::INPUT_G = KEY_INPUT_G;
const int DxLibPp::Key::INPUT_H = KEY_INPUT_H;
const int DxLibPp::Key::INPUT_I = KEY_INPUT_I;
const int DxLibPp::Key::INPUT_J = KEY_INPUT_J;
const int DxLibPp::Key::INPUT_K = KEY_INPUT_K;
const int DxLibPp::Key::INPUT_L = KEY_INPUT_L;
const int DxLibPp::Key::INPUT_M = KEY_INPUT_M;
const int DxLibPp::Key::INPUT_N = KEY_INPUT_N;
const int DxLibPp::Key::INPUT_O = KEY_INPUT_O;
const int DxLibPp::Key::INPUT_P = KEY_INPUT_P;
const int DxLibPp::Key::INPUT_Q = KEY_INPUT_Q;
const int DxLibPp::Key::INPUT_R = KEY_INPUT_R;
const int DxLibPp::Key::INPUT_S = KEY_INPUT_S;
const int DxLibPp::Key::INPUT_T = KEY_INPUT_T;
const int DxLibPp::Key::INPUT_U = KEY_INPUT_U;
const int DxLibPp::Key::INPUT_V = KEY_INPUT_V;
const int DxLibPp::Key::INPUT_W = KEY_INPUT_W;
const int DxLibPp::Key::INPUT_X = KEY_INPUT_X;
const int DxLibPp::Key::INPUT_Y = KEY_INPUT_Y;
const int DxLibPp::Key::INPUT_Z = KEY_INPUT_Z;
const int DxLibPp::Key::INPUT_0 = KEY_INPUT_0;
const int DxLibPp::Key::INPUT_1 = KEY_INPUT_1;
const int DxLibPp::Key::INPUT_2 = KEY_INPUT_2;
const int DxLibPp::Key::INPUT_3 = KEY_INPUT_3;
const int DxLibPp::Key::INPUT_4 = KEY_INPUT_4;
const int DxLibPp::Key::INPUT_5 = KEY_INPUT_5;
const int DxLibPp::Key::INPUT_6 = KEY_INPUT_6;
const int DxLibPp::Key::INPUT_7 = KEY_INPUT_7;
const int DxLibPp::Key::INPUT_8 = KEY_INPUT_8;
const int DxLibPp::Key::INPUT_9 = KEY_INPUT_9;
