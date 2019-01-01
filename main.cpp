#include "DxLibPp.hpp"

using namespace DxLibPp;

int main() {
    font fnt{};
    while (system::update()) {
        global::create<image>("img.png");
    }

    return EXIT_SUCCESS;
}
