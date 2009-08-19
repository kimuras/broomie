#include <ctime>
#include <iterator>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h> // may be not need
#include <stdio.h>
#include "tinysegmenterxx.hpp"
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

    bool selectWord(std::string& word)
    {
      bool rv = false;
      if(tcregexmatch(word.c_str(), "^[a-zA-Z]*$")) return true;
      unsigned int inputSiz = word.size();
      int stackSiz = inputSiz + 1;
      uint16_t* ary = NULL;
      if(inputSiz + 1 >= MAX_WORD_BUF_SIZ){
        stackSiz = 0;
        ary = new uint16_t[inputSiz];
      }
      uint16_t stack[stackSiz + 1];
      if(!ary) ary = stack;
      int anum;
      tinysegmenterxx::util::utftoucs(word.c_str(), ary, &anum);
      ary[anum] = 0x0000;
      std::map<std::string, int> charClassMap;
      for(int i = 0; i < anum; ++i){
        uint16_t ucsChar = ary[i];
        std::string cTypeBuf =
          tinysegmenterxx::util::getCharClass(ucsChar);
        charClassMap[cTypeBuf] += 1;
      }
      if(charClassMap.size() == 1){
        std::map<std::string, int>::iterator itr = charClassMap.begin();
        if(itr->first == "H" || itr->first == "K"){
          rv = true;
        } else if(itr->first == "I"){
          if(word.size() > MIN_HIRAGANA_SIZ) rv = true;
        }
      }
      if(ary != stack) delete[] ary;
      return rv;
    }

    void convertbrmFormat(tinysegmenterxx::Segmenter& sg, std::string& line,
                          std::vector<std::string>& features)
    {
      std::vector<std::string> lineElem = broomie::util::split(line, "\t");
      if(lineElem.size() < 2) return;
      features.push_back(lineElem[0]);
      tinysegmenterxx::Segmentes segs;
      sg.segment(lineElem[1], segs);
      std::map<std::string, int> featureMap;
      for(unsigned int i = 0; i < segs.size(); i++){
          if(selectWord(segs[i])){
            featureMap[segs[i]] += 1;
          }
      }
      std::map<std::string, int>::iterator itr;
      for(itr = featureMap.begin(); itr != featureMap.end(); itr++){
        features.push_back(itr->first);
        char* valBuf = tcsprintf("%d", itr->second);
        features.push_back(valBuf);
        std::free(valBuf);
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
      int rv = static_cast<double>(std::rand() % max);
      return rv;
    }

    template<class T> T createRandomVal(int max){
      T rv = static_cast<T>((std::rand() % max));
      return rv;
    }
  }
}
