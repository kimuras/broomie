/*
 * Copyright (C) Shunya KIMURA <brmtrain@gmail.com>
 * Use and distribution of this program is licensed under the
 * BSD license. See the COPYING file for full text.
 *
 * Original version TinySegmenter was written by Taku Kudo <taku@chasen.org>
 * The license is below.
 *   TinySegmenter 0.1 -- Super compact Japanese tokenizer in Javascript
 *   (c) 2008 Taku Kudo <taku@chasen.org>
 *   TinySegmenter is freely distributable under the terms of a new BSD licence.
 *   For details, see http://chasen.org/~taku/software/TinySegmenter/LICENCE.txt
 *   http://www.chasen.org/~taku/software/TinySegmenter/
 *
 */

#ifndef TINYSEGMENTERXX_TINYSEGMENTERXX_H
#define TINYSEGMENTERXX_TINYSEGMENTERXX_H

#include <vector>
#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdarg>
#include <cstdlib>
#include <stdint.h>
#include "tinysegmenterxx_train.hpp"

#ifdef HAVE_CONFIG_H
#include "../config.h"
#else
#define PACKAGE_STRING "TinySegmenterxx 0.0.1"
#endif

namespace tinysegmenterxx {

  /*! @brief The vector object for storing segment result. */
  typedef std::vector<std::string> Segmentes;

  const unsigned int INPUT_MAX_BUF_SIZ = 65536;
  const unsigned int GETSCORE_BUF_SIZ  = 65536;
  const unsigned int SEGMENT_STACK_SIZ = 6;
  const int DEFAULT_SCORE              = -332;
  const char* UP1__ = "UP1__";
  const char* UP2__ = "UP2__";
  const char* UP3__ = "UP3__";
  const char* BP1__ = "BP1__";
  const char* BP2__ = "BP2__";
  const char* UW1__ = "UW1__";
  const char* UW2__ = "UW2__";
  const char* UW3__ = "UW3__";
  const char* UW4__ = "UW4__";
  const char* UW5__ = "UW5__";
  const char* UW6__ = "UW6__";
  const char* BW1__ = "BW1__";
  const char* BW2__ = "BW2__";
  const char* BW3__ = "BW3__";
  const char* TW1__ = "TW1__";
  const char* TW2__ = "TW2__";
  const char* TW3__ = "TW3__";
  const char* TW4__ = "TW4__";
  const char* UC1__ = "UC1__";
  const char* UC2__ = "UC2__";
  const char* UC3__ = "UC3__";
  const char* UC4__ = "UC4__";
  const char* UC5__ = "UC5__";
  const char* UC6__ = "UC6__";
  const char* BC1__ = "BC1__";
  const char* BC2__ = "BC2__";
  const char* BC3__ = "BC3__";
  const char* TC1__ = "TC1__";
  const char* TC2__ = "TC2__";
  const char* TC3__ = "TC3__";
  const char* TC4__ = "TC4__";
  const char* TC5__ = "TC5__";
  const char* UQ1__ = "UQ1__";
  const char* UQ2__ = "UQ2__";
  const char* UQ3__ = "UQ3__";
  const char* BQ1__ = "BQ1__";
  const char* BQ2__ = "BQ2__";
  const char* BQ3__ = "BQ3__";
  const char* BQ4__ = "BQ4__";
  const char* TQ1__ = "TQ1__";
  const char* TQ2__ = "TQ2__";
  const char* TQ3__ = "TQ3__";
  const char* TQ4__ = "TQ4__";
  const char* B__   = "B";
  const char* B3__  = "B3";
  const char* B2__  = "B2";
  const char* B1__  = "B1";
  const char* E3__  = "E3";
  const char* E2__  = "E2";
  const char* E1__  = "E1";
  const char* O__   = "O";
  const char* U__   = "U";
  const char* A__   = "A";
  const char* N__   = "N";
  const char* I__   = "I";
  const char* K__   = "K";
  const char* M__   = "M";
  const char* H__   = "H";

  namespace util {


    /*!
      @brief This method is used in order to get charcter type.
      @param c : the charcter code of unicode.
      @return Return any of the following charcter
      - ASCII           : "A"
      - Number          : "N"
      - Kana            : "I"
      - Katakana        : "K"
      - Number in Kanji : "M"
      - Kanji           : "H"
      - Other           : "O"
     */
    const char* getCharClass(uint16_t c)
    {
      const char* rv = O__;
      if (c <= 0x007F ) {
        // ASCII
        if(c >= 'a' && c <= 'z'){
          rv = A__;
        } else if(c >= 'A' && c <= 'Z'){
          rv = A__;
        } else if(c >= '0' && c <= '9'){
          rv = N__;
        }
      } else if (c >= 0x3040 && c <= 0x309F) {
        // Kana
        rv = I__;
      } else if (c >= 0x30A0 && c <= 0x30FA) {
        // Katakana
        rv = K__;
      } else if (c >= 0x4E00 && c <= 0x9FFF) {
        // Kanji
        if(c == 0x4E00 || c == 0x4E8C || c == 0x4E09 || c == 0x56DB ||
           c == 0x4E94 || c == 0x516D || c == 0x4E03 || c == 0x516B ||
           c == 0x4E5D || c == 0x5341 || c == 0x767E || c == 0x5343 ||
           c == 0x4E07 || c == 0x5104 || c == 0x5146){
          rv = M__;
        }
        rv = H__;
      } else {
        rv = O__;
      }
      return rv;
    }

    /*!
      @brief This method is used in encode utf charcters to unicode charcters.
      @param str : The pointer to the utf charcters.
      @param ary : The region for stroing the unicode charcters.
      @param np  : The pointer for storing num of charcters.
     */
    void utftoucs(const char *str, uint16_t *ary, int *np)
    {
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

    /*!
      @brief This method is used in encode unicode charcters to utf charcters.
      @param ary  : The pointer to the unicode charcters.
      @param num  : Number of charcters in ary.
      @param str  : The region for storing utf charcters.
     */
    int ucstoutf(const uint16_t *ary, int num, char *str)
    {
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

    void getVersion(std::string& body)
    {
      body.append(PACKAGE_STRING);
      body.append("\n");
      body.append("Copyright(C) Shunya Kimura");
    }
  }

  /*!
    @brief Class for getting segmentes from Japanese sentense.
   */
  class Segmenter {

  public:
    /*!
      @brief Constructor of Segmentr objetc.
     */
    Segmenter() : train(){}

    /*!
      @brief Destructor of Segmentr objetc.
     */
    ~Segmenter(){}


    /*!
      @brief This method is extract segmentes from Japanese sentense.
      @param input : String object of input sentense.
      @param result : Segmentes object, the results of segment will be stored this object.
      @attention Segmentes object is typedef object of vector<string>
     */
    void segment(std::string& input, Segmentes& result)
    {
      char seg[SEGMENT_STACK_SIZ][4];
      char ctype[SEGMENT_STACK_SIZ][4];

      std::strcpy(seg[0], B2__);
      std::strcpy(seg[1], B1__);
      std::strcpy(ctype[0], O__);
      std::strcpy(ctype[1], O__);

      unsigned int inputSiz = input.size();

      int stackSiz = inputSiz + 1;
      uint16_t* ary = NULL;
      if(inputSiz + 1 >= INPUT_MAX_BUF_SIZ){
        stackSiz = 0;
        ary = new uint16_t[inputSiz];
      }
      uint16_t stack[stackSiz + 1];
      if(!ary) ary = stack;
      int anum;
      util::utftoucs(input.c_str(), ary, &anum);
      if(anum < 1) return;
      if(anum == 1){
        result.push_back(input);
        return;
      }
      ary[anum] = 0x0000;
      if(anum < 4){
        for(int i = 0; i < anum; ++i){
          uint16_t ucsChar = ary[i];
          const uint16_t* ucsCharPtr = &ucsChar;
          std::strcpy(ctype[i + 2], util::getCharClass(ucsChar));
          util::ucstoutf(ucsCharPtr, 1, seg[i + 2]);
        }
        if(anum == 2){
          std::strcpy(ctype[4], O__);
          std::strcpy(ctype[5], O__);
          std::strcpy(seg[4], E1__);
          std::strcpy(seg[5], E2__);
        } else if(anum == 3){
          std::strcpy(seg[5], E1__);
          std::strcpy(ctype[5], O__);
        }
      } else {
        for(int i = 0; i < 4; ++i){
          uint16_t ucsChar = ary[i];
          const uint16_t* ucsCharPtr = &ucsChar;
          std::strcpy(ctype[i + 2], util::getCharClass(ucsChar));
          util::ucstoutf(ucsCharPtr, 1, seg[i + 2]);
        }
      }
      std::string word = seg[2];
      const char* p1 = U__;
      const char* p2 = U__;
      const char* p3 = U__;
      for(int i = 4; i < anum + 3 ; ++i){
        int score = getScore(seg, ctype, p1, p2, p3);
        const char* p = O__;
        if(score > 0){
          result.push_back(word);
          word = "";
          p    = B__;
        }
        p1 = p2;
        p2 = p3;
        p3 = p;
        word.append(seg[3]);
        std::memmove(seg, seg + 1, sizeof(seg) - sizeof(seg[0]));
        std::memmove(ctype, ctype + 1, sizeof(ctype) - sizeof(ctype[0]));
        if(i < anum){
          uint16_t ucsChar = ary[i];
          const uint16_t* ucsCharPtr = &ucsChar;
          std::strcpy(ctype[5], util::getCharClass(ucsChar));
          util::ucstoutf(ucsCharPtr, 1, seg[5]);
        } else {
          std::strcpy(ctype[5], O__);
          if(i >= anum + 1){
            std::strcpy(seg[5], E2__);
          } else if(i >= anum ){
            std::strcpy(seg[5], E1__);
          } else {
            std::strcpy(seg[5], E3__);
          }
        }
      }
      if(word.size() > 0)
        result.push_back(word);

      if(ary != stack) delete[] ary;
    }

  private:

    /*! @brief Trainig data object.Perfect hashing. */
    TrainHash train;

    /*!
      @brief This method is used in order to get score of the each stack elementes.
      @param seg[][4] : The pointer for the word stack.
      @param ctype[][4] : The pointer for the char type stack.
      @return The score.
    */
    int getScore(char seg[][4], char ctype[][4],
                 const char* p1, const char* p2, const char* p3)
    {
      int score = DEFAULT_SCORE;
      const char* w1 = seg[0];
      const char* w2 = seg[1];
      const char* w3 = seg[2];
      const char* w4 = seg[3];
      const char* w5 = seg[4];
      const char* w6 = seg[5];
      const char* c1 = ctype[0];
      const char* c2 = ctype[1];
      const char* c3 = ctype[2];
      const char* c4 = ctype[3];
      const char* c5 = ctype[4];
      const char* c6 = ctype[5];
      score += getScoreImpl(2, UP1__, p1);
      score += getScoreImpl(2, UP2__, p2);
      score += getScoreImpl(2, UP3__, p3);
      score += getScoreImpl(3, BP1__, p1, p2);
      score += getScoreImpl(3, BP2__, p2, p3);
      score += getScoreImpl(2, UW1__, w1);
      score += getScoreImpl(2, UW2__, w2);
      score += getScoreImpl(2, UW3__, w3);
      score += getScoreImpl(2, UW4__, w4);
      score += getScoreImpl(2, UW5__, w5);
      score += getScoreImpl(2, UW6__, w6);
      score += getScoreImpl(3, BW1__, w2, w3);
      score += getScoreImpl(3, BW2__, w3, w4);
      score += getScoreImpl(3, BW3__, w4, w5);
      score += getScoreImpl(4, TW1__, w1, w2, w3);
      score += getScoreImpl(4, TW2__, w2, w3, w4);
      score += getScoreImpl(4, TW3__, w3, w4, w5);
      score += getScoreImpl(4, TW4__, w4, w5, w6);
      score += getScoreImpl(2, UC1__, c1);
      score += getScoreImpl(2, UC2__, c2);
      score += getScoreImpl(2, UC3__, c3);
      score += getScoreImpl(2, UC4__, c4);
      score += getScoreImpl(2, UC5__, c5);
      score += getScoreImpl(2, UC6__, c6);
      score += getScoreImpl(3, BC1__, c2, c3);
      score += getScoreImpl(3, BC2__, c3, c4);
      score += getScoreImpl(3, BC3__, c4, c5);
      score += getScoreImpl(4, TC1__, c1, c2, c3);
      score += getScoreImpl(4, TC2__, c2, c3, c4);
      score += getScoreImpl(4, TC3__, c3, c4, c5);
      score += getScoreImpl(4, TC4__, c4, c5, c6);
      score += getScoreImpl(3, UQ1__, p1, c1);
      score += getScoreImpl(3, UQ2__, p2, c2);
      score += getScoreImpl(3, UQ3__, p3, c3);
      score += getScoreImpl(4, BQ1__, p2, c2, c3);
      score += getScoreImpl(4, BQ2__, p2, c3, c4);
      score += getScoreImpl(4, BQ3__, p3, c2, c3);
      score += getScoreImpl(4, BQ4__, p3, c3, c4);
      score += getScoreImpl(5, TQ1__, p2, c1, c2, c3);
      score += getScoreImpl(5, TQ2__, p2, c2, c3, c4);
      score += getScoreImpl(5, TQ3__, p3, c1, c2, c3);
      score += getScoreImpl(5, TQ4__, p3, c2, c3, c4);
#ifdef DEBUG
      std::cout << "score:" << score << std::endl;
      std::cout << w1 << "|\t|" << w2 << "|\t|" << w3 << "|\t|" << w4
                << "|\t|" << w5 << "|\t|" << w6 << std::endl;
      std::cout << c1 << "|\t|" << c2 << "|\t|" << c3 << "|\t|" << c4
                << "|\t|" << c5 << "|\t|" << c6 << std::endl;
      std::cout << p1 << "|\t|" << p2 << "|\t|" << p3 << std::endl;
      std::cout << std::endl;
#endif
      return score;
    }

#ifdef ENABLE_THREAD
    /*!
      @brief This method is used in order to get score from training data.
      @param num : Number of argument.
      @param ... : The key name for getting the score from training data.
      @return : Return a score if exist, return 0 if not exist.
     */
    int getScoreImpl(void* arg)
    {
      const struct Train* rv =
        train.in_word_set(arg->key.c_str());

#ifdef DEBUG
      if(rv) std::cout << stack << "\t" << rv->val << std::endl;
#endif

      if(rv) return rv->val;

      return 0;
    }

#else
    /*!
      @brief This method is used in order to get score from training data.
      @param num : Number of argument.
      @param ... : The key name for getting the score from training data.
      @return : Return a score if exist, return 0 if not exist.
     */
    int getScoreImpl(int num, ...)
    {
      char stack[GETSCORE_BUF_SIZ];
      char *wp = stack;
      std::va_list ap;
      va_start(ap, num);
      unsigned int cnt = 0;
      for(int i = 0; i < num; i++){
        const char* str = va_arg(ap, const char*);
        while(*str != '\0'){
          *(wp++) = *(str++);
          cnt++;
          if(cnt >= GETSCORE_BUF_SIZ - 1) break;
        }
      }
      va_end(ap);
      *wp = '\0';

      const struct Train* rv =
        train.in_word_set(stack, wp - stack);

#ifdef DEBUG
      if(rv) std::cout << stack << "\t" << rv->val << std::endl;
#endif

      if(rv) return rv->val;

      return 0;
    }
#endif
  };
}

#endif /* TINYSEGMENTERXX_TINYSEGMENTERXX_H */
