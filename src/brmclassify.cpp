#include "brmutil.hpp"
#include "brmalgorithm.hpp"

const std::string CONFIG_NAME  = "broomie.conf";
const std::string METHOD_BAYES = "bayes";

namespace broomie {

  namespace classify {

    typedef std::map<std::string, std::map<std::string, int> > ClassResult;
    typedef std::map<std::string, double> ClassVal;

    enum {
      TEST   = 1,
      CLASSIFY = 2,
    };

    ClassVal* getRecall(ClassResult& accuracyBuf, CList* classList)
    {
      ClassResult::iterator itr;
      ClassVal* recall = new ClassVal;
      double sumRecall = 0.0;
      for(unsigned int i = 0; i < (*classList).size(); i++){
        double NumTrue = 0.0;
        double NumFalse = 0.0;
        for(unsigned int j = 0; j < (*classList).size(); j++){
          itr = accuracyBuf.find((*classList)[i]);
          if(itr->first == (*classList)[j]){
            NumTrue = static_cast<double>(itr->second[(*classList)[j]]);
          } else {
            NumFalse += static_cast<double>(itr->second[(*classList)[j]]);
          }
        }
        double recallBuf = NumTrue / (NumFalse + NumTrue);
        sumRecall += recallBuf;
        (*recall)[(*classList)[i]] = recallBuf;
      }
      (*recall)["RECALL_AVERAGE"] = (sumRecall / (*classList).size());

      return recall;
    }

    ClassVal* getPrecision(ClassResult& accuracyBuf, CList* classList)
    {
      ClassResult::iterator itr;
      ClassVal* precision = new ClassVal;
      ClassVal trueCnt;
      ClassVal falseCnt;
      for(unsigned int i = 0; i < (*classList).size(); i++){
        for(unsigned int j = 0; j < (*classList).size(); j++){
          itr = accuracyBuf.find((*classList)[i]);
          if(itr->first == (*classList)[j]){
            trueCnt[(*classList)[j]] =
              static_cast<double>(itr->second[(*classList)[j]]);
          } else {
            falseCnt[(*classList)[j]] +=
              static_cast<double>(itr->second[(*classList)[j]]);
          }
        }
      }
      double precisionAve = 0.0;
      for(unsigned int i = 0; i < (*classList).size(); i++){
        double numTrue = trueCnt[(*classList)[i]];
        double numFalse = falseCnt[(*classList)[i]];
        double precisionBuf = 0.0;
        if(numTrue > 0)
          precisionBuf= numTrue / (numFalse + numTrue);
        (*precision)[(*classList)[i]] = precisionBuf;
        precisionAve += precisionBuf;
      }
      (*precision)["PRECISION_AVERAGE"] = precisionAve / (*classList).size();

      return precision;
    }

    void printAccuracy(ClassVal* precision, ClassVal* recall,
                       ClassResult& accuracyBuf, CList* classList)
    {
      std::cerr << "=== Detailed Accuracy By Class ===" << std::endl;
      printf("\tpre\trec\tf-m\tclass\n");
      printf("---------------------------------------------\n");
      for(unsigned int i = 0; i < (*classList).size(); i++){
        double pre = (*precision)[(*classList)[i]];
        double rec = (*recall)[(*classList)[i]];
        double fMeasure = 0.0;
        if(!pre) pre = 0.0;
        if(pre > 0 && rec > 0) {
          fMeasure = (2 * (pre * rec)) / (pre + rec);
        }
        printf("\t%.3f\t%.3f\t%.3f\t%s\n", pre, rec,
               fMeasure, (*classList)[i].c_str());
      }
      double preAve = (*precision)["PRECISION_AVERAGE"];
      double recAve = (*recall)["RECALL_AVERAGE"];
      double aveFmeasure = 0.0;
      if(preAve > 0 && recAve > 0){
        aveFmeasure = (2 * (preAve * recAve)) / (preAve + recAve);
      }
      printf("---------------------------------------------\n");
      printf("Avg.\t%.3f\t%.3f\t%.3f\n\n", preAve, recAve, aveFmeasure);

      printf("=== Confusion Matrix ===\n");
      for(unsigned int i = 0; i < (*classList).size(); i++){
        printf("%d\t", i);
      }
      ClassResult::iterator itr;
      printf("<-- classified as\n");
      printf("---------------------------------------------\n");
      for(unsigned int i = 0; i < (*classList).size(); i++){
        for(unsigned int j = 0; j < (*classList).size(); j++){
          itr = accuracyBuf.find((*classList)[i]);
          printf("%d\t", itr->second[(*classList)[j]]);
        }
        printf("| %d = %s\n", i, (*classList)[i].c_str());
      }
      printf("\n");
    }

    void printSummary(int collectNum, int numTest)
    {
      std::cout << "=== classify result ===" << std::endl;
      printf("Correctly Classified Instance:\t%d\t%.3f\n", collectNum,
             (static_cast<double>(collectNum)/numTest));
      printf("Number of test examples:\t%d\n\n", numTest);
    }

    bool classifyTestData(std::string basePath, std::string testPath, int mode){
      bool ok = true;
      std::ifstream ifs(testPath.c_str(), std::ios::in);
      if(!ifs){
        std::cerr << "file open error: " << testPath << std::endl;
        return false;
      }
      broomie::ModelFactoryImpl *modelFactory = new broomie::ModelFactoryImpl();
      broomie::Classifier cl(modelFactory, basePath);
      std::string confPath = basePath + CONFIG_NAME;
      std::ifstream confifs(confPath.c_str(), std::ios::in);
      if(!confifs){
        std::cerr << "file open error: " << confPath << std::endl;
        return false;
      }
      int classifierMethod = 0;
      std::string line;
      while(std::getline(confifs, line)){
        std::vector<std::string> features = broomie::util::split(line, "\t");
        if(features[0] == DEFINE_METHOD_NAME){
          if(features[1] == METHOD_BAYES){ // algorithm
            classifierMethod = broomie::BAYES;
          } else {
            std::cerr << "chose classifier method error: "
                      << features[1] <<  std::endl;
            return false;
          }
        }
      }
      if(!cl.beginClassification(classifierMethod)){
        std::string errMessage = cl.traceErr();
        std::cerr << errMessage << std::endl;
        return false;
      }

      CList* classList = cl.getClassList();
      CMap classMap;
      for(unsigned int i = 0; i < (*classList).size(); i++){
        classMap[(*classList)[i]] = 1;
      }

      int cnt = 0;
      int collectNum = 0;
      ClassResult accuracyBuf;
      while(std::getline(ifs, line)){
        std::vector<std::string> features = broomie::util::split(line, "\t");
        broomie::Document *doc =
          new broomie::Document((features.size() - 1) / 2);
        std::string docAttr("");
        std::string feature("");
        for(unsigned int i = 0; i < features.size(); i++){
          if(i < 1){
            docAttr = features[i];
            if(mode == TEST){
              if(classMap.find(docAttr) == classMap.end()){
                std::cerr << "unknown class:[" << docAttr << "]" << std::endl;
                std::cerr << "check class names of test data or traning data"
                          << std::endl;
                return false;
              }
              std::cout << "correct answer:" << docAttr << std::endl;
            } else if(mode == CLASSIFY){
              std::cout << docAttr;
            }
          } else if(i % 2 > 0){
            feature = features[i];
          } else {
            double point = atof(features[i].c_str());
            doc->addFeature(feature, point);
          }
        }
        broomie::ResultSet *rs = cl.classify(*doc);
        float maxPoint = 0.0;
        std::string maxPointClass("");
        for(int i = 0; i < rs->getResultSetNum(); i++){
          float point;
          std::string className = rs->getResult(i, point);
          if(i == 0){
            accuracyBuf[docAttr][className] += 1;
            maxPoint = point;
            maxPointClass = className;
          }
          if(mode == TEST){
            std::cout << "  " << className << ":" << point << std::endl;
          } else if(mode == CLASSIFY){
            std::cout << "\t" << className << "\t" << point;
          }
        }

        if(mode == TEST){
          std::cout << "system answer:" << maxPointClass + ':'
                    << maxPoint << std::endl;
          if(maxPointClass == docAttr) {
            std::cout << "==> true" << std::endl;
            std::cout << std::endl;
            collectNum++;
          } else {
            std::cout << "==> false" << std::endl;
            std::cout << line << std::endl;
            std::cout << "---------" << std::endl;
            std::cout << std::endl;
          }
        } else if(mode == CLASSIFY){
          std::cout << std::endl;
        }
        delete rs;
        delete doc;
        cnt++;
      }

      if(mode == TEST){
        ClassVal* precision = getPrecision(accuracyBuf, classList);
        ClassVal* recall = getRecall(accuracyBuf, classList);
        printSummary(collectNum, cnt);
        printAccuracy(precision, recall, accuracyBuf, classList);
        delete recall;
        delete precision;
      }
      ifs.close();
      if(!cl.endClassification()){
        ok = false;
        std::string errMessage = cl.traceErr();
        std::cerr << errMessage << std::endl;
      }
      delete modelFactory;
      if(cl.checkErr()){
        std::string errMessage = cl.traceErr();
        std::cout << errMessage << std::endl;
      }
      delete classList;
      return ok;
    }

    void printUsage(std::string fileName)
    {
      std::cerr << fileName << " : the utility to classify example with broomie" << std::endl;
      std::cerr << "  " << fileName << " accuracy  [model_dir] [test_date(labbeled)]" << std::endl;
      std::cerr << "  " << fileName << " classify  [model_dir] [test_date(no label)]" << std::endl;
      std::cerr << "  " << std::endl;
      std::cerr << "  classify example with `accuracy' mode:" << std::endl;
      std::cerr << "    " << fileName << " test /tmp/model/ /tmp/test_data.tsv" << std::endl;
      std::cerr << "    - test\t classify mode. when given `accuracy' test data needs correct class to each line." << std::endl;
      std::cerr << "    - /tmp/model/\t path to saved model." << std::endl;
      std::cerr << "    - /tmp/test_data.tsv\t test data." << std::endl;
      std::cerr << std::endl;
      std::cerr << "  format of test data with `accuracy' mode:" << std::endl;
      std::cerr << "    - class(category)['\\t']word_1['\\t']point_1['\\t']word_2['\\t']point_2['\\n']" << std::endl;
      std::cerr << "      it\thoge\t0.1\tfuga\t0.2\thoo\t0.32" << std::endl;
      std::cerr << "      sport\thoge\t0.32\tfuga\t0.19\thoo\t0.01" << std::endl;
      std::cerr << std::endl;
      std::cerr << "  format of test data without `accuracy' mode:" << std::endl;
      std::cerr << "    - word_1['\\t']point_1['\\t']word_2['\\t']point_2['\\n']" << std::endl;
      std::cerr << "      hoge\t0.1\tfuga\t0.2\thoo\t0.32" << std::endl;
      std::cerr << "      hoge\t0.32\tfuga\t0.19\thoo\t0.01" << std::endl;
      exit(0);
    }
  }
}

int main(int argc, char **argv)
{
  std::string fileName(argv[0]);
  if(argc < 3) broomie::classify::printUsage(fileName);
  std::vector<std::string> arg(argc-1);
  for(int i = 0; i < argc - 1; i++){
    arg[i] = argv[i+1];
  }
  int mode = broomie::classify::TEST;
  if(arg[0] == "accuracy"){
    mode = broomie::classify::TEST;
  } else if(arg[0] == "classify"){
    mode = broomie::classify::CLASSIFY;
  } else {
    broomie::classify::printUsage(fileName);
  }
  std::string basePath = arg[1];
  std::string testData = arg[2];
  if(!broomie::util::checkDir(basePath)){
    std::cerr << "error: [" << basePath << "] is not directory." << std::endl;
    return false;
  }
  if(!broomie::util::checkFile(testData)){
    std::cerr << "error: [" << testData << "] is not regular file." << std::endl;
    return false;
  }
  std::cout << basePath << "\t" << testData << "\t" << mode << std::endl;
  broomie::classify::classifyTestData(basePath, testData, mode);
  return 0;
}
