/*
 * Copyright (C) 2009 Shunya Kimura <brmtrain@gmail.com>
 * All Rights Reserved.
 *
 * Use and distribution of this program is licensed under the
 * BSD license. See the COPYING file for full text.
 */

/*
 *
 * Test cases for brmNB
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
 */

#include <dirent.h>
#include <sys/stat.h>
#include <gtest/gtest.h>
#include "brmNB.hpp"
#include "brmutil.hpp"

const std::string TEST_DIR_PATH        = "/tmp/broomie_test/";
const std::string TRAIN_PATH           = "../test_data/train";
const std::string TEST_PATH            = "../test_data/test";
const unsigned int TEST_NUM            = 100;

unsigned int numPosTrainBuf = 0;
unsigned int numNegTrainBuf = 0;
double sumPosValBuf = 0.0;
double sumNegValBuf = 0.0;
int uniqValBuf = 0;


namespace broomie {

  bool testINIT(){
    bool err = true;
    DIR *dir = opendir(TEST_DIR_PATH.c_str());
    if(dir == NULL){
      if(mkdir(TEST_DIR_PATH.c_str(), 0777) == -1){
        err = false;
      }
    }
    closedir(dir);
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

  class BRMNBModel : public ::testing::Test {

  public:
    broomie::NB::NaiveBayes* nb;
  protected:
    virtual void SetUp()
    {
      testINIT();
      nb = new broomie::NB::NaiveBayes();
    }

    virtual void TearDown()
    {
      delete nb;
    }

  };

  class BRMNBClassify : public ::testing::Test {

  public:
    broomie::NB::NaiveBayes* nb;
  protected:
    virtual void SetUp()
    {
      testINIT();
      nb = new broomie::NB::NaiveBayes();
    }

    virtual void TearDown()
    {
      delete nb;
    }

  };


  TEST_F(BRMNBModel, createModel)
  {
    std::string basePath = TEST_DIR_PATH;
    std::map<int, int> featureBuf;
    int trainNum = broomie::testutil::createRandomVal<double>(100);
    for(int i = 0; i < trainNum; i++){
      broomie::NB::FeatureVal fv;
      double sumValBuf = 0.0;
      int featureNum = broomie::testutil::createRandomVal<double>(100);
      for(int j = 0; j <featureNum; j++){
        double val = broomie::testutil::createRandomVal<double>(10);
        fv.push_back(std::make_pair(j, val));
        featureBuf[j] = 1;
        sumValBuf += val;
      }
      if(i % 2 == 0){
        ASSERT_TRUE(nb->putTrain(fv, broomie::NB::POSITIVE));
        sumPosValBuf += sumValBuf;
        numPosTrainBuf++;
      } else {
        ASSERT_TRUE(nb->putTrain(fv, broomie::NB::NEGATIVE));
        sumNegValBuf += sumValBuf;
        numNegTrainBuf++;
      }
    }
    uniqValBuf = featureBuf.size();
    ASSERT_EQ(numPosTrainBuf, nb->getNumPosTrain());
    ASSERT_EQ(numNegTrainBuf, nb->getNumNegTrain());
    ASSERT_EQ(sumPosValBuf, nb->getSumPosVal());
    ASSERT_EQ(sumNegValBuf, nb->getSumNegVal());
    std::string modelPath = basePath + "model";
    ASSERT_TRUE(nb->saveTrain(modelPath));
  }


  TEST_F(BRMNBClassify, classify)
  {
    std::string basePath = TEST_DIR_PATH;
    std::string modelPath = basePath + "model";
    ASSERT_TRUE(nb->load(modelPath, broomie::NB::READ));
    ASSERT_EQ(numPosTrainBuf, nb->getNumPosTrain());
    ASSERT_EQ(numNegTrainBuf, nb->getNumNegTrain());
    ASSERT_EQ(sumPosValBuf, nb->getSumPosVal());
    ASSERT_EQ(sumNegValBuf, nb->getSumNegVal());
    ASSERT_EQ(uniqValBuf, nb->getUniqVal());
    testFINISH();
  }


}



int main(int argc, char** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

