#ifndef INPUTWTH_H
#define INPUTWTH_H

struct HeaderDataWeather {
  bool status;
  std::string varname;
  std::string valuestr;
  int size;
  int valueint;
  int line;
};

int YEARDOY(int YRDOY);
int YEAR2KDOY(int YRDOY);
bool ISLEAPYEAR(int year);
bool FORMATDATE(std::string &date, int &yrdoy, int &readFormat, int &errorcode);

void INPUTWEATHER(char *FILEWW, int *YRDOY, int *FirstWeatherDay, int *LastWeatherDay, int *ENDFILE, int *LNUM, int *NRECORDS, char *ERRYRDOY, int *ERRCODE);

#endif