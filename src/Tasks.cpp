// Copyright (c) embedded ocean GmbH
#include "Tasks.hpp"

#include <xentara/utils/core/Uuid.hpp>

#include <string_view>

namespace xentara::samples::simpleDriver::tasks
{

using namespace std::literals;
using namespace xentara::literals;

const process::Task::Role kRead { "367b54bc-2b37-4d50-8905-3ab674ce9de5"_uuid, "read"sv };

const process::Task::Role kWrite { "fb50c907-5d68-42c2-874a-65dfd0f9f715"_uuid, "write"sv };

} // namespace xentara::samples::simpleDriver::tasks