/*******************************************************************************
 *
 * Test cases for broomie
 *
 * This test case is using google test framework.
 * website: http://code.google.com/p/googletest/
 *
 * (Assertion samples)
 *   ASSERT_TRUE(condition)      : condition is true
 *   ASSERT_FALSE(condition)     : condition is false
 *   ASSERT_EQ(expected, actual) : expected == actual
 *   ASSERT_NE(val1, val2)       : val1 != val2
 *   ASSERT_LT(val1, val2)       : val1 < val2
 *   ASSERT_LE(val1, val2)       : val1 <= val2
 *   ASSERT_GT(val1, val2)       : val1 > val2
 *   ASSERT_GE(val1, val2)       : val1 >= val2
 *
 ******************************************************************************/

#include <dirent.h>
#include <sys/stat.h>
#include <gtest/gtest.h>
#include "brmutil.hpp"
#include "brmalgorithm.hpp"

const std::string CONFIG_NAME          = "broomie.conf";
const std::string TEST_DIR_PATH        = "/tmp/broomie_test/";
const std::string TRAIN_PATH           = "../test_data/train";
const std::string TEST_PATH            = "../test_data/test";
const unsigned int ZERO                = 0;
const unsigned int TEST_NUM            = 100;
const unsigned int CLASSIFIER_TYPE_NUM = 1;   /* bayes only */

// algorithm
const std::string METHOD_BAYES         = "bayes";

namespace broomie {
  bool testINIT(){
    bool err = true;
    DIR *dir = opendir(TEST_DIR_PATH.c_str());
    if(dir == NULL){
      if(mkdir(TEST_DIR_PATH.c_str(), 0777) == -1){
        err = false;
      }
    }
    //closedir(dir);
    return !err;
  }

  bool testFINISH(){
    bool err = true;
    TCLIST* files = tcreaddir(TEST_DIR_PATH.c_str());
    for(int i = 0; i < tclistnum(files); i++){
      char* filepath = tcsprintf("%s/%s",TEST_DIR_PATH.c_str(), tclistval2(files, i));
      remove(filepath);
      free(filepath);
    }
    tclistdel(files);
    DIR *dir = opendir(TEST_DIR_PATH.c_str());
    if(dir!= NULL){
      if(rmdir(TEST_DIR_PATH.c_str()) == -1){
        std::cerr << "error rmdir: " + TEST_DIR_PATH << std::endl;
        err = false;
      }
    }
    closedir(dir);
    return err;
  }

  broomie::Document *documentCreator(int seed){
    broomie::Document *doc = new broomie::Document(10);
    for(int i = 0; i < 10; i++){
      std::string feature = broomie::testutil::createRandomString(5);
      double point = broomie::testutil::createRandomDouble();
      doc->addFeature(feature, point);
    }
    return doc;
  }

  class BROOMIEModelTestCase : public ::testing::Test {

  public:
    broomie::Classifier* cl;

  protected:
    virtual void SetUp()
    {
      testINIT();
      broomie::ModelFactoryImpl *modelFactory = new broomie::ModelFactoryImpl();
      std::string modelPath = TEST_DIR_PATH;
      cl = new broomie::Classifier(modelFactory, modelPath);
      std::vector<std::string> classNames;
      classNames.push_back("it");
      classNames.push_back("economic");
      classNames.push_back("sport");
      int classifierMethod = broomie::testutil::createRandomVal<int>(CLASSIFIER_TYPE_NUM);
      cl->beginMakingModel(classNames, classifierMethod);
      std::string confPath = TEST_DIR_PATH + CONFIG_NAME;
      std::ofstream ofs(confPath.c_str());
      ofs.write(DEFINE_METHOD_NAME.c_str(), strlen(DEFINE_METHOD_NAME.c_str()));
      ofs.write("\t", 1);
      if(classifierMethod == broomie::BAYES){    // algorithm
        ofs.write(METHOD_BAYES.c_str(), strlen(METHOD_BAYES.c_str()));
      }
      ofs.close();
      delete modelFactory;
    }

    virtual void TearDown()
    {
      cl->endMakingModel();
      delete cl;
    }
  };

  class BROOMIEClassifyTestCase : public ::testing::Test {

  public:
    broomie::Classifier* cl;

  protected:
    virtual void SetUp()
    {
      std::string modelPath = TEST_DIR_PATH;
      broomie::ModelFactoryImpl *modelFactory = new broomie::ModelFactoryImpl();
      cl = new broomie::Classifier(modelFactory, modelPath);
      std::string confPath = modelPath + CONFIG_NAME;
      std::ifstream confifs(confPath.c_str(), std::ios::in);
      if(!confifs){
        std::cerr << "file open error: " << confPath << std::endl;
        return;
      }
      int classifierMethod = 0;
      std::string line;
      while(std::getline(confifs, line)){
        std::vector<std::string> features = broomie::util::split(line, "\t");
        std::cout << features[0] << ":" << features[1] << std::endl;
        if(features[0] == DEFINE_METHOD_NAME){
          // algorithm
          if(features[1] == METHOD_BAYES){
            classifierMethod = broomie::BAYES;
          } else {
            std::cerr << "chose classifier method error: " << features[1] <<  std::endl;
            return;
          }
        }
      }
      cl->beginClassification(classifierMethod);
      delete modelFactory;
    }

    virtual void TearDown()
    {
      cl->endClassification();
      delete cl;
      testFINISH();
    }
  };

  /*****************************************************************************
   *
   * JOIN TEST(check accuracy)
   *
   ****************************************************************************/

  TEST_F(BROOMIEModelTestCase, hoge)
  {
    std::ifstream ifs(TRAIN_PATH.c_str());
    std::string line;
    int cnt = 0;
    while(getline(ifs, line)){
      std::vector<std::string> features = broomie::util::split(line, "\t");
      broomie::Document *doc = new broomie::Document((features.size() - 1) / 2);
      std::string className;
      std::string feature;
      for(unsigned int i = 0; i < features.size(); i++){
        if(i < 1){
          className = features[i];
        } else if(i % 2 > 0){
          feature = features[i];
        } else {
          double point = atof(features[i].c_str());
          doc->addFeature(feature, point);
        }
      }
      cl->addTrainingData(className, *doc);
      delete doc;
      cnt++;
    }
    ifs.close();
    ASSERT_FALSE(cl->checkErr());
  }

  TEST_F(BROOMIEClassifyTestCase, jointest)
  {
    std::string modelPath = TEST_DIR_PATH;
    std::ifstream ifs(TEST_PATH.c_str(), std::ios::in);
    if(!ifs){
      std::cerr << "file open error: " << TEST_PATH << std::endl;
      return;
    }
    int cnt = 0;
    int collectNum = 0;
    std::map<std::string, int> collectCnt;
    std::map<std::string, int> falseCnt;
    std::string line;
    while(getline(ifs, line)){
      std::vector<std::string> features = broomie::util::split(line, "\t");
      broomie::Document *doc = new broomie::Document((features.size() - 1) / 2);
      std::string className;
      std::string feature;
      for(unsigned int i = 0; i < features.size(); i++){
        if(i < 1){
          className = features[i];
        } else if(i % 2 > 0){
          feature = features[i];
        } else {
          double point = atof(features[i].c_str());
          doc->addFeature(feature, point);
        }
      }
      broomie::ResultSet *rs = cl->classify(*doc);
      float maxPoint = 0.0;
      std::string maxPointClass;
      for(int i = 0; i < rs->getResultSetNum(); i++){
        float point;
        std::string className = rs->getResult(i, point);
        if(maxPoint < point || maxPoint == 0.0) {
          maxPoint = point;
          maxPointClass = className;
        }
      }
      if(maxPointClass == className) {
        collectCnt[className] += 1;
        collectNum++;
      } else {
        falseCnt[className] += 1;
      }
      delete rs;
      delete doc;
      cnt++;
    }
    std::cout << "=== classify result ===" << std::endl;
    std::cout << "test num:";
    std::cout << cnt << std::endl;
    std::cout << "collect num:";
    std::cout << collectNum << std::endl;
    std::cout << "accuracy:" << (static_cast<float>(collectNum)/cnt*100) << std::endl;
    std::vector<std::string>* classList = cl->getClassList();
    std::cout << "=== err anal ===" << std::endl;
    for(unsigned int i = 0; i < classList->size(); i++){
      int errNum = falseCnt[(*classList)[i]];
      std::cout << (*classList)[i] + ":";
      std::cout << errNum << std::endl;
    }
    delete classList;
    ifs.close();
    ASSERT_FALSE(cl->checkErr());
  }

  /*****************************************************************************
   *
   * Unit TEST(check accuracy)
   *
   ****************************************************************************/

  /* Document object TEST */
  TEST(broomie, Document_Document){
    for(unsigned int i = 0; i < TEST_NUM; i++){
      broomie::Document *doc = new broomie::Document(broomie::testutil::createRandomVal<double>(100));
      delete doc;
    }
  }


  TEST(broomie, Document_addFeature){
    broomie::Document *doc = new broomie::Document(TEST_NUM);
    int featureNum = 0;
    for(unsigned int i = 0; i < TEST_NUM; i++){
      int featureLength = broomie::testutil::createRandomVal<double>(50);
      featureLength++;
      std::string feature = broomie::testutil::createRandomString(featureLength);
      double point = broomie::testutil::createRandomVal<double>(100);
      doc->addFeature(feature, point);
      ASSERT_EQ(doc->getFeatureNum(),featureNum + 1);
      ASSERT_GT(doc->getFeatureNum(), featureNum);
      featureNum++;
    }

    // Null TEST
    for(unsigned int i = 0; i < TEST_NUM; i++){
      int featureNum = doc->getFeatureNum();
      std::string feature("");
      double point = broomie::testutil::createRandomVal<double>(100);
      doc->addFeature(feature, point);
      ASSERT_EQ(featureNum, doc->getFeatureNum());
    }

    delete doc;
  }


  TEST(broomie, Document_getFeature){
    broomie::Document *doc = new broomie::Document(TEST_NUM);
    int featureNum = 0;
    for(unsigned int i = 0; i < TEST_NUM; i++){
      int featureLength = broomie::testutil::createRandomVal<double>(50);
      featureLength++;
      std::string feature = broomie::testutil::createRandomString(featureLength);
      double point = broomie::testutil::createRandomVal<double>(100);
      point += 0.1;
      doc->addFeature(feature, point);
      ASSERT_EQ(doc->getFeatureNum(),featureNum + 1);
      ASSERT_GT(doc->getFeatureNum(), featureNum);
      featureNum++;
    }

    for(unsigned int i = 0; i < TEST_NUM; i++){
      double point = 0.0;
      std::string feature = doc->getFeature(i, point);
      ASSERT_GT(feature.size(), ZERO);
      ASSERT_GT(point,  0.0);
    }

    for(unsigned int i = TEST_NUM - 1; i < TEST_NUM + 1; i++){
      double point = 0.0;
      std::string feature = doc->getFeature(i, point);
      if(i < TEST_NUM){
        ASSERT_GT(feature.size(), ZERO);
        ASSERT_GT(point,  0.0);
      } else if(i == TEST_NUM){
        ASSERT_EQ(feature.size(), ZERO);
        ASSERT_EQ(point,  0.0);
      } else {
        ASSERT_EQ(feature.size(), ZERO);
        ASSERT_EQ(point,  0.0);
      }
    }

    double point = 0.0;
    std::string feature("");
    feature = doc->getFeature(-1, point);
    ASSERT_EQ(feature.size(), ZERO);
    ASSERT_EQ(point,  0.0);
    feature = "";
    point = 0.0;

    feature = doc->getFeature(0, point);
    ASSERT_GT(feature.size(), ZERO);
    ASSERT_GT(point,  0.0);
    feature = "";
    point = 0.0;

    feature = doc->getFeature((TEST_NUM - 1), point);
    ASSERT_GT(feature.size(), ZERO);
    ASSERT_GT(point,  0.0);
    feature = "";
    point = 0.0;

    feature = doc->getFeature(TEST_NUM, point);
    ASSERT_EQ(feature.size(), ZERO);
    ASSERT_EQ(point,  0.0);
    feature = "";
    point = 0.0;

    feature = doc->getFeature((TEST_NUM + 1), point);
    ASSERT_EQ(feature.size(), ZERO);
    ASSERT_EQ(point,  0.0);
    feature = "";
    point = 0.0;

    delete doc;
  }

  /* ResultSet Object TEST */
  TEST(broomie, ResultSet_ResultSet){
    int resultNum = TEST_NUM;
    broomie::Result result[resultNum];
    for(int i = 0; i < resultNum; i++){
      result[i].name  = broomie::testutil::createRandomString(30);
      result[i].point = broomie::testutil::createRandomVal<double>(100);
      result[i].point += 1;
    }
    broomie::ResultSet *rs = new broomie::ResultSet(result, resultNum);
    ASSERT_EQ(resultNum, rs->getResultSetNum());

    delete rs;
  }

  TEST(broomie, ResultSet_getResultSetNum){
    for(unsigned int i = 0; i < TEST_NUM; i++){
      broomie::Result result[i];
      for(unsigned int j = 0; j < i; j++){
        result[j].name  = broomie::testutil::createRandomString(30);
        result[j].point = broomie::testutil::createRandomVal<double>(100);
        result[j].point += 1;
      }
      broomie::ResultSet* rs = new broomie::ResultSet(result, i);
      int iBuf = i;
      ASSERT_EQ(iBuf, rs->getResultSetNum());
      delete rs;
    }
  }

  TEST(broomie, ResultSet_getResult){
    int resultNum = TEST_NUM;
    broomie::Result result[resultNum];
    for(int i = 0; i < resultNum; i++){
      result[i].name  = broomie::testutil::createRandomString(30);
      result[i].point = broomie::testutil::createRandomVal<double>(100);
      result[i].point += 1;
    }
    broomie::ResultSet *rs = new broomie::ResultSet(result, resultNum);
    for(int j = 0; j < rs->getResultSetNum(); j++){
      float point;
      std::string className = rs->getResult(j, point);
      ASSERT_GT(className.size(), ZERO);
      ASSERT_GT(point, 0);
    }

    float point;
    std::string className;
    className = rs->getResult(rs->getResultSetNum() - 1, point);
    ASSERT_GT(className.size(), ZERO);
    ASSERT_GT(point, 0);
    className = rs->getResult(rs->getResultSetNum(), point);
    ASSERT_EQ(className.size(), ZERO);
    ASSERT_EQ(point, 0.0);
    className = rs->getResult(-1, point);
    ASSERT_EQ(className.size(), ZERO);
    ASSERT_EQ(point, 0.0);
    delete rs;
  }


  // Model Factory TEST
  TEST(broomie, OllModelFactory){
    for(unsigned int i = 0; i < TEST_NUM; i++){
      broomie::ModelFactoryImpl *modelFactory = new broomie::ModelFactoryImpl();
      delete modelFactory;
    }
  }

  /* Classifier object TEST */
  TEST(broomie, Classifier_Classifier){
    testINIT();
    std::string basePath = TEST_DIR_PATH;
    broomie::ModelFactoryImpl *modelFactory = new broomie::ModelFactoryImpl();
    broomie::Classifier cl(modelFactory, basePath);
    ASSERT_EQ(cl.getClassSettingNum(), 0);
    ASSERT_FALSE(cl.checkErr());
    std::string errMessage = cl.traceErr();
    ASSERT_EQ(errMessage.size(), ZERO);

    std::string basePath2("");
    broomie::Classifier cl2(modelFactory, basePath2);
    ASSERT_EQ(cl2.getClassSettingNum(), 0);
    ASSERT_TRUE(cl2.checkErr());
    errMessage = cl2.traceErr();
    ASSERT_NE(errMessage.size(), ZERO);

    testFINISH();

    delete modelFactory;
  }


  TEST(broomie, Classifier_beginMakingModel){
    std::string basePath = TEST_DIR_PATH;
    broomie::ModelFactoryImpl *modelFactory = new broomie::ModelFactoryImpl();
    broomie::Classifier cl(modelFactory, basePath);
    std::vector<std::string> classNames;
    std::string className1 = broomie::testutil::createRandomString(10);
    std::string className2 = broomie::testutil::createRandomString(10);
    std::string className3 = broomie::testutil::createRandomString(10);
    classNames.push_back(className1);
    classNames.push_back(className2);
    classNames.push_back(className3);
    for(unsigned int i = 0; i < CLASSIFIER_TYPE_NUM; i++){
      testINIT();
      ASSERT_TRUE(cl.beginMakingModel(classNames, i));
      cl.endMakingModel();
      testFINISH();
    }
    classNames.clear();

    // class type only 1
    broomie::Classifier cl2(modelFactory, basePath);
    classNames.push_back(className1);
    for(unsigned int i = 0; i < CLASSIFIER_TYPE_NUM; i++){
      testINIT();
      ASSERT_FALSE(cl2.beginMakingModel(classNames, i));
      std::string errMessage("");
      ASSERT_TRUE(cl2.checkErr());
      errMessage = cl2.traceErr();
      ASSERT_NE(errMessage.size(), ZERO);
      cl2.endMakingModel();
      testFINISH();
    }
    delete modelFactory;
  }

  TEST(broomie, Classifier_addTrainigData){
    std::string basePath = TEST_DIR_PATH;
    broomie::ModelFactoryImpl *modelFactory = new broomie::ModelFactoryImpl();
    broomie::Classifier cl(modelFactory, basePath);
    std::vector<std::string> classNames;
    std::string className1 = broomie::testutil::createRandomString(10);
    std::string className2 = broomie::testutil::createRandomString(10);
    std::string className3 = broomie::testutil::createRandomString(10);
    classNames.push_back(className1);
    classNames.push_back(className2);
    classNames.push_back(className3);

    for(int i = 0; i < 2; i++){
      testINIT();
      if(cl.beginMakingModel(classNames, i) == false){
        cl.endMakingModel();
        continue;
      }

      for(unsigned int j = 0; j < TEST_NUM; j++){
        broomie::Document *doc = documentCreator(j);
        ASSERT_TRUE(cl.addTrainingData(classNames[j%3], *doc));
        ASSERT_FALSE(cl.addTrainingData(broomie::testutil::createRandomString(10), *doc));
        ASSERT_FALSE(cl.addTrainingData("", *doc));
        delete doc;
      }
      cl.endMakingModel();
      testFINISH();
    }
    classNames.clear();
    delete modelFactory;
  }

  /*
    TEST(broomie, Classifier_endMakingModel){
    testINIT();
    std::string basePath = TEST_DIR_PATH;
    broomie::ModelFactoryImpl *modelFactory = new broomie::ModelFactoryImpl();
    broomie::Classifier cl(modelFactory, basePath);
    std::vector<std::string> classNames;
    std::string className1 = broomie::testutil::createRandomString(10);
    std::string className2 = broomie::testutil::createRandomString(10);
    std::string className3 = broomie::testutil::createRandomString(10);
    classNames.push_back(className1);
    classNames.push_back(className2);
    classNames.push_back(className3);
    cl.beginMakingModel(classNames, broomie::OLL);
    for(unsigned int i = 0; i < TEST_NUM; i++){
      broomie::Document *doc = documentCreator(i);
      cl.addTrainingData(classNames[i%3], *doc);
      delete doc;
    }
    classNames.clear();
    ASSERT_TRUE(cl.endMakingModel());
    delete modelFactory;
    testFINISH();
  }
  */

  TEST(broomie, Classifier_endMakingModel){
    testINIT();
    std::string basePath = TEST_DIR_PATH;
    broomie::ModelFactoryImpl *modelFactory = new broomie::ModelFactoryImpl();
    broomie::Classifier cl(modelFactory, basePath);
    std::vector<std::string> classNames;
    std::string className1 = broomie::testutil::createRandomString(10);
    std::string className2 = broomie::testutil::createRandomString(10);
    std::string className3 = broomie::testutil::createRandomString(10);
    classNames.push_back(className1);
    classNames.push_back(className2);
    classNames.push_back(className3);
    cl.beginMakingModel(classNames, broomie::BAYES);
    for(unsigned int i = 0; i < TEST_NUM; i++){
      broomie::Document *doc = documentCreator(i);
      cl.addTrainingData(classNames[i%3], *doc);
      delete doc;
    }
    classNames.clear();
    ASSERT_TRUE(cl.endMakingModel());
    delete modelFactory;
    testFINISH();
  }


  TEST(broomie, Classifier_beginClassification){
    std::string basePath = TEST_DIR_PATH;
    broomie::ModelFactoryImpl *modelFactory = new broomie::ModelFactoryImpl();
    std::vector<std::string> classNames;
    std::string className1 = broomie::testutil::createRandomString(10);
    std::string className2 = broomie::testutil::createRandomString(10);
    std::string className3 = broomie::testutil::createRandomString(10);
    classNames.push_back(className1);
    classNames.push_back(className2);
    classNames.push_back(className3);
    for(unsigned int i = 0; i < 2; i++){
      testINIT();
      broomie::Classifier cl(modelFactory, basePath);
      if(!(cl.beginMakingModel(classNames, i))){
        cl.endMakingModel();
        continue;
      }
      for(unsigned int j = 0; j < TEST_NUM; j++){
        broomie::Document *doc = documentCreator(j);
        cl.addTrainingData(classNames[j%3], *doc);
        delete doc;
      }
      cl.endMakingModel();
      ASSERT_TRUE(cl.beginClassification(i));
      for(unsigned int k = 0; k < TEST_NUM; k++){
        broomie::Document *doc = documentCreator(k);
        broomie::ResultSet *rs = cl.classify(*doc);
        delete rs;
        delete doc;
      }
      cl.endClassification();
      testFINISH();
    }
    delete modelFactory;
  }
}


int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  bool ok = true;
  if(argc > 1){
    std::vector<std::string> arg(argc-1);
    for(int i = 0; i < argc - 1; i++){
      arg[i] = argv[i+1];
    }
    if(arg[0] == "forever"){
      while(1){
        ok = RUN_ALL_TESTS();
      }
    }
  }
  return RUN_ALL_TESTS();
}
