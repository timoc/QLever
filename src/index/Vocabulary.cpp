// Copyright 2011, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Björn Buchhold <buchholb>

#include <iostream>
#include <fstream>

#include "../util/File.h"
#include "./Vocabulary.h"
#include "../util/HashSet.h"
#include "../util/HashMap.h"

using std::string;

// _____________________________________________________________________________
Vocabulary::Vocabulary() {
}

// _____________________________________________________________________________
Vocabulary::~Vocabulary() {
}


// _____________________________________________________________________________
void Vocabulary::readFromFile(const string& fileName,
                              const string& extLitsFileName) {
  LOG(INFO) << "Reading vocabulary from file " << fileName << "\n";
  _words.clear();
  std::fstream in(fileName.c_str(), std::ios_base::in);
  string line;
  while (std::getline(in, line)) {
    _words.push_back(line);
  }
  in.close();
  LOG(INFO) << "Done reading vocabulary from file.\n";
  if (extLitsFileName.size() > 0) {
    LOG(INFO) << "Registering external vocabulary for literals.\n";
    _externalLiterals.initFromFile(extLitsFileName);
    LOG(INFO) << "Done registering external vocabulary for literals.\n";
  }
}

// _____________________________________________________________________________
void Vocabulary::writeToFile(const string& fileName) const {
  LOG(INFO) << "Writing vocabulary to file " << fileName << "\n";
  std::ofstream out(fileName.c_str(), std::ios_base::out);
  for (size_t i = 0; i + 1 < _words.size(); ++i) {
    out << _words[i] << '\n';
  }
  if (_words.size() > 0) {
    out << _words[_words.size() - 1];
  }
  out.close();
  LOG(INFO) << "Done writing vocabulary to file.\n";
}

// _____________________________________________________________________________
void Vocabulary::createFromSet(const ad_utility::HashSet<string>& set) {
  LOG(INFO) << "Creating vocabulary from set ...\n";
  _words.clear();
  _words.reserve(set.size());
  _words.insert(begin(_words), begin(set), end(set));
  LOG(INFO) << "... sorting ...\n";
  std::sort(begin(_words), end(_words));
  LOG(INFO) << "Done creating vocabulary.\n";
}

// _____________________________________________________________________________
google::sparse_hash_map<string, Id> Vocabulary::asMap() {
  google::sparse_hash_map<string, Id> map;
  for (size_t i = 0; i < _words.size(); ++i) {
    map[_words[i]] = i;
  }
  return map;
}

// _____________________________________________________________________________
void Vocabulary::externalizeLiterals(const string& fileName) {
  LOG(INFO) << "Externalizing literals..." << std::endl;
  auto ext = std::lower_bound(_words.begin(), _words.end(),
                              string({EXTERNALIZED_LITERALS_PREFIX}));
  size_t nofInternal = ext - _words.begin();
  vector<string> extVocab;
  while (ext != _words.end()) {
    extVocab.push_back((*ext++).substr(1));
  }
  _words.resize(nofInternal);
  _externalLiterals.buildFromVector(extVocab, fileName);
  LOG(INFO) << "Done externalizing literals." << std::endl;
}

// _____________________________________________________________________________
bool Vocabulary::shouldBeExternalized(const string& word) {
  if (word.size() > 100) { return true; }
  string lang = getLanguage(word);
  if (lang != "") {
    return (lang != "en"); // && lang != "en_gb" && lang != "en_us" &&
    // lang != "de" && lang != "es" && lang != "fr");
  }
  return false;
}

// _____________________________________________________________________________
string Vocabulary::getLanguage(const string& literal) {
  auto lioAt = literal.rfind('@');
  if (lioAt != string::npos) {
    auto lioQ = literal.rfind('\"');
    if (lioQ != string::npos && lioQ < lioAt) {
      return literal.substr(lioAt + 1);
    }
  }
  return "";
}

