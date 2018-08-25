#include "queueControl.h"

#include <nds.h>
#include <vector>
#include <functional>


std::vector<graphics_callback> queuedFunctors;


void defer(graphics_callback function) {
    queuedFunctors.push_back(function);
}


void execQueue() {
    for (auto functor : queuedFunctors) {
        functor();
    }
    queuedFunctors.clear();
}