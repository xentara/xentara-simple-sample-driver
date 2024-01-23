// Copyright (c) embedded ocean GmbH
#pragma once

#include <system_error>
#include <type_traits>

namespace xentara::samples::simpleDriver
{

// These are custom error codes used in addition to system errors for error attributes.
// This enum is in integrated into the standard C++ error code mechanism (std::error_code),
// By supplying a specialization for std::is_error_code_enum, and a function make_error_code().
enum class CustomError
{
	// No error has occurred. This has the value 0 to conform to the conventions of std::error_code.
	NoError = 0,

	// No data has been read yet.
	NoData,
	// The format of a value file was invalid
	CorruptFileContent,
	// The value in the file was out of range
	ValueOutOfRange,

	// An unknown error occurred
	UnknownError = 999
};

// Returns the error category for custom errors
auto customErrorCategory() noexcept -> const std::error_category &;

// Support automatic conversion from CustomError to std::error_code
inline auto make_error_code(CustomError error) noexcept -> std::error_code
{
	return { int(error), customErrorCategory() };
}

} // namespace xentara::samples::simpleDriver

namespace std
{

// This specialization enables automatic conversion from CustomError to std::error_code.
template<>
struct is_error_code_enum<xentara::samples::simpleDriver::CustomError> : public std::true_type
{
};

}