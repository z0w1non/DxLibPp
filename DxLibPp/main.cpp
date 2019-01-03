#include "DxLibPp.hpp"
#include "DxLib.h"

using namespace DxLibPp;

int main() {
    auto v = std::make_shared<std::vector<int>>();
    v->push_back(1);
    v->push_back(2);
    v->push_back(3);
    font fnt{};
    while (system::update()) {
        auto iter = make_iterator(v);
        font f{fnt};
        while(iter->has_next()) {
            auto next = iter->next();
            f.set_y(f.get_y() + 20);
            f.set_text(std::to_string(next));
            f.draw();
        }
    }
    return EXIT_SUCCESS;
}
