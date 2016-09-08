/**********************************************************************************************************************
**         __________              ___                              ________                                         **
**         \______   \_____     __| _/ _____  _____     ____       /  _____/ _____     _____    ____    ______       **
**          |       _/\__  \   / __ | /     \ \__  \   /    \     /   \  ___ \__  \   /     \ _/ __ \  /  ___/       **
**          |    |   \ / __ \_/ /_/ ||  Y Y  \ / __ \_|   |  \    \    \_\  \ / __ \_|  Y Y  \\  ___/  \___ \        **
**          |____|_  /(____  /\____ ||__|_|  /(____  /|___|  /     \______  /(____  /|__|_|  / \___  \/____  \       **
**                 \/      \/      \/      \/      \/      \/             \/      \/       \/      \/      \/        **
**                                                         2012                                                      **
**********************************************************************************************************************/

#ifndef RAD_CUSTOMOPTIONDESCRIPTION_HPP
#define RAD_CUSTOMOPTIONDESCRIPTION_HPP

#include "boost/program_options.hpp"

#include <string>

namespace rad
{
//*********************************************************************************************************************
  /**
   * @brief Format a string describing a given option
   */
  class CustomOptionDescription
  {
  public:
    /**
     * @brief Creat a descibtion for a given option
     * 
     * @param option the option to descibe
     */
    CustomOptionDescription(boost::shared_ptr<boost::program_options::option_description> option);

    /**
     * @brief Check if an option is a positional option
     * @details Also removes all 's' from its display name
     * 
     * @param positionalDesc the list of positional options
     */
    void checkIfPositional(const boost::program_options::positional_options_description& positionalDesc);

    /**
     * @brief Format a string describing the option
     * @return the string describing the option
     */
    std::string getOptionUsageString();

  public:
    std::string optionID_; //!< The ID of the option
    std::string optionDisplayName_; //!< The name associated to the option
    std::string optionDescription_; //!< The description of the option
    std::string optionFormatName_; //!< The option name, formatted suitably for usage message

    bool required_; //!< Is the option required
    bool hasShort_; //!< Does the option have a shorthand
    bool hasArgument_; //!< Does the option require an argument
    bool isPositional_; //!< Is the option positional


  }; // class

//*********************************************************************************************************************

} // namespace

#endif // RAD_CUSTOMOPTIONDESCRIPTION_HPP
