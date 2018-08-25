#include <vector>
#include <functional>
#pragma once

/**
 * Defer the execution of a lambda until the end of the VBlank Handler.
 */
void defer(std::function<void()> function);

/**
 * Executes all the functions in the queue, and clears it.
 */
void execQueue();