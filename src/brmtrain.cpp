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
      std::cerr << fileName << " : the utility to make a model from training data with broomie" << std::endl;
      std::cerr << "  " << fileName << " bayes [save_dir] [traing_file] [`class1', `class2', `class3']" << std::endl;
      std::cerr << "  " << std::endl;
      std::cerr << "  create model example:" << std::endl;
      std::cerr << "    " << fileName << " bayes /tmp/model/ ~/train/ex1.tsv sports it economic" << std::endl;
      std::cerr << "    - /tmp/train_data/\tdirectory path to save model and dbms. (need to create dir before run this mode)" << std::endl;
      std::cerr << "    - ~/train/ex1.tsv\ttraining data to making models." << std::endl;
      std::cerr << "    - sport it economic\tclasses(categories) include in training data. (classes must be known)" << std::endl;
      std::cerr << "    " << std::endl;
      std::cerr << "  format of [training_data] & [test_file]:" << std::endl;
      std::cerr << "    - class(category)['\\t']word_1['\\t']point_1['\\t']word_2['\\t']point_2['\\n']" << std::endl;
      std::cerr << "    -- class(string)" << std::endl;
      std::cerr << "    -- word(string)" << std::endl;
      std::cerr << "    -- point(double)" << std::endl;
      std::cerr << "  training data sample:" << std::endl;
      std::cerr << "      it\thoge\t0.1\tfuga\t 0.2\t hoo\t 0.32" << std::endl;
      std::cerr << "      sport\thoge\t0.32\tfuga\t0.19\thoo\t0.01" << std::endl;
      std::cerr << "      economic\thoge\t0.23\tfuga\t0.61\thoo\t0.77" << std::endl;
      std::cerr << "    " << std::endl;
      exit(EXIT_SUCCESS);
    }
  }
}

int main(int argc, char **argv)
{
  std::string fileName(argv[0]);
  if(argc < 6) broomie::train::printUsage(fileName);
  std::vector<std::string> arg(argc-1);
  for(int i = 0; i < argc - 1; i++){
    arg[i] = argv[i+1];
  }
  std::string method    = arg[0];
  std::string basePath  = arg[1];
  std::string trainFile = arg[2];
  if(!broomie::util::checkDir(basePath)){
    std::cerr << "error: [" << basePath << "] is not directory." << std::endl;
    return false;
  }
  if(!broomie::util::checkFile(trainFile)){
    std::cerr << "error: [" << trainFile << "] is not regular file." << std::endl;
    return false;
  }
  broomie::CList classNames;
  for(unsigned int i = 3; i < arg.size(); i++) {
    classNames.push_back(arg[i]);
  }
  if(method == METHOD_BAYES){ //algorithm
    if(!broomie::train::createTrain(basePath, trainFile, classNames,
                                    broomie::BAYES)) return false;
  } else {
    std::cerr << "error: unknown classifier method [" <<
      method << "] (`bayes' only)"
              << std::endl;
    return false;
  }
  return EXIT_SUCCESS;
}
