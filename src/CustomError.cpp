// Copyright (c) embedded ocean GmbH
#include "CustomError.hpp"

#include <system_error>
#include <string>

namespace xentara::samples::simpleDriver
{

namespace
{

	// This is a custom error category to encapsulate CustomError errors in std::error_code objects
	class CustomErrorCategory final : public std::error_category
	{
	public:
		// Override that returns a name for the category
		auto name() const noexcept -> const char * final;

		// Override that returns an error message
		auto message(int errorCode) const -> std::string final;
	};

	auto CustomErrorCategory::name() const noexcept -> const char *
	{
		return "Xentara.simpleSampleDriver";
	}

	auto CustomErrorCategory::message(int errorCode) const -> std::string
	{
		using namespace std::literals;

		// Check the error code
		switch (CustomError(errorCode))
		{
		case CustomError::NoError:
			return "success"s;

		case CustomError::NoData:
			return "no data was read yet"s;

		case CustomError::CorruptFileContent:
			return "the data file does not contain a valid number"s;

		case CustomError::ValueOutOfRange:
			return "the value in the data file is out of range"s;

		case CustomError::UnknownError:
		default:
			return "an unknown error occurred"s;
		}
	}

} // namespace

auto customErrorCategory() noexcept -> const std::error_category &
{
	// Return a static object
	static const CustomErrorCategory kErrorCategory;
	return kErrorCategory;
}

} // namespace xentara::samples::simpleDriver