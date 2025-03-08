#include "Core/Scripting/HelperClasses/VersionComparisonFunctions.h"

namespace Scripting
{

// This helper function parses/returns the next sequence of numbers in the input_string, starting at
// the specified starting_position. Each number in the input_string is treated as ending when a
// period or the end of the string is encountered (whichever happens first).
std::string GetNextNumber(const std::string& input_string, const size_t starting_position)
{
  size_t string_length = input_string.length();
  if (string_length == 0 || starting_position >= string_length)
    return "";

  size_t index_of_char_after_end_of_number = std::string::npos;

  for (size_t i = starting_position; i < string_length; ++i)
  {
    if (input_string[i] == '.')
    {
      index_of_char_after_end_of_number = i;
      break;
    }
  }

  if (index_of_char_after_end_of_number == std::string::npos)
    index_of_char_after_end_of_number = string_length;

  return input_string.substr(starting_position,
                             index_of_char_after_end_of_number - starting_position);
}

// This helper function returns -1 if firstVersion < secondVersion, 0 if firstVersion ==
// secondVersion, and 1 if firstVersion > secondVersion
int CompareFirstVersionToSecondVersion(std::string first_version, std::string second_version)
{
  size_t number_of_periods_in_first_version =
      std::count(first_version.begin(), first_version.end(), '.');
  size_t number_of_periods_in_second_version =
      std::count(second_version.begin(), second_version.end(), '.');

  if (first_version[0] == '.')
    first_version = "0" + first_version;

  if (second_version[0] == '.')
    second_version = "0" + second_version;

  while (number_of_periods_in_first_version < number_of_periods_in_second_version)
  {
    first_version += ".0";
    ++number_of_periods_in_first_version;
  }

  while (number_of_periods_in_second_version < number_of_periods_in_first_version)
  {
    second_version += ".0";
    ++number_of_periods_in_second_version;
  }

  size_t index_of_next_digit_in_first_version = 0;
  size_t index_of_next_digit_in_second_version = 0;

  while (index_of_next_digit_in_first_version < first_version.length())
  {
    std::string next_number_in_first_version =
        GetNextNumber(first_version, index_of_next_digit_in_first_version);
    index_of_next_digit_in_first_version += (next_number_in_first_version.length() + 1);

    std::string next_number_in_second_version =
        GetNextNumber(second_version, index_of_next_digit_in_second_version);
    index_of_next_digit_in_second_version += (next_number_in_second_version.length() + 1);

    int first_number = std::stoi(next_number_in_first_version);
    int second_number = std::stoi(next_number_in_second_version);

    if (first_number > second_number)
      return 1;
    else if (first_number < second_number)
      return -1;
  }

  return 0;
}

bool IsFirstVersionGreaterThanSecondVersion(const std::string& first_version,
                                            const std::string& second_version)
{
  return CompareFirstVersionToSecondVersion(first_version, second_version) > 0;
}

bool IsFirstVersionGreaterThanOrEqualToSecondVersion(const std::string& first_version,
                                                     const std::string& second_version)
{
  return CompareFirstVersionToSecondVersion(first_version, second_version) >= 0;
}

}  // namespace Scripting
