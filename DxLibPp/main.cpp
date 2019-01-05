#include "DxLibPp.hpp"
#include "DxLib.h"

using namespace DxLibPp;

int main() {
    std::vector<int> v = {1, 2, 3};
    auto fnt = std::make_shared<Font>();
    while (System::Update()) {
        Iterator iter = GetIterator(v);
        auto f = std::make_shared<Font>(*fnt);
        while(iter.HasNext()) {
            int next = iter.Next();
            f->SetY(f->GetY() + 20);
            f->SetText(std::to_string(next));
            f->Draw();
        }
        {
            auto f = std::make_shared<Font>(*fnt);
            f->SetText(std::to_string(Key::GetTimer(Key::INPUT_RETURN)));
            f->Draw();
        }
    }
    return EXIT_SUCCESS;
}
