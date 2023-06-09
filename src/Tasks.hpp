// Copyright (c) embedded ocean GmbH
#pragma once

#include <xentara/process/Task.hpp>

/// @brief Contains the Xentara tasks of the driver
namespace xentara::samples::simpleDriver::tasks
{

/// @brief A Xentara tasks used to read an input
extern const process::Task::Role kRead;
/// @brief A Xentara tasks used to write an output
extern const process::Task::Role kWrite;

} // namespace xentara::samples::simpleDriver::tasks