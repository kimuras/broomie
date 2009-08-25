/*
 * Copyright (C) 2009 Shunya Kimura <brmtrain@gmail.com>
 * All Rights Reserved.
 *
 * Use and distribution of this program is licensed under the
 * BSD license. See the COPYING file for full text.
 */


#include "brmalgorithm.hpp"

namespace broomie {

  /*****************************************************************************
   * NBModel object implement
   ****************************************************************************/
  broomie::NBModel::NBModel() : nb(), addTrainingDataNum(0){};

  broomie::NBModel::~NBModel(){};

  bool broomie::NBModel::put(const Feature features[], int num, int mode)
  {
    bool ok = true;
    broomie::NB::FeatureVal fv;
    for(int i = 0; i < num; i++){
      fv.push_back(std::make_pair(features[i].id, features[i].point));
    }
    if(mode == POSITIVE){
      if(!nb.putTrain(fv, broomie::NB::POSITIVE)) ok = false;
      this->addTrainingDataNum++;
    } else if(mode == NEGATIVE){
      if(!nb.putTrain(fv, broomie::NB::NEGATIVE)) ok = false;
      this->addTrainingDataNum++;
    } else {
      ok = false;
    }
    return ok;
  };


  bool broomie::NBModel::save()
  {
    if(!nb.saveTrain(path)){
      return false;
    } else {
      return true;
    }
  };


  bool broomie::NBModel::load()
  {
    if(!nb.load(path, broomie::NB::READ)){
      return false;
    } else {
      return true;
    }
  }


  double broomie::NBModel::classify(const Feature features[], int num)
  {
    broomie::NB::FeatureVal fv;
    for(int i = 0; i < num; i++){
      fv.push_back(std::make_pair(features[i].id, features[i].point));
    }
    double score = nb.classify(fv);
    return score;
  }


  void broomie::NBModel::setPath(const std::string& path)
  {
    this->path = path;
  }


  int broomie::NBModel::getTrainingNum(){
    return this->addTrainingDataNum;
  }

  /*****************************************************************************
   * Model Factory Impl object
   ****************************************************************************/

  broomie::ModelFactoryImpl::ModelFactoryImpl(){}

  broomie::ModelFactoryImpl::~ModelFactoryImpl(){}

  Model* broomie::ModelFactoryImpl::create(int classifierMethod)
  {
    if(classifierMethod == BAYES){
      Model* model = new broomie::NBModel();
      return model;
    } else {
      return NULL;
    }
  }
}
