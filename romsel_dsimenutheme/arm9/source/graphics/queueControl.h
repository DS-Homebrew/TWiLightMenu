#include <vector>
#include <functional>
#pragma once

typedef void (*graphics_callback)();


/**
 * Defer the execution of a lambda until the end of the VBlank Handler.
 */
void defer(graphics_callback function);

/**
 * Executes all the functions in the queue, and clears it.
 */
void execQueue();