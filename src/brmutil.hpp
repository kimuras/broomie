#include <ctime>
#include <iterator>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <stdio.h>

#include "libtinysegmenter.hpp"
#include "libbrm.hpp"

#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif

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

    bool selectWord(segmenter::Segmenter& sg, std::string& word)
    {
      if(tcregexmatch(word.c_str(), "^[a-zA-Z]*$")) return true;
      uint16_t stack[256];
      uint16_t *ary = stack;
      int anum;
      sg.utftoucs(word.c_str(), ary, &anum);
      ary[anum] = 0x0000;
      std::map<std::string, int> charClassMap;
      for(int i = 0; i < anum; ++i){
        char c[4];
        uint16_t stack2[1];
        stack2[0] = ary[i];
        const uint16_t* ary2 = stack2;
        std::string cTypeBuf = sg.getCharClass(stack2[0]);
        sg.ucstoutf(ary2, 1, c);
        charClassMap[cTypeBuf] += 1;
      }
      if(charClassMap.size() == 1){
        std::map<std::string, int>::iterator itr = charClassMap.begin();
        if(itr->first == "H" || itr->first == "K"){
          return true;
        } else if(itr->first == "I"){
          if(word.size() > 6) return true;
        }
      }
      return false;
    }

    void convertbrmFormat(segmenter::Segmenter& sg, std::string& line, std::vector<std::string>& features)
    {
      std::vector<std::string> lineElem = broomie::util::split(line, "\t");
      if(lineElem.size() < 2) return;
      features.push_back(lineElem[0]);
      segmenter::Segmenters* rv = sg.segment(lineElem[1]);
      std::map<std::string, int> featureMap;
      for(unsigned int i = 0; i < rv->size(); i++){
        if(selectWord(sg, (*rv)[i])){
          featureMap[(*rv)[i]] += 1;
        }
      }
      std::map<std::string, int>::iterator itr;
      for(itr = featureMap.begin(); itr != featureMap.end(); itr++){
        features.push_back(itr->first);
        char* valBuf = tcsprintf("%d", itr->second);
        features.push_back(valBuf);
        std::free(valBuf);
      }
      delete rv;
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
