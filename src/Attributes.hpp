// Copyright (c) embedded ocean GmbH
#pragma once

#include <xentara/model/Attribute.hpp>

namespace xentara::samples::simpleDriver::attributes
{

// A Xentara attribute containing a directory path
extern const model::Attribute kDirectory;
// A Xentara attribute containing the file name of an I/O
extern const model::Attribute kFileName;

// A Xentara attribute containing a read error code
extern const model::Attribute kError;
// A Xentara attribute containing a write error code
extern const model::Attribute kWriteError;

} // namespace xentara::samples::simpleDriver::attributes