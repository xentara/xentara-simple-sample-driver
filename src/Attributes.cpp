// Copyright (c) embedded ocean GmbH
#include "Attributes.hpp"

#include <xentara/utils/core/Uuid.hpp>
#include <xentara/data/DataType.hpp>

#include <string_view>

namespace xentara::samples::simpleDriver::attributes
{
	
using namespace std::literals;

const model::Attribute kDirectory { "847d8f1c-59f7-4b98-94e6-3c6ef7ba50b5"_uuid, "directory"sv, model::Attribute::Access::ReadOnly, data::DataType::kString };
const model::Attribute kFileName { "2519cd9d-52f3-4a6b-98ba-cc3ecb33101c"_uuid, "fileName"sv, model::Attribute::Access::ReadOnly, data::DataType::kString };

const model::Attribute kError { model::Attribute::kError, model::Attribute::Access::ReadOnly, data::DataType::kInteger };
const model::Attribute kWriteError { model::Attribute::kWriteError, model::Attribute::Access::ReadOnly, data::DataType::kInteger };

} // namespace xentara::samples::simpleDriver::attributes
