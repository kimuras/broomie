#include "brmutil.hpp"
#include "brmalgorithm.hpp"
#include <iterator>

const std::string CONFIG_NAME  = "broomie.conf";
const std::string METHOD_BAYES = "bayes";

namespace broomie {

  namespace train {
    bool createTrain(std::string basePath, std::string trainPath,
                     CList& classNames, int classifierMethod)
    {
      bool ok = true;
      std::ifstream ifs(trainPath.c_str());
      if(!ifs){
        std::cerr << "file open error: " << trainPath << std::endl;
        return false;
      }
      broomie::ModelFactoryImpl *modelFactory = new broomie::ModelFactoryImpl();
      broomie::Classifier cl(modelFactory, basePath);
      if(!cl.beginMakingModel(classNames, classifierMethod)){
        std::string errMessage = cl.traceErr();
        std::cerr << errMessage << std::endl;
        return false;
      }
      std::string confPath = basePath + CONFIG_NAME;
      std::ofstream ofs(confPath.c_str());
      ofs.write(DEFINE_METHOD_NAME.c_str(), std::strlen(DEFINE_METHOD_NAME.c_str()));
      ofs.write("\t", 1);
      if(classifierMethod == broomie::BAYES){
        ofs.write(METHOD_BAYES.c_str(), std::strlen(METHOD_BAYES.c_str()));
        ofs.write("\n", 1);
      }
      ofs.write(DEFINE_BASE_DIR_NAME.c_str(), std::strlen(DEFINE_BASE_DIR_NAME.c_str()));
      ofs.write("\t", 1);
      ofs.write(basePath.c_str(), std::strlen(basePath.c_str()));
      ofs.write("\n", 1);
      ofs.close();
      std::map<std::string, int> classNamesMap;
      for(unsigned int i = 0; i < classNames.size(); i++){
        classNamesMap.insert(std::make_pair(classNames[i], 1));
      }
      std::string line;
      int cnt = 0;
      std::cerr << "start reading training data\n";
      while(std::getline(ifs, line)){
        std::vector<std::string> features = broomie::util::split(line, "\t");
        broomie::Document *doc =
          new broomie::Document((features.size() - 1) / 2);
        std::string className;
        std::string feature;
        for(unsigned int i = 0; i < features.size(); i++){
          if(i < 1){
            className = features[i];
            if((classNamesMap.find(className)) == classNamesMap.end()){
              std::cerr << "check class name [" << className << "]" << std::endl;
            }
          } else if(i % 2 > 0){
            feature = features[i];
          } else {
            double point = atof(features[i].c_str());
            doc->addFeature(feature, point);
          }
        }
        if(!cl.addTrainingData(className, *doc)){
          ok = false;
          std::string errMessage = cl.traceErr();
          std::cerr << errMessage << std::endl;
        }
        delete doc;
        cnt++;
        if (cnt % 1000 == 0){
          std::cerr << " (" << cnt << ")" << std::endl;
        } else if(cnt % 100 == 0){
          std::cerr << ".";
        }
      }
      std::cerr << std::endl;
      ifs.close();
      std::cout << "start creating model\n";
      if(!cl.endMakingModel()){
        ok = false;
        std::string errMessage = cl.traceErr();
        std::cerr << errMessage << std::endl;
      }
      std::cout << "finish creating model\n";
      delete modelFactory;
      if(cl.checkErr()){
        std::string errMessage = cl.traceErr();
        std::cerr << errMessage << std::endl;
      }
      std::cout << "complete\n";
      return ok;
    }

    void printUsage(std::string fileName)
    {
      std::cerr << fileName
                << " : the utility to make a model from training data with broomie"
                << std::endl;
      std::cerr << std::endl;
      std::cerr << "  usage: " << fileName
                << " -s dir -t train [options] "
                << "[classes (class1 class2 class3 ... classN)]"
                << std::endl;
      std::cerr << "   e.g.: " << fileName
                << " -s dir/ -t train.tsv sports economic education"
                << std::endl;
      std::cerr << "    " << "-s, --save-dir=dir      "
                << "the save path for learning model."
                << std::endl;
      std::cerr << "    " << "-t, --train-data=train  "
                << "the file path of training data."
                << std::endl;
      std::cerr << "    -m, --method=method     "
                << "the choice of classifer. default classifier is `bayes'."
                <<std::endl;
      std::cerr << "    classes                 "
                << "the class names appear in training data. "
                << "give class names as ``space separated'' format."
                <<std::endl;
      std::cerr << std::endl;
      std::cerr << "for more information about this program, "
                << "please visit tutorial pages."
                << std::endl;
      std::cerr << "http://code.google.com/p/broomie/wiki/broomie_turorial_ja"
                << std::endl;
      std::cerr << std::endl;
      exit(EXIT_SUCCESS);
    }

    void printVersin()
    {
      std::string body;
      broomie::util::copyRight(body);
      std::cerr << body << std::endl;
      exit(EXIT_SUCCESS);
    }

    void procArgs(int argc, char** argv, std::string& fileName,
                  std::string& method, std::string& basePath,
                  std::string& trainPath, broomie::CList& classNames)
    {
      for(int i = 1; i < argc; i++){
        std::string argBuf = argv[i];
        if(argBuf == "-m"){
          method = argv[++i];
        } else if((argBuf.find("--method=")) != std::string::npos){
          unsigned int idx = argBuf.find("=");
          method = argv[i] + idx + 1;
        } else if(argBuf == "-s"){
          if(basePath.size() > 0) broomie::train::printUsage(fileName);
          basePath = argv[++i];
        } else if((argBuf.find("--save-dir=")) != std::string::npos){
          if(basePath.size() > 0) broomie::train::printUsage(fileName);
          unsigned int idx = argBuf.find("=");
          basePath = argv[i] + idx + 1;
        } else if(argBuf == "-t"){
          if(trainPath.size() > 0) broomie::train::printUsage(fileName);
          trainPath = argv[++i];
        } else if((argBuf.find("--train-data=")) != std::string::npos){
          if(trainPath.size() > 0) broomie::train::printUsage(fileName);
          unsigned int idx = argBuf.find("=");
          trainPath = argv[i] + idx + 1;
        } else if(argBuf == "--help" || argBuf == "-h"){
          broomie::train::printUsage(fileName);
        } else if(argBuf == "--version" || argBuf == "-v"){
          broomie::train::printVersin();
        } else {
          if(basePath.size() > 0 && trainPath.size() > 0)
            classNames.push_back(argv[i]);
        }
      }
      if(basePath.size() < 1 && trainPath.size() < 1)
        broomie::train::printUsage(fileName);
    }

  }
}

int main(int argc, char **argv)
{
  std::string fileName(argv[0]);
  std::string method = METHOD_BAYES;
  std::string basePath;
  std::string trainPath;
  broomie::CList classNames;
  broomie::train::procArgs(argc, argv,
                           fileName, method, basePath, trainPath, classNames);

  if(!broomie::util::checkDir(basePath)){
    std::cerr << "error: [" << basePath << "] is not directory." << std::endl;
    return false;
  }
  if(!broomie::util::checkFile(trainPath)){
    std::cerr << "error: [" << trainPath << "] is not regular file." << std::endl;
    return false;
  }

  if(method == METHOD_BAYES){ //algorithm
    if(!broomie::train::createTrain(basePath, trainPath, classNames,
                                    broomie::BAYES)) return false;
  } else {
    std::cerr << "error: unknown classifier method [" <<
      method << "] (`bayes' only)"
              << std::endl;
    return false;
  }

  return 0;
}
