#include <vector>
#include <functional>
#pragma once

/**
 * Queues a lambda to be executed at the end of the VBlank Handler.
 */
void queue(std::function<void()> function);

/**
 * Executes all the functions in the queue, and clears it.
 */
void exec_queue();