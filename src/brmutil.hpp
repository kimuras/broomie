/*
 * Copyright (C) 2009 Shunya Kimura <brmtrain@gmail.com>
 * All Rights Reserved.
 *
 * Use and distribution of this program is licensed under the
 * BSD license. See the COPYING file for full text.
 */


#include <ctime>
#include <iterator>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h> // may be not need
#include <stdio.h>
#include "libbrm.hpp"

#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif

const unsigned int MAX_WORD_BUF_SIZ = 2048;
const unsigned int MIN_HIRAGANA_SIZ = 6;

namespace {
  class Initilizer {
  public:
    Initilizer()
    {
      std::srand(std::time(NULL));
    };
    ~Initilizer(){};
  };
  Initilizer init;
}

namespace broomie {

  namespace util {

    void copyRight(std::string& body)
    {
      body.append(PACKAGE_STRING);
      body.append("\n");
      body.append("Copyright(C) 2009 - Shunya Kimura");
    }

    std::vector<std::string> split(std::string str, std::string delim)
    {
      std::vector<std::string> result;
      uint64_t cutAt;
      while((cutAt = str.find_first_of(delim)) != str.npos){
        if(cutAt > 0) result.push_back(str.substr(0, cutAt));
        str = str.substr(cutAt + 1);
      }
      if(str.length() > 0) result.push_back(str);
      return result;
    }

    template <class T> T getMax(T& a, T& b)
    {
      return (a > b) ? a : b;
    }

    bool checkDir(std::string path)
    {
      struct stat sb;
      if(stat(path.c_str(), &sb) == -1){
        return false;
      }
      if(sb.st_mode & S_IFDIR){
        return true;
      } else {
        return false;
      }
    }

    bool checkFile(std::string path)
    {
      struct stat sb;
      if(stat(path.c_str(), &sb) == -1){
        return false;
      }
      if(sb.st_mode & S_IFREG){
        return true;
      } else {
        return false;
      }
    }
  }

  namespace testutil {
    std::string createRandomString(int cnum = 5) {
      std::string rv;
      int wd;
      for(int i = 0; i < cnum; i++){
        wd = std::rand() % 26 + 97;
        char wdbuf = static_cast<char>(wd);
        rv = rv + wdbuf;
      }
      return rv;
    }

    double createRandomDouble() {
      double rv = static_cast<double>(std::rand() % 1000);
      rv /= 100;
      return rv;
    }

    int createRandomInt(int max) {
      int rv = static_cast<int>(std::rand() % max);
      return rv;
    }

    template<class T> T createRandomVal(int max){
      T rv = static_cast<T>((std::rand() % max));
      return rv;
    }
  }
}
