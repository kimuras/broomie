#include "libbrm.hpp"

namespace broomie {

  const char* codecErrMessage(int ecode){
    switch(ecode){
    case BROOMIESUCCESS: return "success";
    case BROOMIEEDBOPEN: return "open db error";
    case BROOMIEEDBCLOSE: return "close db error";
    case BROOMIEELOADCLASSLIST: return "load class list error";
    case BROOMIEECREATEMODEL: return "create model error";
    case BROOMIEESAVEMODEL: return "save model error";
    case BROOMIEELOADMODEL: return "load model error";
    case BROOMIEEPUTTRAIN: return "put training data error";
    case BROOMIEECONSTRUCTOR: return "create constructor error";
    case BROOMIEEPUTCLASSNAME: return "put class name error";
    }
    return "unknown error";
  }

  /*****************************************************************************
   * Documnet object
   ****************************************************************************/

  Document::Document(int fNum) : featureNum(0), featureWord(fNum){}

  void Document::addFeature(const std::string word, double point)
  {
    if(word.size() <= 0) return;
    if(!point) point = 0.0;
    featureWord[featureNum].word = word;
    featureWord[featureNum].point = point;
    featureNum++;
  }


  const std::string Document::getFeature(int index, double& point) const
  {
    if(index > (featureNum - 1) || index < 0){
      point = 0.0;
      return "";
    }
    point = featureWord[index].point;
    return featureWord[index].word;
  }


  int Document::getFeatureNum() const
  {
    return featureNum;
  }

  /*****************************************************************************
   * Classifier classes
   ****************************************************************************/

  ResultSet::ResultSet(const Result results[], int num)
  {
    this->results = new Result[num];
    int i = 0;
    for(i = 0; i < num; i++){
      this->results[i] = results[i];
    }
    resultSetNum = i;
  }

  ResultSet::~ResultSet()
  {
    delete[] results;
  }

  int ResultSet::getResultSetNum()
  {
    return resultSetNum;
  }

  const std::string ResultSet::getResult(int i, float& point){
    if(i < 0 || i >= resultSetNum){
      point = 0.0;
      return "";
    }
    point = results[i].point;
    return results[i].name;
  }

  Classifier::Classifier(ModelFactory* modelFactory, std::string& basePath) :
    modelFactory(modelFactory), basePath(basePath), classSettings(0),
    classSettingNum(0), wordDicMaxNum(0), classListBuf(), classListBufIter()
  {
    if(basePath.size() > 0){
      if(basePath[basePath.size() - 1] != '/'){
        this->basePath = basePath + '/';
      }
    } else {
      setErrMessage(broomie::BROOMIEECONSTRUCTOR,
                    __FILE__, __LINE__, __func__, "basePath is wrong.");
    }
    wordDic = NULL;
    wordDicInverse = NULL;
    classList = NULL;
  }

  Classifier::~Classifier(){}

  bool Classifier::checkErr()
  {
    bool err = errMessage.size();
    return err;
  }

  void Classifier::setErrMessage(int ecode, const char* fileName, int line,
                                 const char* function, const char* otherMessage)
  {
    char* errMessageBuf =
      tcsprintf("%s: %s L.%d [%s] : %s",
                codecErrMessage(ecode), fileName, line, function, otherMessage);
    errMessage.push_back(errMessageBuf);
    free(errMessageBuf);
  }

  void Classifier::clearErrStack()
  {
    errMessage.clear();
  }

  const std::string Classifier::traceErr()
  {
    if(errMessage.size() < 0) return "";
    std::string errMessages("");
    for(unsigned int i = 0; i < errMessage.size(); i++){
      errMessages.append(errMessage[i]);
      errMessages.append("\n");
    }
    errMessage.clear();
    return errMessages;
  }


  bool Classifier::beginMakingModel(const std::vector<std::string>& classNames,
                                    const int classifierMethod)
  {
    bool ok = true;
    clearErrStack();
    classSettingNum = classNames.size();
    if(classSettingNum < 2){
      setErrMessage(broomie::BROOMIEELOADCLASSLIST,
                    __FILE__, __LINE__, __func__, "class type need 2 types or more.");
      return false;
    }
    if(!openWordDic(broomie::CREATE)) return false;
    if(!openClassList(broomie::CREATE)) return false;
    classSettings = new ClassSetting[classSettingNum];
    for(int i = 0; i < classSettingNum; i++){
      Model* model = modelFactory->create(classifierMethod);
      if(!model){
        setErrMessage(broomie::BROOMIEECREATEMODEL, __FILE__, __LINE__, __func__, "");
        classSettings[i].model = NULL;
        delete model;
        return false;
      }
      model->setPath(basePath + classNames[i] + ".model");
      classSettings[i].name = classNames[i];
      classSettings[i].model = model;
      classListBuf.insert(std::pair<std::string, int>(classNames[i], 1));
      if(!tchdbput2(classList, classNames[i].c_str(), "1")){
        setErrMessage(broomie::BROOMIEEPUTCLASSNAME, __FILE__, __LINE__, __func__, "");
        return false;
      }
    }
    return ok;
  }

  bool Classifier::endMakingModel()
  {
    bool ok = true;
    clearErrStack();
    if(classSettingNum > 1){
      for(int i = 0; i < classSettingNum; i++){
        if(classSettings[i].name.size() > 0){
          if(!classSettings[i].model->save()){
            setErrMessage(broomie::BROOMIEESAVEMODEL, __FILE__, __LINE__, __func__, "");
            ok = false;
          }
          delete classSettings[i].model;
        }
      }
      tchdbiterinit(wordDic);
      char* key;
      while((key = tchdbiternext2(wordDic)) != NULL){
        char* val = tchdbget2(wordDic, key);
        tchdbput2(wordDicInverse, val, key);
        free(key);
        free(val);
      }
      delete[] classSettings;
      if(!closeWordDic()){
        ok = false;
        setErrMessage(broomie::BROOMIEEDBCLOSE,
                    __FILE__, __LINE__, __func__, "");
      }
      if(!closeClassList()){
        ok = false;
        setErrMessage(broomie::BROOMIEEDBCLOSE,
                      __FILE__, __LINE__, __func__, "");
      }
    }
    return ok;
  }

  bool Classifier::addTrainingData(const std::string& className, const Document& doc)
  {
    bool ok = true;
    clearErrStack();
    if(doc.getFeatureNum() < 1) return false;
    if(className.size() < 1){
      setErrMessage(broomie::BROOMIEECLASSNAME,
                    __FILE__, __LINE__, __func__, "class name size is zero.");
      return false;
    }
    classListBufIter = classListBuf.find(className);
    if(classListBufIter == classListBuf.end()){
      setErrMessage(broomie::BROOMIEELOADCLASSLIST,
                    __FILE__, __LINE__, __func__, "class name not found.");
      return false;
    }
    int documentFeatureNum = doc.getFeatureNum();
    Feature features[documentFeatureNum];
    for(int j = 0; j < documentFeatureNum; j++){
      std::string word = doc.getFeature(j, features[j].point);
      uint64_t id = convertWordToId(word, broomie::LEARN);
      features[j].id = id;
    }
    for(int k = 0; k < classSettingNum; k++){
      if(classSettings[k].name == className){
        if(!classSettings[k].model->put(features, documentFeatureNum, broomie::POSITIVE)){
          ok = false;
          setErrMessage(broomie::BROOMIEEPUTTRAIN,
                        __FILE__, __LINE__, __func__, "check mode option");
        }
      } else {
        if(!classSettings[k].model->put(features, documentFeatureNum, broomie::NEGATIVE)){
          ok = false;
          setErrMessage(broomie::BROOMIEEPUTTRAIN,
                        __FILE__, __LINE__, __func__, "check mode option");
        }
      }
    }
    return ok;
  }

  uint64_t Classifier::convertWordToId(const std::string& word, int mode)
  {
    uint64_t id = 0;
    char* idBuf = tchdbget2(wordDic, word.c_str());
    if(idBuf){
      id = atol(idBuf);
    } else {
      if(mode == LEARN){
        id = ++wordDicMaxNum;
        char* idSave = tcsprintf("%lld", id);
        tchdbput2(wordDic, word.c_str(), idSave);
        free(idSave);
      }
    }
    free(idBuf);
    return id;
  }

  std::vector<std::string>* Classifier::getClassList()
  {
    std::vector<std::string>* classListBuf = new std::vector<std::string>;
    tchdbiterinit(classList);
    char* keyBuf;
    while((keyBuf= tchdbiternext2(classList)) != NULL){
      std::string key = keyBuf;
      classListBuf->push_back(key);
      free(keyBuf);
    }
    return classListBuf;
  };

  int Classifier::getClassSettingNum()
  {
    return classSettingNum;
  }


  bool Classifier::beginClassification(int classifierMethod)
  {
    bool ok = true;
    clearErrStack();
    if(!openWordDic(broomie::READ)) return false;
    if(!openClassList(broomie::READ)) return false;
    classSettingNum = tchdbrnum(classList);
    if(classSettingNum < 2){
      setErrMessage(broomie::BROOMIEELOADCLASSLIST,
                    __FILE__, __LINE__, __func__, "class type need 2 types or more.");
      ok = false;
      return false;
    }
    classSettings = new ClassSetting[classSettingNum];
    tchdbiterinit(classList);
    char* name;
    int i = 0;
    while((name = tchdbiternext2(classList)) != 0){
      Model* model = modelFactory->create(classifierMethod);
      if(!model){
        setErrMessage(broomie::BROOMIEECREATEMODEL, __FILE__, __LINE__, __func__, "");
        return false;
      }
      model->setPath(basePath + name + ".model");
      if(!model->load()){
        setErrMessage(broomie::BROOMIEELOADMODEL, __FILE__, __LINE__, __func__, "");
        return false;
      }
      classSettings[i].name = name;
      classSettings[i].model = model;
      i++;
      free(name);
    }
    return ok;
  }

  bool Classifier::endClassification()
  {
    bool ok = true;
    clearErrStack();
    for(int i = 0; i < classSettingNum; i++){
      delete classSettings[i].model;
    }
    delete[] classSettings;
    if(!closeWordDic()){
      ok = false;
      setErrMessage(broomie::BROOMIEEDBCLOSE,
                    __FILE__, __LINE__, __func__, "");
    }
    if(!closeClassList()){
      ok = false;
      setErrMessage(broomie::BROOMIEEDBCLOSE,
                      __FILE__, __LINE__, __func__, "");
    }
    return ok;
  }

  ResultSet* Classifier::classify(const Document& doc)
  {
    int documentFeatureNum = doc.getFeatureNum();
    Feature features[documentFeatureNum];
    for(int i = 0; i < documentFeatureNum; i++){
      std::string word = doc.getFeature(i, features[i].point);
      uint64_t id = convertWordToId(word, broomie::CLASSIFY);
      features[i].id = id;
    }
    Result results[classSettingNum];
    for(int j = 0; j < classSettingNum; j++){
      results[j].name = classSettings[j].name;
      results[j].point =
        classSettings[j].model->classify(features, documentFeatureNum);
    }
    ResultSet* rs = new ResultSet(results, classSettingNum);
    return rs;
  }

  bool Classifier::closeWordDic()
  {
    bool ok = true;
    clearErrStack();
    int ecode;
    const char* wordDicMaxNumBuf = tcsprintf("%lld", wordDicMaxNum);
    if(wordDic){
      tchdbput2(wordDic, KEY_DIC_MAX_NUM.c_str(), wordDicMaxNumBuf);
      free(const_cast<char*>(wordDicMaxNumBuf));
      if(!tchdbclose(wordDic)){
        ecode = tchdbecode(wordDic);
        setErrMessage(broomie::BROOMIEEDBCLOSE,
                      __FILE__, __LINE__, __func__, tchdberrmsg(ecode));
        ok = false;
      }
      tchdbdel(wordDic);
    }

    if(wordDicInverse){
      if(!tchdbclose(wordDicInverse)){
        ecode = tchdbecode(wordDicInverse);
        setErrMessage(broomie::BROOMIEEDBCLOSE,
                      __FILE__, __LINE__, __func__, tchdberrmsg(ecode));
        ok = false;
      }
      tchdbdel(wordDicInverse);
    }
    return ok;
  }

  bool Classifier::openWordDic(int omode = READ)
  {
    bool ok = true;
    clearErrStack();
    if(omode != READ && omode != CREATE){
      return false;;
    }
    wordDic = tchdbnew();
    wordDicInverse = tchdbnew();
    std::string wordDicPath = basePath + WORD_DIC_DB_NAME;
    std::string wordDicInversePath = basePath + WORD_DIC_INVERSE_DB_NAME;
    if(omode == CREATE){
      if(!tchdbopen(wordDic, wordDicPath.c_str(),
                    HDBOWRITER | HDBOCREAT | HDBOTRUNC)){
        int ecode = tchdbecode(wordDic);
        setErrMessage(broomie::BROOMIEEDBOPEN,
                      __FILE__, __LINE__, __func__, tchdberrmsg(ecode));
        ok = false;
      }
      if(!tchdbopen(wordDicInverse, wordDicInversePath.c_str(),
                    HDBOWRITER | HDBOCREAT | HDBOTRUNC)){
        int ecode = tchdbecode(wordDicInverse);
        setErrMessage(broomie::BROOMIEEDBOPEN,
                      __FILE__, __LINE__, __func__, tchdberrmsg(ecode));
        ok = false;
      }
    } else if(omode == READ){
      if(!tchdbopen(wordDic, wordDicPath.c_str(), HDBOREADER)){
        int ecode = tchdbecode(wordDic);
        setErrMessage(broomie::BROOMIEEDBOPEN,
                      __FILE__, __LINE__, __func__, tchdberrmsg(ecode));
        ok = false;
      }
      char* wordDicMaxNumBuf = tchdbget2(wordDic, KEY_DIC_MAX_NUM.c_str());
      if(wordDicMaxNumBuf) wordDicMaxNum = std::atol(wordDicMaxNumBuf);
      free(wordDicMaxNumBuf);
      if(!tchdbopen(wordDicInverse, wordDicInversePath.c_str(), HDBOREADER)){
        int ecode = tchdbecode(wordDicInverse);
        setErrMessage(broomie::BROOMIEEDBOPEN,
                      __FILE__, __LINE__, __func__, tchdberrmsg(ecode));
        ok = false;
      }
    }
    return ok;
  }

  bool Classifier::openClassList(int omode = READ)
  {
    bool ok = true;
    clearErrStack();
    if(omode != READ && omode != CREATE){
      ok = false;
      return ok;
    }
    classList = tchdbnew();
    std::string classListPath = basePath + CLASS_LIST_DB_NAME;
    if(omode == CREATE){
      if(!tchdbopen(classList, classListPath.c_str(),
                    HDBOWRITER | HDBOCREAT | HDBOTRUNC)){
        int ecode = tchdbecode(classList);
        setErrMessage(broomie::BROOMIEEDBOPEN,
                      __FILE__, __LINE__, __func__, tchdberrmsg(ecode));
        ok = false;
      }
    } else if(omode == READ){
      if(!tchdbopen(classList, classListPath.c_str(), HDBOREADER)){
        int ecode = tchdbecode(classList);
        setErrMessage(broomie::BROOMIEEDBOPEN,
                      __FILE__, __LINE__, __func__, tchdberrmsg(ecode));
        ok = false;
      }
    }
    return ok;
  }

  bool Classifier::closeClassList()
  {
    bool ok = true;
    clearErrStack();
    int ecode;
    if(classList){
      if(!tchdbclose(classList)){
        ecode = tchdbecode(classList);
        setErrMessage(broomie::BROOMIEEDBCLOSE,
                      __FILE__, __LINE__, __func__, tchdberrmsg(ecode));
        ok = false;
      }
      tchdbdel(classList);
    }
    return ok;
  }
}
