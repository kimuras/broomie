/*
 * Copyright (C) 2009 Shunya Kimura <brmtrain@gmail.com>
 * All Rights Reserved.
 *
 * Use and distribution of this program is licensed under the
 * BSD license. See the COPYING file for full text.
 */


#include "brmNB.hpp"

namespace broomie {

  namespace NB {

    NaiveBayes::NaiveBayes() : numPosTrain(0), numNegTrain(0), trainingCount(), sumPosVal(0.0), sumNegVal(0.0), uniqVal(0) {
      trainDB = tchdbnew();
    };

    NaiveBayes::~NaiveBayes(){
      tchdbclose(trainDB);
      tchdbdel(trainDB);
    };

    bool NaiveBayes::putTrain(const FeatureVal& fv, int mode)
    {
      bool ok = true;
      if(mode & broomie::NB::POSITIVE){
        for(unsigned int i = 0; i < fv.size(); i++){
          int key = fv[i].first;
          trainingCount[key].first += fv[i].second;
          trainingCount[key].second += 0.0;
          sumPosVal += fv[i].second;
        }
        numPosTrain++;
      } else if(mode & broomie::NB::NEGATIVE) {
        for(unsigned int i = 0; i < fv.size(); i++){
          int key = fv[i].first;
          trainingCount[key].first += 0.0;
          trainingCount[key].second += fv[i].second;
          sumNegVal += fv[i].second;
        }
        numNegTrain++;
      } else {
        return false;
      }
      return ok;
    };

    bool NaiveBayes::saveTrain(std::string& filePath)
    {
      bool ok = true;
      load(filePath, broomie::NB::TRUNC);
      TrainingData::iterator itr = trainingCount.begin();
      while(itr != trainingCount.end()){
        int key = itr->first;
        double pVal = itr->second.first;
        double nVal = itr->second.second;
        int valSiz = sizeof(pVal);
        char saveBuf[valSiz* 2];
        std::memcpy(saveBuf, &pVal, valSiz);
        std::memcpy(saveBuf + valSiz, &nVal, valSiz);
        char* saveKey = tcsprintf("%d", key);
        if(!tchdbput(trainDB, saveKey, std::strlen(saveKey),
                     saveBuf, sizeof(saveBuf))){
          ok = false;
        }
        std::free(saveKey);
        itr++;
      }
      int nTrainSiz = sizeof(numPosTrain);
      char docNumBuf[nTrainSiz * 2];
      std::memcpy(docNumBuf, &numPosTrain, nTrainSiz);
      std::memcpy(docNumBuf + nTrainSiz, &numNegTrain, nTrainSiz);
      const char* docNumKey = DEFINE_DOC_NUM_KEY.c_str();
      if(!tchdbput(trainDB, docNumKey, std::strlen(docNumKey),
                   docNumBuf, sizeof(docNumBuf))){
        ok = false;
      }

      int sValSiz = sizeof(sumPosVal);
      char sumValBuf[sValSiz * 2];
      std::memcpy(sumValBuf, &sumPosVal, sValSiz);
      std::memcpy(sumValBuf + sValSiz, &sumNegVal, sValSiz);
      const char* sumValKey = DEFINE_SUM_VAL_KEY.c_str();
      if(!tchdbput(trainDB, sumValKey, std::strlen(sumValKey),
                   sumValBuf, sizeof(sumValBuf))){
        ok = false;
      }
      return ok;
    };

    bool NaiveBayes::load(const std::string& filePath, int omode)
    {
      bool ok = true;
      if(omode & broomie::NB::TRUNC){
        if(!tchdbopen(this->trainDB, filePath.c_str(),
                      HDBOWRITER | HDBOCREAT | HDBOTRUNC)){
          return false;
        }
      } else if(omode & broomie::NB::READ){
        if(!tchdbopen(this->trainDB, filePath.c_str(), HDBOREADER)){
          return false;
        }
      }

      const char* docNumKey = DEFINE_DOC_NUM_KEY.c_str();
      int vsiz;
      void* docNumBuf;
      if(!(docNumBuf = tchdbget(trainDB, docNumKey,
                                std::strlen(docNumKey), &vsiz))){
        return false;
      } else {
        int trainBufSize = sizeof(numPosTrain);
        std::memcpy(&numPosTrain, docNumBuf, trainBufSize);
        std::memcpy(&numNegTrain, static_cast<const char*>(docNumBuf)
                    + trainBufSize, trainBufSize);
        std::free(docNumBuf);
      }

      const char* sumValKey = DEFINE_SUM_VAL_KEY.c_str();
      void* sumValBuf;
      if(!(sumValBuf = tchdbget(trainDB, sumValKey,
                                std::strlen(sumValKey), &vsiz))){
        return false;
      } else {
        int sumValSiz = sizeof(sumPosVal);
        std::memcpy(&sumPosVal, sumValBuf, sumValSiz);
        std::memcpy(&sumNegVal, static_cast<const char*>(sumValBuf) + sumValSiz, sumValSiz);
        std::free(sumValBuf);
      }
      uniqVal = tchdbrnum(trainDB) - 2;

      return ok;
    };


    double NaiveBayes::classify(const FeatureVal& fv)
    {
      double positiveResult = 0.0;
      double negativeResult = 0.0;
      double positiveRatio =
        static_cast<double>(numPosTrain) / (numPosTrain + numNegTrain);
      double negativeRatio =
        static_cast<double>(numNegTrain) / (numPosTrain + numNegTrain);
      for(unsigned int i = 0; i < fv.size(); i++){
        int keyBuf = fv[i].first;
        double fVal = fv[i].second;
        if(keyBuf == 0) continue;
        int vsiz;
        char* key = tcsprintf("%d", keyBuf);
        void* trainBuf = tchdbget(trainDB, key, std::strlen(key), &vsiz);
        if(!trainBuf){
          std::free(key);
          continue;
        }
        double positiveVal, negativeVal;
        int valSiz = sizeof(positiveVal);
        std::memcpy(&positiveVal, trainBuf, valSiz);
        std::memcpy(&negativeVal,
               static_cast<const char*>(trainBuf) + valSiz, valSiz);
        smoothing(positiveVal, sumPosVal);
        smoothing(negativeVal, sumNegVal);
        positiveVal = std::log(positiveVal);
        negativeVal = std::log(negativeVal);
        positiveVal *= (fVal);
        negativeVal *= (fVal);
        positiveResult += positiveVal;
        negativeResult += negativeVal;
        std::free(key);
        std::free(trainBuf);
      }
      positiveResult += std::log(positiveRatio);
      negativeResult += std::log(negativeRatio);
      double resultVal = std::fabs(positiveResult - negativeResult);
      if(positiveResult < negativeResult) resultVal *= -1;
      return resultVal;
    };

    unsigned int NaiveBayes::getNumPosTrain()
    {
      return numPosTrain;
    }

    unsigned int NaiveBayes::getNumNegTrain()
    {
      return numNegTrain;
    }

    double NaiveBayes::getSumPosVal()
    {
      return sumPosVal;
    }

    double NaiveBayes::getSumNegVal()
    {
      return sumNegVal;
    }

    int NaiveBayes::getUniqVal()
    {
      return uniqVal;
    }

    void NaiveBayes::smoothing(double& val, double sumFeatureVal)
    {
      val = (val + broomie::NB::SMOOTHING_W) /
        (sumFeatureVal + (broomie::NB::SMOOTHING_W * uniqVal));
    }
  }
}
