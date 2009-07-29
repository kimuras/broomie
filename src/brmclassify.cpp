#include "brmutil.hpp"
#include "brmalgorithm.hpp"
#include <iomanip>

const std::string CONFIG_NAME  = "broomie.conf";
const std::string METHOD_BAYES = "bayes";

namespace broomie {

  namespace classify {

    typedef std::map<std::string, std::map<std::string, int> > ClassResult;
    typedef std::map<std::string, double> ClassVal;

    enum {
      TEST     = 1,
      CLASSIFY = 2,
    };

    enum {
      DEFAULT = 1,
      EASY    = 2,
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
      std::cout.precision(3);
      std::cout << "=== Detailed Accuracy By Class ===" << std::endl;
      std::cout << "\tpre\trec\tf-m\tclass" << std::endl;
      std::cout << "---------------------------------------------" << std::endl;
      for(unsigned int i = 0; i < (*classList).size(); i++){
        double pre = (*precision)[(*classList)[i]];
        double rec = (*recall)[(*classList)[i]];
        double fMeasure = 0.0;
        if(!pre) pre = 0.0;
        if(pre > 0 && rec > 0) {
          fMeasure = (2 * (pre * rec)) / (pre + rec);
        }
        std::cout << "\t" << pre << "\t" << rec << "\t"  << fMeasure << "\t"
                  << (*classList)[i].c_str() << std::endl;
      }
      double preAve = (*precision)["PRECISION_AVERAGE"];
      double recAve = (*recall)["RECALL_AVERAGE"];
      double aveFmeasure = 0.0;
      if(preAve > 0 && recAve > 0){
        aveFmeasure = (2 * (preAve * recAve)) / (preAve + recAve);
      }
      std::cout << "---------------------------------------------" << std::endl;
      std::cout << "Avg.\t" << preAve << "\t" << recAve << "\t"
                << aveFmeasure << std::endl;
      std::cout << std::endl;
      std::cout << "=== Confusion Matrix ===" << std::endl;
      for(unsigned int i = 0; i < (*classList).size(); i++){
        std::cout << i << "\t";
      }
      ClassResult::iterator itr;
      std::cout << "<-- classified as" << std::endl;
      std::cout << "---------------------------------------------" << std::endl;
      for(unsigned int i = 0; i < (*classList).size(); i++){
        for(unsigned int j = 0; j < (*classList).size(); j++){
          itr = accuracyBuf.find((*classList)[i]);
          std::cout << itr->second[(*classList)[j]] << "\t";
        }
        std::cout << "| " << i << " = " << (*classList)[i].c_str() << std::endl;
      }
      std::cout << std::endl;
    }

    void printSummary(int collectNum, int numTest)
    {
      std::cout.precision(3);
      std::cout << "=== classify result ===" << std::endl;
      std::cout << "Correctly Classified Instance:\t" << collectNum << "\t"
                << (static_cast<double>(collectNum)/numTest) << std::endl;
      std::cout << "Number of test examples:\t" << numTest << std::endl;
      std::cout << std::endl;
    }

    bool classifyTestData(std::string basePath, std::string testPath,
                          int clmode, int imode)
    {
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
      segmenter::Segmenter sg;
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
        std::vector<std::string> features;
        if(imode & broomie::classify::EASY){
          broomie::util::convertbrmFormat(sg, line, features);
        } else {
          features = broomie::util::split(line, "\t");
        }
        broomie::Document *doc =
          new broomie::Document((features.size() - 1) / 2);
        std::string docAttr("");
        std::string feature("");
        for(unsigned int i = 0; i < features.size(); i++){
          if(i < 1){
            docAttr = features[i];
            if(clmode & TEST){
              if(classMap.find(docAttr) == classMap.end()){
                std::cerr << "unknown class:[" << docAttr << "]" << std::endl;
                std::cerr << "check class names of test data or traning data"
                          << std::endl;
                return false;
              }
              std::cout << "correct answer:" << docAttr << std::endl;
            } else if(clmode & CLASSIFY){
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
          if(clmode & TEST){
            std::cout << "  " << className << ":" << point << std::endl;
          } else if(clmode & CLASSIFY){
            std::cout << "\t" << className << "\t" << point;
          }
        }

        if(clmode & TEST){
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
        } else if(clmode & CLASSIFY){
          std::cout << std::endl;
        }
        delete rs;
        delete doc;
        cnt++;
      }

      if(clmode & TEST){
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
      std::cerr << fileName
                << " : the utility to classify example with broomie"
                << std::endl;
      std::cerr << std::endl;
      std::cerr << "  usage: " << fileName
                << " [accuracy|classify] -m dir -t test_data" << std::endl;
      std::cerr << "  " << fileName
                << " accuracy -m dir -t test_data(labbeled)" << std::endl;
      std::cerr << "  " << fileName
                << " classify -m dir -t test_data(unlabbeled)" << std::endl;
      std::cerr << "    " << "accuracy                 "
                << "check the accuracy of the classifier with class "
                << "name given test examaples."
                << std::endl;
      std::cerr << "    " << "classify                 "
                << "classify test examaples not given class names."
                << std::endl;
      std::cerr << "    " << "-m, --model-dir=dir      "
                << "the path for learning model."
                << std::endl;
      std::cerr << "    " << "-t, --test-datad=test    "
                << "test examples, classifing data."
                << std::endl;
      std::cerr << std::endl;
      std::cerr << "for more information about this program, "
                << "please visit tutorial pages."
                << std::endl;
      std::cerr << "http://code.google.com/p/broomie/wiki/broomie_turorial_ja"
                << std::endl;
      std::cerr << std::endl;
      exit(EXIT_SUCCESS);
    }

    void printVersion()
    {
      std::string body;
      broomie::util::copyRight(body);
      std::cerr << body << std::endl;
      exit(EXIT_SUCCESS);
    }


    void procArgs(int argc, char** argv, int& clmode, int& imode, std::string& basePath, std::string& testPath)
    {
      std::string fileName = argv[0];
      std::string argBuf   = argv[1];

      if(argBuf == "accuracy"){
        clmode = broomie::classify::TEST;
      } else if(argBuf == "classify"){
        clmode = broomie::classify::CLASSIFY;
      } else if(argBuf == "-h" || argBuf == "--help"){
        broomie::classify::printUsage(fileName);
      } else if(argBuf == "-v" || argBuf == "--version"){
        broomie::classify::printVersion();
      } else {
        broomie::classify::printUsage(fileName);
      }
      if(argc < 4) broomie::classify::printUsage(fileName);

      for(int i = 2; i < argc; i++){
        argBuf = argv[i];
        if(argBuf == "-m"){
          if(basePath.size() > 0) broomie::classify::printUsage(fileName);
          basePath = argv[++i];
        } else if((argBuf.find("--model-dir=")) != std::string::npos){
          if(basePath.size() > 0) broomie::classify::printUsage(fileName);
          unsigned int idx = argBuf.find("=");
          basePath = argv[i] + idx + 1;
        } else if(argBuf == "-t"){
          if(testPath.size() > 0) broomie::classify::printUsage(fileName);
          testPath = argv[++i];
        } else if((argBuf.find("--test-data=")) != std::string::npos){
          if(testPath.size() > 0) broomie::classify::printUsage(fileName);
          unsigned int idx = argBuf.find("=");
          testPath = argv[i] + idx + 1;
        } else if(argBuf == "-e" || argBuf == "--easy"){
          imode = broomie::classify::EASY;
        } else {
          broomie::classify::printUsage(fileName);
        }
      }
      if(testPath.size() < 1 || basePath.size() < 1)
        broomie::classify::printUsage(fileName);
    }

  }
}

int main(int argc, char **argv)
{
  if(argc < 2) broomie::classify::printUsage(argv[0]);
  std::string basePath;
  std::string testPath;
  int clmode = broomie::classify::TEST;
  int imode  = broomie::classify::DEFAULT;
  broomie::classify::procArgs(argc, argv, clmode, imode, basePath, testPath);

  if(!broomie::util::checkDir(basePath)){
    std::cerr << "error: [" << basePath << "] is not directory." << std::endl;
    return false;
  }
  if(!broomie::util::checkFile(testPath)){
    std::cerr << "error: [" << testPath << "] is not regular file." << std::endl;
    return false;
  }
  std::cout << basePath << "\t" << testPath << "\t" << clmode<< std::endl;
  broomie::classify::classifyTestData(basePath, testPath, clmode, imode);

  return 0;
}
