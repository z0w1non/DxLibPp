#include "DxLibPp.hpp"
#include "DxLib.h"

using namespace DxLibPp;

int main() {
    auto v = std::make_shared<std::vector<int>>();
    v->push_back(1);
    v->push_back(2);
    v->push_back(3);
    auto fnt = std::make_shared<Font>();
    while (System::Update()) {
        std::shared_ptr<Iterator<int>> iter = CreateIterator(v);
        auto f = std::make_shared<Font>(*fnt);
        while(iter->HasNext()) {
            int next = iter->Next();
            f->SetY(f->GetY() + 20);
            f->SetText(std::to_string(next));
            f->Draw();
        }
    }
    return EXIT_SUCCESS;
}
