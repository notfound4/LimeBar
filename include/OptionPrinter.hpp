/**********************************************************************************************************************
**         __________              ___                              ________                                         **
**         \______   \_____     __| _/ _____  _____     ____       /  _____/ _____     _____    ____    ______       **
**          |       _/\__  \   / __ | /     \ \__  \   /    \     /   \  ___ \__  \   /     \ _/ __ \  /  ___/       **
**          |    |   \ / __ \_/ /_/ ||  Y Y  \ / __ \_|   |  \    \    \_\  \ / __ \_|  Y Y  \\  ___/  \___ \        **
**          |____|_  /(____  /\____ ||__|_|  /(____  /|___|  /     \______  /(____  /|__|_|  / \___  \/____  \       **
**                 \/      \/      \/      \/      \/      \/             \/      \/       \/      \/      \/        **
**                                                         2012                                                      **
**********************************************************************************************************************/

#ifndef RAD_PRETTYOPTIONPRINTER_HPP
#define RAD_PRETTYOPTIONPRINTER_HPP

#include "CustomOptionDescription.hpp"

namespace rad
{
//*********************************************************************************************************************
  /**
   * @brief Helper class for printing program usage and options
   * @details It uses boost program_options
   */
  class OptionPrinter
  {    
  public: // interface
    /**
     * @brief Add an option to the list of option to print
     * 
     * @param optionDesc the option to add
     */
    void addOption(const CustomOptionDescription& optionDesc);

    /**
     * @brief Print the single line application usage description
     * @return the single line application usage description
     */
    std::string usage();

    /**
     * @brief Print the positional options usage description
     * @return Print the positional options usage description
     */
    std::string positionalOptionDetails();
    /**
     * @brief Print the non positional options usage description
     * @return Print the non positional options usage description
     */
    std::string optionDetails();

  public: // static
    /**
     * @brief Print the usage message and option description
     * 
     * @param appName application name
     * @param out output stream
     * @param desc options description
     * @param positionalDesc positional options description
     */
    static void printStandardAppDesc(const std::string& appName,
                                     std::ostream& out,
                                     boost::program_options::options_description desc,
                                     boost::program_options::positional_options_description* positionalDesc=NULL);
    /**
     * @brief Format the option name in the required option error
     * @details it removes the '-' at the beginning of the option's name
     * 
     * @param error the required option error
     */
    static void formatRequiredOptionError(boost::program_options::required_option& error);

  private: // data
    std::vector<CustomOptionDescription> options_;
    std::vector<CustomOptionDescription> positionalOptions_;

  }; // class

//*********************************************************************************************************************

} // namespace

#endif // RAD_PRETTYOPTIONPRINTER_HPP
