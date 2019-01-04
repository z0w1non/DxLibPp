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

const int DxLibPp::Key::INPUT_BACK = KEY_INPUT_BACK;	// バックスペースキー
const int DxLibPp::Key::INPUT_TAB = KEY_INPUT_TAB;	// タブキー
const int DxLibPp::Key::INPUT_RETURN = KEY_INPUT_RETURN;	// エンターキー

const int DxLibPp::Key::INPUT_LSHIFT = KEY_INPUT_LSHIFT;	// 左シフトキー
const int DxLibPp::Key::INPUT_RSHIFT = KEY_INPUT_RSHIFT;	// 右シフトキー
const int DxLibPp::Key::INPUT_LCONTROL = KEY_INPUT_LCONTROL;	// 左コントロールキー
const int DxLibPp::Key::INPUT_RCONTROL = KEY_INPUT_RCONTROL;	// 右コントロールキー
const int DxLibPp::Key::INPUT_ESCAPE = KEY_INPUT_ESCAPE;	// エスケープキー
const int DxLibPp::Key::INPUT_SPACE = KEY_INPUT_SPACE;	// スペースキー
const int DxLibPp::Key::INPUT_PGUP = KEY_INPUT_PGUP;	// ＰａｇｅＵＰキー
const int DxLibPp::Key::INPUT_PGDN = KEY_INPUT_PGDN;	// ＰａｇｅＤｏｗｎキー
const int DxLibPp::Key::INPUT_END = KEY_INPUT_END;	// エンドキー
const int DxLibPp::Key::INPUT_HOME = KEY_INPUT_HOME;	// ホームキー
const int DxLibPp::Key::INPUT_LEFT = KEY_INPUT_LEFT;	// 左キー
const int DxLibPp::Key::INPUT_UP = KEY_INPUT_UP;	// 上キー
const int DxLibPp::Key::INPUT_RIGHT = KEY_INPUT_RIGHT;	// 右キー
const int DxLibPp::Key::INPUT_DOWN = KEY_INPUT_DOWN;	// 下キー
const int DxLibPp::Key::INPUT_INSERT = KEY_INPUT_INSERT;	// インサートキー
const int DxLibPp::Key::INPUT_DELETE = KEY_INPUT_DELETE;	// デリートキー

const int DxLibPp::Key::INPUT_MINUS = KEY_INPUT_MINUS;	// －キー
const int DxLibPp::Key::INPUT_YEN = KEY_INPUT_YEN;	// ￥キー
const int DxLibPp::Key::INPUT_PREVTRACK = KEY_INPUT_PREVTRACK;	// ＾キー
const int DxLibPp::Key::INPUT_PERIOD = KEY_INPUT_PERIOD;	// ．キー
const int DxLibPp::Key::INPUT_SLASH = KEY_INPUT_SLASH;	// ／キー
const int DxLibPp::Key::INPUT_LALT = KEY_INPUT_LALT;	// 左ＡＬＴキー
const int DxLibPp::Key::INPUT_RALT = KEY_INPUT_RALT;	// 右ＡＬＴキー
const int DxLibPp::Key::INPUT_SCROLL = KEY_INPUT_SCROLL;	// ScrollLockキー
const int DxLibPp::Key::INPUT_SEMICOLON = KEY_INPUT_SEMICOLON;	// ；キー
const int DxLibPp::Key::INPUT_COLON = KEY_INPUT_COLON;	// ：キー
const int DxLibPp::Key::INPUT_LBRACKET = KEY_INPUT_LBRACKET;	// ［キー
const int DxLibPp::Key::INPUT_RBRACKET = KEY_INPUT_RBRACKET;	// ］キー
const int DxLibPp::Key::INPUT_AT = KEY_INPUT_AT;	// ＠キー
const int DxLibPp::Key::INPUT_BACKSLASH = KEY_INPUT_BACKSLASH;	// ＼キー
const int DxLibPp::Key::INPUT_COMMA = KEY_INPUT_COMMA;	// ，キー
const int DxLibPp::Key::INPUT_CAPSLOCK = KEY_INPUT_CAPSLOCK;	// CaspLockキー
const int DxLibPp::Key::INPUT_PAUSE = KEY_INPUT_PAUSE;	// PauseBreakキー

const int DxLibPp::Key::INPUT_NUMPAD0 = KEY_INPUT_NUMPAD0;	// テンキー０
const int DxLibPp::Key::INPUT_NUMPAD1 = KEY_INPUT_NUMPAD1;	// テンキー１
const int DxLibPp::Key::INPUT_NUMPAD2 = KEY_INPUT_NUMPAD2;	// テンキー２
const int DxLibPp::Key::INPUT_NUMPAD3 = KEY_INPUT_NUMPAD3;	// テンキー３
const int DxLibPp::Key::INPUT_NUMPAD4 = KEY_INPUT_NUMPAD4;	// テンキー４
const int DxLibPp::Key::INPUT_NUMPAD5 = KEY_INPUT_NUMPAD5;	// テンキー５
const int DxLibPp::Key::INPUT_NUMPAD6 = KEY_INPUT_NUMPAD6;	// テンキー６
const int DxLibPp::Key::INPUT_NUMPAD7 = KEY_INPUT_NUMPAD7;	// テンキー７
const int DxLibPp::Key::INPUT_NUMPAD8 = KEY_INPUT_NUMPAD8;	// テンキー８
const int DxLibPp::Key::INPUT_NUMPAD9 = KEY_INPUT_NUMPAD9;	// テンキー９
const int DxLibPp::Key::INPUT_MULTIPLY = KEY_INPUT_MULTIPLY;	// テンキー＊キー
const int DxLibPp::Key::INPUT_ADD = KEY_INPUT_ADD;	// テンキー＋キー
const int DxLibPp::Key::INPUT_SUBTRACT = KEY_INPUT_SUBTRACT;	// テンキー－キー
const int DxLibPp::Key::INPUT_DECIMAL = KEY_INPUT_DECIMAL;	// テンキー．キー
const int DxLibPp::Key::INPUT_DIVIDE = KEY_INPUT_DIVIDE;	// テンキー／キー
const int DxLibPp::Key::INPUT_NUMPADENTER = KEY_INPUT_NUMPADENTER;	// テンキーのエンターキー

const int DxLibPp::Key::INPUT_F1 = KEY_INPUT_F1;	// Ｆ１キー
const int DxLibPp::Key::INPUT_F2 = KEY_INPUT_F2;	// Ｆ２キー
const int DxLibPp::Key::INPUT_F3 = KEY_INPUT_F3;	// Ｆ３キー
const int DxLibPp::Key::INPUT_F4 = KEY_INPUT_F4;	// Ｆ４キー
const int DxLibPp::Key::INPUT_F5 = KEY_INPUT_F5;	// Ｆ５キー
const int DxLibPp::Key::INPUT_F6 = KEY_INPUT_F6;	// Ｆ６キー
const int DxLibPp::Key::INPUT_F7 = KEY_INPUT_F7;	// Ｆ７キー
const int DxLibPp::Key::INPUT_F8 = KEY_INPUT_F8;	// Ｆ８キー
const int DxLibPp::Key::INPUT_F9 = KEY_INPUT_F9;	// Ｆ９キー
const int DxLibPp::Key::INPUT_F10 = KEY_INPUT_F10;	// Ｆ１０キー
const int DxLibPp::Key::INPUT_F11 = KEY_INPUT_F11;	// Ｆ１１キー
const int DxLibPp::Key::INPUT_F12 = KEY_INPUT_F12;	// Ｆ１２キー

const int DxLibPp::Key::INPUT_A = KEY_INPUT_A;	// Ａキー
const int DxLibPp::Key::INPUT_B = KEY_INPUT_B;	// Ｂキー
const int DxLibPp::Key::INPUT_C = KEY_INPUT_C;	// Ｃキー
const int DxLibPp::Key::INPUT_D = KEY_INPUT_D;	// Ｄキー
const int DxLibPp::Key::INPUT_E = KEY_INPUT_E;	// Ｅキー
const int DxLibPp::Key::INPUT_F = KEY_INPUT_F;	// Ｆキー
const int DxLibPp::Key::INPUT_G = KEY_INPUT_G;	// Ｇキー
const int DxLibPp::Key::INPUT_H = KEY_INPUT_H;	// Ｈキー
const int DxLibPp::Key::INPUT_I = KEY_INPUT_I;	// Ｉキー
const int DxLibPp::Key::INPUT_J = KEY_INPUT_J;	// Ｊキー
const int DxLibPp::Key::INPUT_K = KEY_INPUT_K;	// Ｋキー
const int DxLibPp::Key::INPUT_L = KEY_INPUT_L;	// Ｌキー
const int DxLibPp::Key::INPUT_M = KEY_INPUT_M;	// Ｍキー
const int DxLibPp::Key::INPUT_N = KEY_INPUT_N;	// Ｎキー
const int DxLibPp::Key::INPUT_O = KEY_INPUT_O;	// Ｏキー
const int DxLibPp::Key::INPUT_P = KEY_INPUT_P;	// Ｐキー
const int DxLibPp::Key::INPUT_Q = KEY_INPUT_Q;	// Ｑキー
const int DxLibPp::Key::INPUT_R = KEY_INPUT_R;	// Ｒキー
const int DxLibPp::Key::INPUT_S = KEY_INPUT_S;	// Ｓキー
const int DxLibPp::Key::INPUT_T = KEY_INPUT_T;	// Ｔキー
const int DxLibPp::Key::INPUT_U = KEY_INPUT_U;	// Ｕキー
const int DxLibPp::Key::INPUT_V = KEY_INPUT_V;	// Ｖキー
const int DxLibPp::Key::INPUT_W = KEY_INPUT_W;	// Ｗキー
const int DxLibPp::Key::INPUT_X = KEY_INPUT_X;	// Ｘキー
const int DxLibPp::Key::INPUT_Y = KEY_INPUT_Y;	// Ｙキー
const int DxLibPp::Key::INPUT_Z = KEY_INPUT_Z;	// Ｚキー
const int DxLibPp::Key::INPUT_0 = KEY_INPUT_0;	// ０キー
const int DxLibPp::Key::INPUT_1 = KEY_INPUT_1;	// １キー
const int DxLibPp::Key::INPUT_2 = KEY_INPUT_2;	// ２キー
const int DxLibPp::Key::INPUT_3 = KEY_INPUT_3;	// ３キー
const int DxLibPp::Key::INPUT_4 = KEY_INPUT_4;	// ４キー
const int DxLibPp::Key::INPUT_5 = KEY_INPUT_5;	// ５キー
const int DxLibPp::Key::INPUT_6 = KEY_INPUT_6;	// ６キー
const int DxLibPp::Key::INPUT_7 = KEY_INPUT_7;	// ７キー
const int DxLibPp::Key::INPUT_8 = KEY_INPUT_8;	// ８キー
const int DxLibPp::Key::INPUT_9 = KEY_INPUT_9;	// ９キー
