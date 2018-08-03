#ifndef TIL_H
#define TIL_H

#include <map>
#include <deque>
#include <string>
#include <iostream>
#include <Rcpp.h>
using namespace Rcpp;

class TipSeqLinker {
public:
  TipSeqLinker(CharacterVector sequence, IntegerVector tipPath);
  void proceed();
  const std::vector<std::string> siteComp(const std::vector<int> &sites);
  const float compare(TipSeqLinker *linker);
  const int nextClade();
  const int currentClade();
  const int getTip();
  const int getRoot();
  const int getSeqLen();
  std::deque<int> getPath();
  std::deque<int> getFullPath();
private:
  CharacterVector seq;
  IntegerVector path;
  const int maxIndex;
  int pIndex;
};

class TreeAlignmentMatch {
public:
  TreeAlignmentMatch(
    ListOf<IntegerVector> tipPaths, 
    ListOf<CharacterVector> alignedSeqs
  );
  void setThreshold(const float sim);
  void setSites(IntegerVector rSites);
protected:
  bool pruned;
  float simCut;
  const int root, seqLen;
  std::vector<int> sites;
  std::vector<TipSeqLinker*> linkers;
  std::map< int, std::vector<TipSeqLinker*> > clusters;
  void pruneTree();
private:
  std::map<std::pair<int, int>, float> compared;
  const bool qualified(const std::vector<TipSeqLinker*> &clstr);
};

#endif