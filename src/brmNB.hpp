/*
 * Copyright (C) 2009 Shunya Kimura <brmtrain@gmail.com>
 * All Rights Reserved.
 *
 * Use and distribution of this program is licensed under the
 * BSD license. See the COPYING file for full text.
 */


#include <vector>
#include <utility>
#include <cstring>
#include <cstdlib>
#include <iterator>
#include <map>
#include <cmath>

/* tokyocabinet library */
#include <tcutil.h>
#include <tchdb.h>

namespace broomie {

  namespace NB {
    typedef std::vector<std::pair<int, double> > FeatureVal;
    typedef std::map<int, std::pair<double, double> > TrainingData;

    /* constant variables */
    const std::string DEFINE_DOC_NUM_KEY = "NUMBER_OF_DOCUMENT";
    const std::string DEFINE_SUM_VAL_KEY = "SUM_OF_VALUE";
    const double SMOOTHING_W = 0.001;

    /*!
      @brief model open mode.
      - READ : read only.
      - TRUNC : create new model.
    */
    enum {
      READ  = 1,
      TRUNC = 2,
    };


    /*!
      @brief Setting of training data.
      - POSITIVE : Positive training data.
      - NEGATIVE : Negative training data.
    */
    enum {
      POSITIVE = 1,
      NEGATIVE = 2,
    };


    /*!
     * Naive Bayes Classifier Class.
     * - create model for Naive Bayes classifier.
     * - classify example with Naive Bayes classifier.
     */
    class NaiveBayes {

    public:

      /*! @brief Constructor of NaiveBayes object. */
      NaiveBayes();


      /*! @brief Destructor of NaiveBayes object. */
      ~NaiveBayes();


      /*!
        @brief this method is userd in order add features to Naive Bayes learning model.
        @param fv : vector object including pair of feature id and value of the feature.
        @param mode : set the features broomie::NB::POSTIVE sample or broomie::NB::NEGATIVE sample.
        @return return true if success, return false if fail.
     */
      bool putTrain(const FeatureVal& fv, int mode);


      /*!
        @brief this method is used in order to save the learning model.
        @param filePath : set the file path of saving model.
        @return return true if success, return false if fail.
      */
      bool saveTrain(std::string& filePath);


      /*!
        @brief this method is used in order to load the learning model.
        @param filePath : file path of loading model.
        @param omode : set a model open mode broomie::NB::TRUNC or broomie::NB::READ.
        @return return true if success, return false if fail.
        @attention broomie::NB::TRUNC is create new model, broomie::NB::READ is read only.
      */
      bool load(const std::string& filePath, int omode);


      /*!
        @brief this method is used in order to classify each example.
        @param fv : test example. ector object including pair of feature id and value of the feature.
        @return : the point of result of classify.
     */
      double classify(const FeatureVal& fv);

      unsigned int getNumPosTrain();

      unsigned int getNumNegTrain();

      double getSumPosVal();

      double getSumNegVal();

      int getUniqVal();

    private:
      /*! @brief Number of Positive training data in model. */
      unsigned int numPosTrain;

      /*! @brief Number of Negative training data in model. */
      unsigned int numNegTrain;

      /*! @brief map object for counting feature, */
      TrainingData trainingCount;

      /*! @brief Sum of feature values in positive data. */
      double sumPosVal;

      /*! @brief Sum of feature values in negative data. */
      double sumNegVal;

      /*! @brief Number of uniq feature. */
      int uniqVal;

      /*! @brief DB for saving learning model. */
      TCHDB* trainDB;

      /*!
        @brief smoothing each value ot the feature, like a `expected likelihood estimation'.
        @param val : value of the feature,
        @param numTrain :
        @return return true if success, return false if fail.
     */
      void smoothing(double& val, double sumFeatureVal);
    };
  }
}
