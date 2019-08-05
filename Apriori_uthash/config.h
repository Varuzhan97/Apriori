#include <string.h>
#include <iostream>
using namespace std;
#include <fstream>
#include <algorithm>


namespace configurations
{
  string database_name;
  uint32_t minimum_support;
  float minimum_confidence;
}

void detect_description(string description, string argument)
{
  if(description == "databasename")
  {
    configurations::database_name = argument;
    return;
  }
  if(description == "minimumsupport")
  {
    assert(stoi(argument)>=0);
    configurations::minimum_support = stoi(argument);
    return;
  }
  if(description == "minimumconfidence")
  {
    assert(stof(argument)>=0);
    configurations::minimum_confidence = stof(argument);
    return;
  }
}

void is_config_valid()
{
  std::ifstream config_file("config.cfg");
  std::string line;
  if(!config_file.is_open()) {
    std::cout << "Can not open config file." << '\n';
    exit(1);
  }
  while (getline(config_file, line)) {
      int32_t pos = line.find(":");
      assert(pos != string::npos);
      string argument = line.substr(pos + 1);
      string description = line.substr(0, pos - 1);
      transform(argument.begin(), argument.end(), argument.begin(), ::tolower);
      transform(description.begin(), description.end(), description.begin(), ::tolower);
      argument.erase(remove(argument.begin(), argument.end(), ' '), argument.end());
      description.erase(remove(description.begin(), description.end(), ' '), description.end());
      assert((description=="databasename") || (description=="minimumsupport") || (description=="minimumconfidence"));
      detect_description(description, argument);
  }
  config_file.close();
}
