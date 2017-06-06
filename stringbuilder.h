// stringbuilder to build strings using the << operator
//
#pragma once
#include <sstream>
#include <ostream>
#include "devio_strings.h"
#include <QString>

using std::string;

struct stringbuilder
{
   std::ostringstream ss;

   template<typename T>
   stringbuilder & operator << (const T &data)
   {
        ss << data;
        return *this;
   }

   operator std::string() { return ss.str(); }
   operator QString() { return QString::fromUtf8(ss.str().c_str()); }
};

string describe_err( bool bresult);
string describe_err();
