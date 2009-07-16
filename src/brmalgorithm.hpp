#include "libbrm.hpp"
#include "brmNB.hpp"

namespace broomie {

  enum classifierMethod {
    BAYES  = 0,
  };


  class NBModel : public Model {

  public:
    NBModel();

    ~NBModel();

    /*!
      @brief this method is userd in order add features to learning model.
      @param features[] : array of Feature objects.including feature name and point of it.
      @param num : num of features.
      @param mode : set the features POSTIVE sample or NEGATIVE sample.
      @return return true if success, return false if fail.
      @attention mode parameter must use enum object of broomie::POSITIVE or broomie::NEGATIVE.
     */
    virtual bool put(const Feature features[], int num, int mode);


    /*!
      @brief this method is used in order to save the learning model.
      @return return true if success, return false if fail.
     */
    virtual bool save();


    /*!
      @brief this method is used in order to load the learning model.
      @return return true if success, return false if fail.
     */
    virtual bool load();


    /*!
      @brief this method is used in order to classify each example.
      @param features[] : test sample. array of Feature objects.including feature name and point of it.
      @param num : num of features.
      @return : the point of result of classify.
     */
    virtual double classify(const Feature features[], int num);


  private:
    /*! @brief the region which saves model directory path */
    std::string path;

    /*! @brief naivebayes object */
    broomie::NB::NaiveBayes nb;

    /*! @brief number of training data*/
    int addTrainingDataNum;

    /*!
      @brief this method is used in order to set directory path of saving model.
      @param path : directory is must created befor set.
     */
    virtual void setPath(const std::string& path);


    /*!
      @brief This method is used in order to get num of traing data in learning model.
      @return Num of training data.
     */
    virtual int getTrainingNum();
  };


  /**
   * Class in order to create oll model Object.
   */
  class ModelFactoryImpl : public ModelFactory {

  public:
    /*!
     * @brief Constructor of Model object of oll.
     */
    ModelFactoryImpl();


    /*!
     * @brief Destructor of ModelFactory object of oll.
     */
    ~ModelFactoryImpl();


    /*!
     * @brief this method is used in order to create Model object of oll.
     * @return pointer to created Model object of oll.
     */
    virtual Model* create(int classifierMethod);
  };
}
