// Copyright (c) embedded ocean GmbH
#pragma once

#include <xentara/process/Event.hpp>

namespace xentara::samples::simpleDriver::events
{

// A Xentara event that is fired when an output was written
extern const process::Event::Role kWritten;
// A Xentara event that is fired when an error occurred writing an output
extern const process::Event::Role kWriteError;

} // namespace xentara::samples::simpleDriver::events