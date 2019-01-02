#include "DxLibPp.hpp"

using namespace DxLibPp;

int main() {
    font fnt{};
    while (system::update()) {
        fnt.set_text("hogehoge");
        fnt.draw();
    }

    return EXIT_SUCCESS;
}
