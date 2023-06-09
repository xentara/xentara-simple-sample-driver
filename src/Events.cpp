// Copyright (c) embedded ocean GmbH
#include "Events.hpp"

#include <xentara/utils/core/Uuid.hpp>

#include <string_view>

namespace xentara::samples::simpleDriver::events
{

using namespace std::literals;
using namespace xentara::literals;

const process::Event::Role kWritten { "3d915fc1-3ca7-4694-97d0-b60bab9242cd"_uuid, "written"sv };

const process::Event::Role kWriteError { "aa747a5e-0d57-4c67-b65f-93212b86dd2d"_uuid, "writeError"sv };

} // namespace xentara::samples::simpleDriver::events