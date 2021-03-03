// Copyright (c) embedded ocean GmbH
#include "CustomError.hpp"

#include <system_error>
#include <string>

namespace xentara::samples::simpleDriver
{

namespace
{

	// This is a custom error category to encapsulate ErrorCode errors in std::error_code objects
	class ErrorCategory final : public std::error_category
	{
	public:
		// Override that returns a name for the category
		auto name() const noexcept -> const char * final;

		// Override that converts an error code to an error condition
		auto default_error_condition(int errorCode) const noexcept -> std::error_condition final;

		// Override that returns an error message
		auto message(int errorCode) const -> std::string final;
	};

	auto ErrorCategory::name() const noexcept -> const char *
	{
		return "Xentara.simpleSampleDriver";
	}

	auto ErrorCategory::default_error_condition(int errorCode) const noexcept -> std::error_condition
	{
		// Replace some errors with standard errors
		switch (CustomError(errorCode))
		{
		case CustomError::NoError:
			return {};

		case CustomError::CorruptFileContent:
			return std::make_error_condition(std::errc::invalid_argument);

		case CustomError::ValueOutOfRange:
			return std::make_error_condition(std::errc::result_out_of_range);

		case CustomError::UnknownError:
		default:
			return { errorCode, customErrorCategory() };
		}
	}

	auto ErrorCategory::message(int errorCode) const -> std::string
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

	// In Xentara error attributes, we use an offset to distinguish system errors from custom errors.
	// Custom errors start at 1,000,000,000.
	constexpr int kCustomErrorOffset { 1'000'000'000 };

} // namespace

// Support automatic conversion from CustomError to std::error_code
auto customErrorCategory() noexcept -> const std::error_category &
{
	// Return a static object
	static const ErrorCategory kErrorCategory;
	return kErrorCategory;
}

auto errorAttributeValue(CustomError error) noexcept -> ErrorAttributeValue
{
	// Just add
	return ErrorAttributeValue(std::underlying_type_t<CustomError>(error) + kCustomErrorOffset);
}

auto errorAttributeValue(std::error_code error) noexcept -> ErrorAttributeValue
{
	// If we have no error, use 0
	if (!error)
	{
		return 0;
	}
	
	// Use errors from the system category as-is. These should really never be negative, or too large,
	// but we make sure anyways.
	if (error.category() == std::system_category() && error.value() >= 0 && error.value() < kCustomErrorOffset)
	{
		return ErrorAttributeValue(error.value());
	}

	// Convert custom errors using the utility function
	if (error.category() == customErrorCategory())
	{
		return errorAttributeValue(CustomError(error.value()));
	}

	// Everything else is an unknown error
	return errorAttributeValue(CustomError::UnknownError);
}

} // namespace xentara::samples::simpleDriver