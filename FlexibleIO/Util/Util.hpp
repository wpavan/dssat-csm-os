/*=======================================================================
  Util.hpp, Felipe de Vargas, Willingthon Pavan, Fabio Oliveira
  Util definition class.
-----------------------------------------------------------------------
  REVISION HISTORY
  12/11/2017 FO Added checkDate function to DD/MM and MM/DD date format.
========================================================================*/
#ifndef Util_H
#define Util_H

#include<string>
#include<vector>

class Util
{
public:
    static std::string trim(std::string str);
    static char* convert(const std::string & s);
    static int ignore_line( std::string line);
    static int ignore_line2(std::string line);
    static int ignore_line3( std::string line);
    static int Y2K_DOY(int YRDOY);
    static bool isLeap(int year);
    static bool formatDate(int &checkDateFormat, int &dateReadFormat, std::string &date);
    static int checkDate(int dateReadFormat, std::vector<std::string> &dtidentfied, std::string &date);

};
#endif // Util_H
