#include <OpenANN/io/LibSVM.h>
#include <fstream>
#include <string>
#include <set>
#include <vector>
#include <algorithm>

namespace OpenANN {
namespace LibSVM {

int load(Eigen::MatrixXd& in, Eigen::MatrixXd& out, const char* filename)
{
  std::ifstream fin(filename, std::ios_base::in);
  int count = load(in, out, fin);
  fin.close();

  return count;
}


int load(Eigen::MatrixXd& in, Eigen::MatrixXd& out, std::istream& stream)
{
  std::vector<std::string> features;
  std::vector<int> targets;
  std::set<int> classes;
  int maximum_features = 0;
  int instances = 0;

  // preprocessing for retrieving all necessary information
  while(stream.good()) {
    std::vector<std::string> line;
    std::string buffer, token;
    std::getline(stream, buffer);
    
    std::stringstream ss(buffer);
    while(ss >> token) {
      if(token.size() > 0)
        line.push_back(token);
    }

    if(line.size() > 0) {
      std::string& classStr = line.front();
      std::string& last = line.back();

      int y = std::atoi(classStr.c_str());
      classes.insert(y);
      targets.push_back(y);

      maximum_features = 
        std::max(
            maximum_features, 
            std::atoi(last.substr(0, last.find_first_of(':')).c_str())
        );

      std::copy(line.begin() + 1, line.end(), std::back_inserter(features));
      features.push_back("");
      instances++;
    }
  }

  // prepare matrices
  in.resize(instances, maximum_features);
  
  if(classes.size() > 2)
    out.resize(instances, classes.size());
  else
    out.resize(instances, 1);
  
  in.setZero();
  out.setZero();

  // setup output matrix from collected targets
  for(int i = 0; i < targets.size(); ++i) {
    if(classes.size() > 2) {
      out(i, targets[i] - 1) = 1.0;
    } else {
      out(i, 0) = targets.at(i);
    }
  }

  // setup input matrix from collected tokens
  for(int i = 0, row = 0; i < features.size(); ++i) {
    std::string& tok = features.at(i);

    if(!tok.empty()) {
      size_t pos = tok.find_first_of(':');
      int index = std::atoi(tok.substr(0, pos).c_str());
      double value = std::atof(tok.substr(pos + 1).c_str());

      // index counting is different between supervised- and unsupervised 
      // datasets
      if(classes.size() > 1) 
        index--;

      in(row, index) = value;
    } else {
      row++;
    }
  }

  return instances;
}


void save(const Eigen::MatrixXd& in, const Eigen::MatrixXd& out, const char* filename)
{
  std::ofstream fout(filename, std::ios_base::out);
  save(in, out, fout);
  fout.close();
}


void save(const Eigen::MatrixXd& in, const Eigen::MatrixXd& out, std::ostream& stream)
{
}



}} // namespace OpenANN::LibSVM
