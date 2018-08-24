#include "queueControl.h"

#include <vector>
#include <functional>

std::vector<std::function<void()>> queuedFunctors;


void defer(std::function<void()> function) {
    queuedFunctors.push_back(function);
}

void exec_queue() {
    for (auto functor : queuedFunctors) {
        functor();
    }
    queuedFunctors.clear();
}