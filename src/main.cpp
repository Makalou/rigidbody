#include <iostream>
#include "Application.h"

void simulate(double dt){
    std::cout << dt << std::endl;
}

class scene_renderable;
class scene_physics;

int main() {

    Application app;
    app.run();

    return 0;
}
