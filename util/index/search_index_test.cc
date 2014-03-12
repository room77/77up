// Copyright 2011 Room77, Inc.
// Author: Uygar Oztekin

// Very basic usage example for search index.

#include <map>
#include "search_index.h"

FLAG_string(query, "pretty unique description", "Search query.");

struct Document {
  int id;
  string brief;
  string description;
};

vector<Document> documents = {
  { 1, "brief description for doc 1.", "This is the full description for document 1." },
  { 2, "brief description for doc 2.", "This is the full description for document 2." },
  { 5, "brief description for doc 5.", "This is the full description for document 5." },
  { 9, "brief description for doc 9.", "This is pretty unique content." },
};

int main() {
  cout << "Indexing starts" << endl;
  meta::SearchIndex index;
  vector<int> ids;
  for (const Document& document: documents) {
    ids.push_back(document.id);
    index.IndexBlob(document.id, "brief", document.brief);
    index.IndexBlob(document.id, "description", document.description);

    index.UpdateIdf(document.id, document.brief);
    index.UpdateIdf(document.id, document.description);
  }

  cout << "Indexing done" << endl;

  //cout << index.term_freq_.size() << endl;
  //cout << index.index_.size() << endl;
  index.DumpContents();

  cout << "\nIDFs of some terms:" << endl;
  cout << " is\t" << index.Idf("is") << endl;
  cout << " the\t" << index.Idf("the") << endl;
  cout << " unique\t" << index.Idf("uniqu") << endl;

  cout << "\nIssuing query: " << gFlag_query << endl;
  auto query = index.ProcessQuery(gFlag_query);
  auto matches = index.Search(ids, query);
  vector<string> terms = index.Tokenize(gFlag_query);
  for (int i = 0; i < matches.size(); ++i) {
    auto candidates = index.GenerateCandidateMatches(matches[i]);
    cout << "id: " << ids[i] << ", score: " << index.Score(candidates) << endl;
    if (matches[i].size())
      cout << index.GenerateSnippet(matches[i], terms) << endl << endl;
  }
  return 0;
}
