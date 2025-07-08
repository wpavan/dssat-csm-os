/*=======================================================================
  Util.hpp, Felipe de Vargas, Willingthon Pavan, Fabio Oliveira
  Util definition class.
-----------------------------------------------------------------------
  REVISION HISTORY
  12/11/2017 FO Added checkDate function to DD/MM and MM/DD date format.
========================================================================*/
#ifndef Util_H
#define Util_H

#include <string>
#include <vector>

class Util{
  private:
    static const std::string BASE52_CODING;
    static const std::string INCLUDES_CODE;
  public:
    // NOTE: Look into static vs. inline for speed improvements.
    static std::string trim(std::string str);
    static char* convert(const std::string & s);
    static int ignore_line( std::string line);
    static int ignore_line2(std::string line);
    static int ignore_line3( std::string line);
    static int Y2K_DOY(int YRDOY);
    static bool isLeap(int year);
    static bool formatDate(int &checkDateFormat, int &dateReadFormat, std::string &date);
    static int checkDate(int dateReadFormat, std::vector<std::string> &dtidentfied, std::string &date);

    /**
     * Encode a hash value into an alpha string of base 52.
     * 
     * @param hashValue The hash value to encode.
     * 
     * @return The base 52 encoded string.
     */
    static std::string base52Encode(size_t hashValue);

    /**
     * Write the code inside the yaml file to a cpp file for compilation.
     * 
     * @param code The code to write.
     * @param filename The name of the file to write to.
     */
    static void writeInjectedCode(const std::string& code, const std::string& filename);
};

#endif // Util_H
