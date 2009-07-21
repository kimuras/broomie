#ifndef BROOMIE_BROOMIE_H
#define BROOMIE_BROOMIE_H

#include <cstring>
#include <vector>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <utility>
#include <map>
#include <cassert>
#include <fstream>

/* tokyocabinet library */
#include <tcutil.h>
#include <tchdb.h>
#include <tcbdb.h>
#include <tcfdb.h>
#include <tcadb.h>

namespace broomie {

  typedef std::vector<std::string> CList;
  typedef std::map<std::string, int> CMap;

  /* constant variables */
  const std::string DEFINE_BASE_DIR_NAME     = "BASEDIR";
  const std::string DEFINE_METHOD_NAME       = "METHOD";
  const std::string KEY_DIC_MAX_NUM          = "wordDicMaxNum";
  const std::string WORD_DIC_DB_NAME         = "wordDic.hdb";
  const std::string WORD_DIC_INVERSE_DB_NAME = "wordDicInverse.hdb";
  const std::string CLASS_LIST_DB_NAME       = "classList.hdb";

  /*!
    @brief Region for saving each classify result.
    - name : The string object of class name.
    - point : The point of classify result of the class.
   */
  struct Result {
    std::string name;
    double point;
  };

  enum {
    BROOMIESUCCESS,        /* success*/
    BROOMIEEDBOPEN,        /* db open error */
    BROOMIEEDBCLOSE,       /* db close error */
    BROOMIEELOADCLASSLIST, /* class list close error */
    BROOMIEECLASSNAME,     /* class name list close error */
    BROOMIEECREATEMODEL,   /* create model error */
    BROOMIEESAVEMODEL,     /* save model error */
    BROOMIEELOADMODEL,     /* close model error */
    BROOMIEEPUTTRAIN,      /* put training data to model error */
    BROOMIEECONSTRUCTOR,   /* constructor error */
    BROOMIEEPUTCLASSNAME,  /* error for putting class name to class name list object */
  };

  const char* codecErrMessage(int ecode);
  int compareResult(Result* a, Result* b);

  /*****************************************************************************
   * Documnet object
   ****************************************************************************/

  /*!
    @brief Feature object is used in order to save each feature element.
    - id : feature id. it is created automaticlally by this system.
    - point : a point of the feature.
   */
  struct Feature {
    uint64_t id;
    double point;
  };


  /*!
    @brief FeatureWord object is region which saves each feature element.
    - word : the string object of feature name.
    - point : a point of the feature.
   */
  struct FeatureWord {
    std::string word;
    double point;
  };


  /**
   * Class having FeatureWord object.
   * - Document object has feature elements which occur in document.
   */
  class Document {


  public:
    /*!
     @brief Constructor of Document object.
     including Feature object elements.
     @param fNum : Num of feature elements.
    */
    Document(int fNum);


    /*!
      @brief Destructor of Document object.
     */
    ~Document(){};

    /*!
      @brief This method is used in order to add new feature(piar of a word and a point) to the Document object.
      @param word : String object of feature name.
      @param point : The point of the feature.
      @attention If success sotre the feature, increment the featureNum of Document object.
    */
    void addFeature(const std::string word, double point);


    /*!
      @brief This method is used in order to get i-th Feature object(pair of a word and a point).
      of an element in a Document object.
      @param index : Specifies the index of the element.
      @param point : Pointer to the point.
      @return String object of each feature name.
      @attention If `index' is equal to or more than the number of elements,
      the return value is "".
    */
    const std::string getFeature(int index, double& point) const;


    /*!
      @brief This method is used in order to get number of feature in Document.
      @return Number of feature in Document.
    */
    int getFeatureNum() const;


  private:
    /*! @brief Number of feature in Document */
    int featureNum;

    /*! @brief Save buffer of each feature. */
    std::vector<FeatureWord> featureWord;

  };

  /*****************************************************************************
   * Model classes
   ****************************************************************************/

  /**
   * Model is abstract class of learning model.
   */
  class Model {

  public:
    /*!
      @brief Destructor of Model object.
    */
    virtual ~Model(){}


    /*!
      @brief This method is used in order to set directory path to saving model.
      @param path : Directory is must created befor set.
     */
    virtual void setPath(const std::string &path) = 0;


    /*!
      @brief This method is used in order add new features to model instance.
      @param features[] : Array of Feature objects.
      including feature name and point of it.
      @param num : Num of features.
      @param mode : Set the features `broomie::POSTIVE' sample or `broomie::NEGATIVE' sample.
      @return Return `true' if success, return `false' if fail.
      @attention Mode must be use enum object of broomie::POSITIVE or broomie::NEGATIVE.
     */
    virtual bool put(const Feature features[], int num, int mode) = 0;


    /*!
      @brief This method is used in order to store the learning model.
      @return Return `true' if success, return `false' if fail.
     */
    virtual bool save() = 0;


    /*!
      @brief This method is used in order to load the learning model.
      @return Return `true' if success, return `false' if fail.
     */
    virtual bool load() = 0;


    /*!
      @brief This method is used in order to classify each document.
      @param features[] : Test examples. Array of Feature objects.
      @param num : Num of features.
      @return : The point of result of classify.
     */
    virtual double classify(const Feature features[], int num) = 0;


  private:
    /*!
      @brief This method is used in order to get num of traing data in learning model.
      @return Num of training data.
     */
    virtual int getTrainingNum() = 0;
  };


  /**
   * Class in order to create model Object.
   */
  class ModelFactory {

  public:
    /*!
     * @brief Destructor of ModelFactory object.
     */
    virtual ~ModelFactory(){}


    /*!
     * @brief This method is used in order to create Model object.
     * @return Pointer to created Model object.
     * @attention Because the region of the return value is allocated with `new',
     * it should be released with the `delete' when it is no longer in use.
     */
    virtual Model* create(int classifierMethod) = 0;
  };


  /*****************************************************************************
   * Classifier Result classes
   ****************************************************************************/

  /*!
    @brief Region for saving all classify result.
    - results : Array of Result object.
    - num : Num of element in results(num of class).
   */
  class ResultSet {

  public:
    /*! @brief ResultSet object Constructor. */
    ResultSet(Result results[], int num);


    /*! @brief ResultSet object Destructor. */
    ~ResultSet();


    /*!
      @brief Accesor of num in ResultSet object.
      @return Num of element in results.
    */
    int getResultSetNum();


    /*!
      @brief This is used in order to get classify result of the index-th class.
      @param index : Specifies the index of the element.
      @param point : Specifies the value of the result of classify of the class.
      @return Class name
    */
    const std::string getResult(int index, float& point);

  private:
    /*! @brief Classify results of each class. */
    Result* results;

    /*! @brief Number of class. */
    int resultSetNum;
  };


  /*****************************************************************************
   * Classifier classes
   ****************************************************************************/

  /*!
    @brief Open mode of DBM.
    - READ : Read only.
    - CREATE : Truncate.
  */
  enum {
    READ = 0,
    CREATE = 1,
  };


  /*!
    @brief Setting of training data.
    - POSITIVE : Positive training data.
    - NEGATIVE : Negative training data.
   */
  enum {
    POSITIVE = 0,
    NEGATIVE = 1,
  };


  /*!
    @brief Mode for word dictionary.
    - LEARN : Open dictionary with learning mode.
    - CLASSIFY : Open dictionary with classify mode.
   */
  enum {
    LEARN = 0,
    CLASSIFY = 1,
  };


  /*!
    @brief Region of saving each class setting.
    - name : Name of class.
    - model : Learning model of the class.
  */
  struct ClassSetting {
    std::string name;
    Model *model;
  };


  /**
   * Class for classifying and learning.
   */
  class Classifier {

  public:
    /*!
      @brief Constructor of Classifier object.
      @param modelFactory : ModelFactory object.
      @param basePath : Base directory path for saving learning models.
    */
    Classifier(ModelFactory* modelFactory, std::string& basePath);


    /*! @brief Destructor of Classifier object. */
    ~Classifier();

    /*!
      @brief Check if something problem in process
      @return If problem is existing, return `true', not existing return `false'.
     */
    bool checkErr();


    /*!
      @brief Trace the error messages of a Classifier object if problem is occured.
      @return Error Messages it is occured in process of a Classifier object.
     */
    const std::string traceErr();


    /*!
      @brief Begin making learning model process.
      @param classNames : Vecotr objcet including Name of classes.
      @param classifierMethod : Classifier algotirhm (broomie::OLL or broomie::SVM)
      @return Return `true' if success, return `false' if fail.
     */
    bool beginMakingModel(const CList& classNames, const int classifierMethod);


    /*!
      @brief Finish making learning model.
      @return Return `true' if success, return `false' if fail.
    */
    bool endMakingModel();


    /*!
      @brief Add new traing example.
      @param className : ClassName of exmaple.
      @param doc : Document object including features.
      @return Return `true' if success, return `false' if fail.
    */
    bool addTrainingData(const std::string& className, const Document& doc);


    /*!
      @brief Convertor of word(string) to id(uint64_t).
      @param word : Name of the Feature.
      @param mode : broomie::LEARN or broomie::CLASSIFY.
      @return Word id of the Feature.
      @attention broomie::LEARN : Return word id if find in word database.
      Return (max id + 1) if not find.
      @attention broomie::CLASSIFY : Return word id if find in word database.
      Return 0 if not find.
    */
    uint64_t convertWordToId(const std::string& word, int mode);


    /*! @brief Accsessor for classList */
    CList* getClassList();


    /*!
      @brief Get the num of class setting.
      @return Number of class setting.
    */
    int getClassSettingNum();


    /*!
      @brief Begin classification mode.
      @param classifierMethod : Classifier method(broomie::OLL or broomie::SVM)
      @return Return `true' if success, return `false' if fail.
    */
    bool beginClassification(int classifierMethod);


    /*!
      @brief Finish classification mode.
      @return Return `true' if success, return `false' if fail.
    */
    bool endClassification();


    /*!
      @brief Classify the example
      @param doc : Document object including features.
      @attention Because the region of the return value is allocated with `new',
      it should be released with the 'delete' when it is no longer in use.
    */
    ResultSet *classify(const Document& doc);


  private:
    /*! @brief Model factory object. */
    ModelFactory* modelFactory;

    /*! @brief Base directory path.*/
    std::string basePath;

    /*! @brief Learning model path */
    std::string modelPath;

    /*! @brief Class settings object */
    ClassSetting *classSettings;

    /*! @brief Number of class types */
    int classSettingNum;

    /*! @brief Pointer of tokyocabinet hash database for word dictionary */
    TCHDB *wordDic;

    /*! @brief Pointer of tokyocabinet hash database for inverse word dictionary */
    TCHDB *wordDicInverse;

    /*! @brief Max word id in word dictionary */
    uint64_t wordDicMaxNum;

    /*! @brief Class name list */
    TCHDB *classList;

    /*! @brief Buffer of class name list */
    std::map<std::string, int> classListBuf;

    /*! @brief Iterator of class list */
    std::map<std::string, int>::iterator classListBufIter;

    /*! @brief Specifies the error messages occured in process*/
    std::vector<std::string> errMessage;

    /*!
      @brief Set the error code of a Classifier object.
      @param ecode : Specifies the error code.
      @param fileNmae : Specifies the file name of the code.
      @param line : Specifies the line number of the code.
      @param function : Specifies the function name of the code.
      @param otherMessage : Specifies the other message of the error.
     */
    void setErrMessage(int ecode, const char* fileName,
                       int line, const char* function, const char* otherMessage);

    void clearErrStack();

    /*!
      @brief Open word dictionary database.
      @param omode : Open mode.(broomie::REAR or broomie::CREATE)
      @return Return `true' if success, return `false' if fail.
    */
    bool openWordDic(int omode);

    /*!
      @brief Close word dictionary database.
      @return Return `true' is success, return `false' is fail.
    */
    bool closeWordDic();

    /*!
      @brief Open class list database.
      @param omode : Open mode.(broomie::REAR or broomie::CREATE)
      @return Return `true' if success, return `false' if fail.
    */
    bool openClassList(int omode);


    /*!
      @brief Close class list database.
      @return Return `true' is success, return `false' is fail.
    */
    bool closeClassList();

  };
}

#endif /* BROOMIE_BROOMIE_H */
// EOF
