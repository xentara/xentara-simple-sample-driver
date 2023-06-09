// Copyright (c) embedded ocean GmbH
#pragma once

#include <xentara/process/Task.hpp>

namespace xentara::samples::simpleDriver::tasks
{

// A Xentara task used to read an input
extern const process::Task::Role kRead;
// A Xentara task used to write an output
extern const process::Task::Role kWrite;

} // namespace xentara::samples::simpleDriver::tasks