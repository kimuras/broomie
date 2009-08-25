/*
 * Copyright (C) 2009 Shunya Kimura <brmtrain@gmail.com>
 * All Rights Reserved.
 *
 * Use and distribution of this program is licensed under the
 * BSD license. See the COPYING file for full text.
 */

#include <math.h>
#include <iterator>
#include <fcgi_stdio.h>
#include "brmalgorithm.hpp"
#include "brmutil.hpp"

const std::string INTERFACE_CONFIG_NAME = "./broomie.conf";
const std::string METHOD_BAYES = "bayes";

class CGIManager {

  private:
  std::string basePath;
  std::string confPath;
  int method;
  broomie::Classifier* classifier;
  void readParameters(){}
  enum {
    FCGLIFETIME = 10000,
    MINIBNUM    = 31,
    UPLOADMAX   = (256*1024*1024),
  };

public:
  CGIManager(std::string configName) : confPath(configName), classifier(){}

  ~CGIManager()
  {
    delete classifier;
  }

  bool readConfig()
  {
    bool ok = true;
    std::ifstream ifs(confPath.c_str(), std::ios::in);
    if(!ifs) return false;
    std::string line;
    while(std::getline(ifs, line)){
      std::vector<std::string> features = broomie::util::split(line, "\t");
      if(features[0] == broomie::DEFINE_METHOD_NAME){
        if(features[1] == METHOD_BAYES){ // algorithm
          method = broomie::BAYES;
        } else {
          return false;
        }
      } else if(features[0] == broomie::DEFINE_BASE_DIR_NAME.c_str()){
        basePath = features[1];
      }
    }
    return ok;
  }

  const unsigned int getLifeTime()
  {
    return FCGLIFETIME;
  }

  int accept()
  {
    return FCGI_Accept();
  }

  bool openClassifier()
  {
    bool ok = true;
    broomie::ModelFactoryImpl* modelFactory = new broomie::ModelFactoryImpl();
    classifier = new broomie::Classifier(modelFactory, basePath);
    ok = classifier->beginClassification(this->method);
    delete modelFactory;
    return ok;
  }

  bool closeClassifier()
  {
    bool ok = true;
    ok = classifier->endClassification();
    return ok;
  }

  /* dispatch the acctual process by mode */
  void dispatch()
  {
    TCMAP* params = tcmapnew2(MINIBNUM);
    readparameters(params);
    std::string text = tcmapget2(params, "text");
    std::vector<std::string>features = broomie::util::split(text, "\t"); // tcsplit
    broomie::Document doc(features.size() / 2);
    std::string className;
    std::string feature;
    for(unsigned int i = 0; i < features.size(); i++){
      if(i % 2 == 0){
        feature = features[i];
      } else {
        double point = atof(features[i].c_str());
        doc.addFeature(feature, point);
      }
    }
    broomie::ResultSet* rs = classifier->classify(doc);
    for(int i = 0; i < rs->getResultSetNum(); i++){
      float point;
      std::string className = rs->getResult(i, point);
      printf("%s:%f\n", className.c_str(), point);
    }
    delete rs;
    tcmapdel(params);
  }

  /* read CGI parameters */
  void readparameters(TCMAP* params)
  {
    char* buf  = NULL;
    int len    = 0;
    const char* rp;
    if((rp = getenv("REQUEST_METHOD")) != NULL && !strcmp(rp, "POST") &&
       (rp = getenv("CONTENT_LENGTH")) != NULL && (len = atoi(rp)) > 0){
      if(len > UPLOADMAX) len = UPLOADMAX;
      buf = static_cast<char*>(tccalloc(len + 1, 1));
      if(static_cast<int>(fread(buf, 1, len, stdin)) != len){
        tcfree(buf);
        buf = NULL;
      }
    } else if((rp = getenv("QUERY_STRING")) != NULL){
      buf = tcstrdup(rp);
      len = strlen(buf);
    }
    if(buf && len > 0){
      if((rp = getenv("CONTENT_TYPE")) != NULL && tcstrfwm(rp, "multipart/form-data") &&
         (rp = strstr(rp, "boundary=")) != NULL){
        rp += 9;
        if(*rp == '"') rp++;
        char bstr[strlen(rp)+1];
        strcpy(bstr, rp);
        char* wp = strchr(bstr, ';');
        if(wp) *wp = '\0';
        wp = strchr(bstr, '"');
        if(wp) *wp = '\0';
        TCLIST* parts = tcmimeparts(buf, len, bstr);
        int pnum      = tclistnum(parts);
        for(int i = 0; i < pnum; i++){
          int psiz;
          const char* part = static_cast<const char*>(tclistval(parts, i, &psiz));
          TCMAP* hmap = tcmapnew2(MINIBNUM);
          int bsiz;
          char* body = tcmimebreak(part, psiz, hmap, &bsiz);
          int nsiz;
          const char* name = static_cast<const char*>(tcmapget(hmap, "NAME", 4, &nsiz));
          if(name){
            tcmapput(params, name, nsiz, body, bsiz);
            const char* fname = tcmapget2(hmap, "FILENAME");
            if(fname){
              if(*fname == '/'){
                fname = strrchr(fname, '/') + 1;
              } else if(((*fname >= 'a' && *fname <= 'z') || (*fname >= 'A' && *fname <= 'Z')) &&
                        fname[1] == ':' && fname[2] == '\\'){
                fname = strrchr(fname, '\\') + 1;
              }
              if(*fname != '\0'){
                char key[nsiz+10];
                sprintf(key, "%s_filename", name);
                tcmapput2(params, key, fname);
              }
            }
          }
          tcfree(body);
          tcmapdel(hmap);
        }
        tclistdel(parts);
      } else {
        TCLIST* pairs = tcstrsplit(buf, "&");
        int num = tclistnum(pairs);
        for(int i = 0; i < num; i++){
          char* key = tcstrdup(tclistval2(pairs, i));
          char* val = strchr(key, '=');
          if(val){
            *(val++)   = '\0';
            char* dkey = tcurldecode(key, &len);
            char* dval = tcurldecode(val, &len);
            tcmapput2(params, dkey, dval);
            tcfree(dval);
            tcfree(dkey);
          }
          tcfree(key);
        }
        tclistdel(pairs);
      }
    }
    tcfree(buf);
  }
};


int main(int argc, char** argv)
{
  bool ok = true;
  unsigned int cnt = 0;
  CGIManager cgiMgr(INTERFACE_CONFIG_NAME);
  cgiMgr.readConfig();
  ok = cgiMgr.openClassifier();
  while(cgiMgr.accept() >= 0){
    cnt++;
    printf("Content-Type: text/plain; charset=UTF-8\r\n");
    printf("X-FCGI-Count: %d\r\n", cnt);
    printf("\r\n");
    cgiMgr.dispatch();
    fflush(stdout);
    if(cnt >= cgiMgr.getLifeTime()) break;
  }
  ok = cgiMgr.closeClassifier();

  return 0;
}

