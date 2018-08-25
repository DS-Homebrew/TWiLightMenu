#include "queueControl.h"

#include <nds.h>
#include <vector>
#include <functional>

std::vector<std::function<void()>> queuedFunctors;


void defer(std::function<void()> function) {
    queuedFunctors.push_back(function);
}


void execQueue() {
    for (auto functor : queuedFunctors) {
        functor();
    }
    queuedFunctors.clear();
}