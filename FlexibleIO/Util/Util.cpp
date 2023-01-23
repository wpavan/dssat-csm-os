/*=======================================================================
  Util.cpp, Felipe de Vargas, Willingthon Pavan, Fabio Oliveira
  Implementation of the Util class.
-----------------------------------------------------------------------
  REVISION HISTORY
  07/10/2016 FV Written.
	12/11/2017 FO Added new date format.
	12/11/2017 FO Added checkDate function to DD/MM and MM/DD date format.
	12/11/2017 FO Added variable to return error code.
========================================================================*/
#include "Util.hpp"
#include <string>
#include <cstring>
#include <cstdlib>
#include <new>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>


std::string Util::trim(std::string str)
{
    str.erase(0, str.find_first_not_of(" "));
    str.erase(str.find_last_not_of(" ") + 1, str.size());
    return str;
}
char *Util::convert(const std::string & s)
{
    char *pc = new char[s.size() + 1];
    strcpy(pc, s.c_str());
    return pc;
}
int Util::ignore_line( std::string line)
{
    line = trim(line);
    if (line[0] == '*' || line[0] == '$')
    {
        return 2;
    }
    if (line[0] != '!' && line[0] != '@' && line.size() > 1)
        if (!line.empty())
        {
            return 1;
        }
    return 0;

}
int Util::ignore_line2(std::string line)
{
    line = trim(line);
    if (line[0] == '*' || line[0] == '$')
    {
        return 2;
    }
    if (line[0] == '@')
    {
        return 3;
    }
    if (line[0] != '!' && line[0] != '@' && line.size() > 1)
        if (!line.empty())
        {
            return 1;
        }
    return 0;

}
int Util::ignore_line3( std::string line)
{
    line = trim(line);
    if (line[0] == '*')
        return 2;

    if (line[0] == '$')
        return 4;

    if (line[0] == '#' || line[0] == '@')
        return 3;

    if (line[0] != '!' && line[0] != '@' && line.size() > 1 && !line.empty())
        if (!line.empty())
            return 1;

    return 0;

}

int Util::Y2K_DOY(int YRDOY)
{

    int DOY, YR;

    if (YRDOY <= 99365)
    {
        YR  = int(YRDOY / 1000);
        DOY = YRDOY - YR * 1000;
        if (YRDOY > 0)
        {


            if (YR <= 30)
            {
                YRDOY = (2000 + YR) * 1000 + DOY;
            }
            else
                YRDOY = (1900 + YR) * 1000 + DOY;
        }
    }
    return YRDOY;
}

bool Util::isLeap(int year)
{
    if ( (year % 4 == 0 && year % 100 != 0) || ( year % 400 == 0))
        return true;
    else
        return false;
}

bool Util::formatDate(int &checkDateFormat, int &dateReadFormat, std::string &date)
{
    int yr, m, d, yrdoy = 0;
    std::string yyyy, mm, dd, straux;
    std::string dateout = date;

    yyyy = mm = dd = straux = "";

    if(dateout.find("/") != std::string::npos)
    {
        straux = dateout.substr(0, dateout.find_first_of("/"));
        if(straux.size() <= 2)
        {
            mm = straux;
            dateout.erase(0, dateout.find_first_of("/") + 1);
            dd = dateout.substr(0, dateout.find_first_of("/"));
            dateout.erase(0, dateout.find_first_of("/") + 1);
            yyyy = dateout;
        }
        else
        {
            yyyy = straux;
            dateout.erase(0, dateout.find_first_of("/") + 1);
            mm = dateout.substr(0, dateout.find_first_of("/"));
            dateout.erase(0, dateout.find_first_of("/") + 1);
            dd = dateout;
        }

        if(yyyy.size() != 4 || mm.size() != 2 && mm.size() != 1 || dd.size() != 2 && dd.size() != 1)
        {
            checkDateFormat = 59;
            return false;
        }

        yr = strtol(yyyy.c_str(), NULL, 10);
        m = strtol(mm.c_str(), NULL, 10);
        d = strtol(dd.c_str(), NULL, 10);
        int daysPerMonth[] = {0, 31, (isLeap(yr) ? 29 : 28), 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

        if(m != d && m <= 12 && d <= 12)
            return false;
        else
        {
            if( m > 12)
            {
                int iaux = d;
                d = m;
                m = iaux;

                dateReadFormat = 1;
            }
            else
                dateReadFormat = 0;

            for (int i = 0; i < m; i++)
                yrdoy += daysPerMonth[i];
            yrdoy += d;
            yr *= 1000;
            yrdoy += yr;

            date = std::to_string(Y2K_DOY(yrdoy));
            return true;
        }

    }
    else if(dateout.find("-") != std::string::npos)
    {
        yyyy = dateout.substr(0, dateout.find_first_of("-"));
        dateout.erase(0, dateout.find_first_of("-") + 1);
        mm = dateout.substr(0, dateout.find_first_of("-"));
        dateout.erase(0, dateout.find_first_of("-") + 1);
        dd = dateout;

        if(yyyy.size() != 4 || mm.size() != 2 && mm.size() != 1 || dd.size() != 2 && dd.size() != 1)
        {
            checkDateFormat = 59;
            return false;
        }

        yr = strtol(yyyy.c_str(), NULL, 10);
        m = strtol(mm.c_str(), NULL, 10);
        d = strtol(dd.c_str(), NULL, 10);
        int daysPerMonth[] = {0, 31, (isLeap(yr) ? 29 : 28), 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

        for (int i = 0; i < m; i++)
            yrdoy += daysPerMonth[i];
        yrdoy += d;
        yr *= 1000;
        yrdoy += yr;

        date = std::to_string(Y2K_DOY(yrdoy));
        return true;

    }
    else if(dateout.size() == 7)
    {
        yyyy = dateout.substr(0, 4);
        dateout.erase(0,4);
        dd = dateout;
        if(strtol(dd.c_str(), NULL, 10) >= 1 && strtol(dd.c_str(), NULL, 10) <= 366)
        {
            return true;
        }
        else
        {
            checkDateFormat = 59;
            return false;
        }
    }
    else if(dateout.size() == 5)
    {
        if (strtol(dateout.c_str(), NULL, 10) <= 1000000)
        {
            date = std::to_string(Y2K_DOY(strtol(dateout.c_str(), NULL, 10)));
            return true;
        }
    }
    else
    {
        checkDateFormat = 59;
        return false;
    }

}

int Util::checkDate(int dateReadFormat, std::vector<std::string> &dtidentfied, std::string &date)
{
    int iaux, yr, m, d, yrdoy = 0;
    std::string yyyy, mm, dd, straux;
    std::string dateout2, dateout = date;

    yyyy = mm = dd = straux = "";

    if(dateout.find("/") != std::string::npos)
    {
        straux = dateout.substr(0, dateout.find_first_of("/"));
        if(straux.size() <= 2)
        {
            mm = straux;
            dateout.erase(0, dateout.find_first_of("/") + 1);
            dd = dateout.substr(0, dateout.find_first_of("/"));
            dateout.erase(0, dateout.find_first_of("/") + 1);
            yyyy = dateout;
        }
        else
        {
            yyyy = straux;
            dateout.erase(0, dateout.find_first_of("/") + 1);
            mm = dateout.substr(0, dateout.find_first_of("/"));
            dateout.erase(0, dateout.find_first_of("/") + 1);
            dd = dateout;
        }

        if(yyyy.size() != 4 || mm.size() != 2 && mm.size() != 1 || dd.size() != 2 && dd.size() != 1)
            return 59;

        yr = strtol(yyyy.c_str(), NULL, 10);
        m = strtol(mm.c_str(), NULL, 10);
        d = strtol(dd.c_str(), NULL, 10);
        int daysPerMonth[] = {0, 31, (isLeap(yr) ? 29 : 28), 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

        for (int i = 0; i < m; i++)
            yrdoy += daysPerMonth[i];
        yrdoy += d;
        yr *= 1000;
        yrdoy += yr;

        dateout = std::to_string(Y2K_DOY(yrdoy));

        yr = strtol(yyyy.c_str(), NULL, 10);
        m = strtol(dd.c_str(), NULL, 10);
        d = strtol(mm.c_str(), NULL, 10);

        yrdoy = 0;
        for (int i = 0; i < m; i++)
            yrdoy += daysPerMonth[i];
        yrdoy += d;
        yr *= 1000;
        yrdoy += yr;

        dateout2 = std::to_string(Y2K_DOY(yrdoy));

        int flagDateEqual = 0;
        for(int i=0; i < dtidentfied.size(); i++)
        {
            if(dateout == dtidentfied[i])
            {
                flagDateEqual += 1;
            }
            if(dateout2 == dtidentfied[i])
            {
                flagDateEqual += 2;
            }
        }

        if(dateReadFormat == 0 && flagDateEqual == 0)
        {
            date = dateout;
            dtidentfied.push_back(date);
            return 0;
        }
        else if(dateReadFormat == 1 && flagDateEqual == 0)
        {
            date = dateout2;
            dtidentfied.push_back(date);
            return 0;
        }
        else if(flagDateEqual == 1)
        {
            date = dateout2;
            return 0;
        }
        else if(flagDateEqual == 2)
        {
            date = dateout;
            return 0;
        }
        else if(flagDateEqual == 3)
        {
            return 59;
        }

    }

}
