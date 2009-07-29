#include <vector>
#include <iostream>
#include <fstream>
#include <cstring>
#include "tinysegmenter_train.hpp"


namespace segmenter {

  typedef std::vector<std::string> Segmenters;

  const unsigned int BUF_SIZ = 2048;
  const std::string UP1 = "UP1__";
  const std::string UP2 = "UP2__";
  const std::string UP3 = "UP3__";
  const std::string BP1 = "BP1__";
  const std::string BP2 = "BP2__";
  const std::string UW1 = "UW1__";
  const std::string UW2 = "UW2__";
  const std::string UW3 = "UW3__";
  const std::string UW4 = "UW4__";
  const std::string UW5 = "UW5__";
  const std::string UW6 = "UW6__";
  const std::string BW1 = "BW1__";
  const std::string BW2 = "BW2__";
  const std::string BW3 = "BW3__";
  const std::string TW1 = "TW1__";
  const std::string TW2 = "TW2__";
  const std::string TW3 = "TW3__";
  const std::string TW4 = "TW4__";
  const std::string UC1 = "UC1__";
  const std::string UC2 = "UC2__";
  const std::string UC3 = "UC3__";
  const std::string UC4 = "UC4__";
  const std::string UC5 = "UC5__";
  const std::string UC6 = "UC6__";
  const std::string BC1 = "BC1__";
  const std::string BC2 = "BC2__";
  const std::string BC3 = "BC3__";
  const std::string TC1 = "TC1__";
  const std::string TC2 = "TC2__";
  const std::string TC3 = "TC3__";
  const std::string TC4 = "TC4__";
  const std::string TC5 = "TC5__";
  const std::string UQ1 = "UQ1__";
  const std::string UQ2 = "UQ2__";
  const std::string UQ3 = "UQ3__";
  const std::string BQ1 = "BQ1__";
  const std::string BQ2 = "BQ2__";
  const std::string BQ3 = "BQ3__";
  const std::string BQ4 = "BQ4__";
  const std::string TQ1 = "TQ1__";
  const std::string TQ2 = "TQ2__";
  const std::string TQ3 = "TQ3__";
  const std::string TQ4 = "TQ4__";

  class Segmenter {

  public:
    Segmenter() : train(){}

    ~Segmenter(){}

    Segmenters* segment(std::string& input)
    {
      Segmenters* result;
      result = new Segmenters;
      std::vector<std::string> seg;
      seg.push_back("B3");
      seg.push_back("B2");
      seg.push_back("B1");
      std::vector<std::string> ctype;
      ctype.push_back("O");
      ctype.push_back("O");
      ctype.push_back("O");

      int bufSiz = BUF_SIZ;
      //int bufSiz = 10000;
      if(std::strlen(input.c_str()) > BUF_SIZ)
        bufSiz = std::strlen(input.c_str());
      uint16_t stack[bufSiz];
      uint16_t *ary = stack;
      int anum;
      utftoucs(input.c_str(), ary, &anum);
      ary[anum] = 0x0000;

      for(int i = 0; i < anum; ++i){
        char c[4];
        uint16_t stack2[1];
        stack2[0] = ary[i];
        const uint16_t* ary2 = stack2;
        std::string cTypeBuf = getCharClass(stack2[0]);
        ucstoutf(ary2, 1, c);
        //std::cout << c << "\t" << cTypeBuf <<  std::endl;
        seg.push_back(c);
        ctype.push_back(cTypeBuf);
      }
      seg.push_back("E1");
      seg.push_back("E2");
      seg.push_back("E3");
      ctype.push_back("O");
      ctype.push_back("O");
      ctype.push_back("O");
      std::string word = seg.at(3);
      std::string p1 = "U";
      std::string p2 = "U";
      std::string p3 = "U";
      for(unsigned int i = 4; i < seg.size() - 3; i++){
        int score = -312;
        std::string w1 = seg[i-3];
        std::string w2 = seg[i-2];
        std::string w3 = seg[i-1];
        std::string w4 = seg[i];
        std::string w5 = seg[i+1];
        std::string w6 = seg[i+2];
        std::string c1 = ctype[i-3];
        std::string c2 = ctype[i-2];
        std::string c3 = ctype[i-1];
        std::string c4 = ctype[i];
        std::string c5 = ctype[i+1];
        std::string c6 = ctype[i+2];
        //std::cout << w1 << "\t" << w2 << "\t" << w3 << "\t" << w4 << "\t" << w5 << "\t" << w6 << std::endl;
        //std::cout << c1 << "\t" << c2 << "\t" << c3 << "\t" << c4 << "\t" << c5 << "\t" << c6 << std::endl;

        score += getScore(UP1 + p1);
        score += getScore(UP2 + p2);
        score += getScore(UP3 + p3);
        score += getScore(BP1 + p1 + p2);
        score += getScore(BP2 + p2 + p3);
        score += getScore(UW1 + w1);
        score += getScore(UW2 + w2);
        score += getScore(UW3 + w3);
        score += getScore(UW4 + w4);
        score += getScore(UW5 + w5);
        score += getScore(UW6 + w6);
        score += getScore(BW1 + w2 + w3);
        score += getScore(BW2 + w3 + w4);
        score += getScore(BW3 + w4 + w5);
        //        std::cout << w1 + w2 + w3 << std::endl;
        score += getScore(TW1 + w1 + w2 + w3);
        score += getScore(TW2 + w2 + w3 + w4);
        score += getScore(TW3 + w3 + w4 + w5);
        score += getScore(TW4 + w4 + w5 + w6);
        score += getScore(UC1 + c1);
        score += getScore(UC2 + c2);
        score += getScore(UC3 + c3);
        score += getScore(UC4 + c4);
        score += getScore(UC5 + c5);
        score += getScore(UC6 + c6);
        score += getScore(BC1 + c2 + c3);
        score += getScore(BC2 + c3 + c4);
        score += getScore(BC3 + c4 + c5);
        score += getScore(TC1 + c1 + c2 + c3);
        score += getScore(TC2 + c2 + c3 + c4);
        score += getScore(TC3 + c3 + c4 + c5);
        score += getScore(TC4 + c4 + c5 + c6);
        score += getScore(UQ1 + p1 + c1);
        score += getScore(UQ2 + p2 + c2);
        //score += getScore(UQ1, p1 + c1); // ayashii
        score += getScore(UQ3 + p3 + c3);
        score += getScore(BQ1 + p2 + c2 + c3);
        score += getScore(BQ2 + p2 + c3 + c4);
        score += getScore(BQ3 + p3 + c2 + c3);
        score += getScore(BQ4 + p3 + c3 + c4);
        score += getScore(TQ1 + p2 + c1 + c2 + c3);
        score += getScore(TQ2 + p2 + c2 + c3 + c4);
        score += getScore(TQ3 + p3 + c1 + c2 + c3);
        score += getScore(TQ4 + p3 + c2 + c3 + c4);
        std::string p = "O";
        if(score > 0){
          result->push_back(word);
          word = "";
          p = "B";
        }
        p1 = p2;
        p2 = p3;
        p3 = p;
        word.append(seg[i]);
      }
      result->push_back(word);

      return result;
    }

    /*
     * Convert a UTF-8 string into a UCS-2 array.
     * copied by tcutil(tokyocabinet)
     */
    void utftoucs(const char *str, uint16_t *ary, int *np){
      const unsigned char *rp = (unsigned char *)str;
      unsigned int wi = 0;
      while(*rp != '\0'){
        int c = *(unsigned char *)rp;
        if(c < 0x80){
          ary[wi++] = c;
        } else if(c < 0xe0){
          if(rp[1] >= 0x80){
            ary[wi++] = ((rp[0] & 0x1f) << 6) | (rp[1] & 0x3f);
            rp++;
          }
        } else if(c < 0xf0){
          if(rp[1] >= 0x80 && rp[2] >= 0x80){
            ary[wi++] = ((rp[0] & 0xf) << 12) | ((rp[1] & 0x3f) << 6) | (rp[2] & 0x3f);
            rp += 2;
          }
        }
        rp++;
      }
      *np = wi;
    }


    /*
     * Convert a UCS-2 array into a UTF-8 string.
     * copied by tcutil(tokyocabinet)
     */
    int ucstoutf(const uint16_t *ary, int num, char *str){
      unsigned char *wp = (unsigned char *)str;
      for(int i = 0; i < num; i++){
        unsigned int c = ary[i];
        if(c < 0x80){
          *(wp++) = c;
        } else if(c < 0x800){
          *(wp++) = 0xc0 | (c >> 6);
          *(wp++) = 0x80 | (c & 0x3f);
        } else {
          *(wp++) = 0xe0 | (c >> 12);
          *(wp++) = 0x80 | ((c & 0xfff) >> 6);
          *(wp++) = 0x80 | (c & 0x3f);
        }
      }
      *wp = '\0';
      return (char *)wp - str;
    }

    std::string getCharClass(uint16_t c)
    {
      if (c <= 0x007F ) {
        // ASCII
        if(c >= 'a' && c <= 'z'){
          return "A";
        } else if(c >= 'A' && c <= 'Z'){
          return "A";
        } else if(c >= '0' && c <= '9'){
          return "N";
        }
      } else if (c >= 0x3040 && c <= 0x309F) {
        // Kana
        return "I";
      } else if (c >= 0x30A0 && c <= 0x30FF) {
        // Katakana
        return "K";
      } else if (c >= 0x4E00 && c <= 0x9FFF) {
        // Kanji
        if(c == 0x4E00 || c == 0x4E8C || c == 0x4E09 || c == 0x56DB ||
           c == 0x4E94 || c == 0x516D || c == 0x4E03 || c == 0x516B ||
           c == 0x4E5D || c == 0x5341 || c == 0x767E || c == 0x5343 ||
           c == 0x4E07 || c == 0x5104 || c == 0x5146){
          return "M";
        }
        return "H";
      } else {
        return "O";
      }
      return "O";
    }

  private:
    TrainHash train;

    int getScore(const std::string& key)
    {
      const struct Train* rv =
        train.in_word_set(key.c_str(), std::strlen(key.c_str()));
      if(rv != NULL){
        return rv->val;
      } else {
        return 0;
      }
      return 0;
    }

    
  };
}
