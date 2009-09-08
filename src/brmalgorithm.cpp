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


#ifdef USE_TINYSVM

  /*****************************************************************************
   * SvmModel object
   ****************************************************************************/

  broomie::SvmModel::SvmModel() : example(), param(), model(),
                                  addTrainingDataNum(0)
  {
    const char* paramBuf = "-t 1 -d 2 -c 1";
    param.set(paramBuf);
  }

  broomie::SvmModel::~SvmModel(){}

  bool broomie::SvmModel::put(const Feature features[], int num, int mode)
  {
    bool ok = true;
    std::string ex("");
    for(int i = 0; i < num; i++){
      char* exBuf = tcsprintf("%llu:%f", features[i].id, features[i].point);
      if(i == 0) {
        ex = exBuf;
      } else {
        ex += " ";
        ex += exBuf;
      }
      free(exBuf);
    }
    double flag = 0;
    if(mode == POSITIVE){
      flag = 1;
      this->addTrainingDataNum++;
    } else if(mode == NEGATIVE){
      flag = -1;
      this->addTrainingDataNum++;
    } else {
      ok = false;
    }
    if(ok){
      const char* node = ex.c_str();
      int rv = example.add(flag, node);
      if(!rv) ok = false;
    }
    return ok;
  }

  bool broomie::SvmModel::save()
  {
    bool ok = true;
    if(getTrainingNum() > 0){
      TinySVM::Model *m = example.learn(param);
      if(!m->write(path.c_str())) ok = false;
      delete m;
    }
    return ok;
  }

  bool broomie::SvmModel::load()
  {
    bool ok = true;
    if(!model.read(path.c_str())) ok = false;
    return ok;
  }

  double broomie::SvmModel::classify(const Feature features[], int num)
  {
    std::string ex("");
    for(int i = 0; i < num; i++){
      if(features[i].id != 0){
        char* exBuf = tcsprintf("%llu:%f", features[i].id, features[i].point);
        if(i == 0){
          ex = exBuf;
        } else {
          ex += " ";
          ex += exBuf;
        }
        free(exBuf);
      }
    }
    const char *node = ex.c_str();
    double point = model.classify(node);
    return point;
  }

  void broomie::SvmModel::setPath(const std::string& path)
  {
    assert(path.size() > 0);
    this->path = path;
  }

  int broomie::SvmModel::getTrainingNum()
  {
    return this->addTrainingDataNum;
  }

#endif // USE_TINYSVM

#ifdef USE_OLL
  /*****************************************************************************
   * OLLModel object implement
   ****************************************************************************/
  broomie::OllModel::OllModel() :
    path(""), oll(), tm(oll_tool::PA1), a(), addTrainingDataNum(0){}

  broomie::OllModel::~OllModel(){}

  bool broomie::OllModel::put(const Feature features[], int num, int mode)
  {
    bool ok = true;
    oll_tool::fv_t fv;

    for(int i = 0; i < num; i++){
      fv.push_back(std::make_pair(features[i].id, features[i].point));
    }
    if(mode == POSITIVE){
      oll.trainExample(a, fv, +1);
      this->addTrainingDataNum++;
    } else if(mode == NEGATIVE){
      oll.trainExample(a, fv, -1);
      this->addTrainingDataNum++;
    } else {
      ok = false;
    }
    return ok;
  }

  bool broomie::OllModel::save()
  {
    bool ok = true;
    if(oll.save(path.c_str()) == -1) ok = false;
    return ok;
  }

  bool broomie::OllModel::load()
  {
    bool ok = true;
    if(oll.load(path.c_str()) == -1) ok = false;
    return ok;
  }

  double broomie::OllModel::classify(const Feature features[], int num)
  {
    oll_tool::fv_t fv;
    for(int i = 0; i < num; i++){
      if(features[i].id != 0){
        fv.push_back(std::make_pair(features[i].id, features[i].point));
      }
    }
    const float score = oll.classify(fv);
    return score;
  }

  void broomie::OllModel::setPath(const std::string& path)
  {
    assert(path.size() > 0);
    this->path = path;
  }

  int broomie::OllModel::getTrainingNum()
  {
    return this->addTrainingDataNum;
  }

#endif // USE_OLL


  /*****************************************************************************
   * Model Factory Impl object
   ****************************************************************************/

  broomie::ModelFactoryImpl::ModelFactoryImpl(){}

  broomie::ModelFactoryImpl::~ModelFactoryImpl(){}

  Model* broomie::ModelFactoryImpl::create(int classifierMethod)
  {
    Model* model = NULL;
    if(classifierMethod == BAYES){
      model = new broomie::NBModel();
#ifdef USE_OLL
    } else if(classifierMethod == OLL){
      model = new broomie::OllModel();
#endif
#ifdef USE_TINYSVM
    } else if(classifierMethod == TINYSVM){
      model = new broomie::SvmModel();
#endif
    } else {
      model = NULL;
    }
    return model;
  }
}
