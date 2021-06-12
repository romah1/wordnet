#include "wordnet.h"

#include <utility>

//Digraph Class
Digraph::Digraph()
{
}
void Digraph::addNode()
{
    adjacencyList.emplace_back();
}

void Digraph::addEdge(int from, int to)
{
    adjacencyList[from].push_back(to);
}

std::size_t Digraph::size() const
{
    return adjacencyList.size();
}
const std::vector<int> & Digraph::operator[](size_t idx) const
{
    return adjacencyList[idx];
}
//-------------

// ShortestCommonAncestor Class
ShortestCommonAncestor::ShortestCommonAncestor(Digraph & dg)
    : digraph(dg)
{
}

std::vector<int> ShortestCommonAncestor::bfs(int node) const
{
    std::deque<int> q;
    std::vector<bool> used(digraph.size(), false);
    std::vector<int> distance(digraph.size(), -1);
    distance[node] = 0;
    used[node] = true;
    q.push_back(node);

    while (!q.empty()) {
        int v = q.front();
        q.pop_front();
        auto children = digraph[v];
        for (const auto & child : children) {
            if (!used[child]) {
                used[child] = true;
                q.push_back(child);
                distance[child] = distance[v] + 1;
            }
        }
    }
    return distance;
}

int ShortestCommonAncestor::ancestor(int v, int w)
{
    auto dv = bfs(v);
    auto dw = bfs(w);
    int min_dist = digraph.size();
    int ancestor = digraph.size();
    for (std::size_t i = 0; i < digraph.size(); ++i) {
        int dist = dv[i] + dw[i];
        if (dv[i] != -1 && dw[i] != -1 && dist < min_dist) {
            min_dist = dist;
            ancestor = i;
        }
    }
    return ancestor;
}
int ShortestCommonAncestor::length(int v, int w)
{
    auto dv = bfs(v);
    auto dw = bfs(w);
    int sca = ancestor(v, w);
    return dv[sca] + dw[sca];
}
int ShortestCommonAncestor::length_subset(const std::set<int> & subset_a, const std::set<int> & subset_b)
{
    int min_length = digraph.size();
    for (const auto & ai : subset_a) {
        for (const auto & bi : subset_b) {
            int len = length(ai, bi);
            if (len < min_length) {
                min_length = len;
            }
        }
    }
    return min_length;
}
int ShortestCommonAncestor::ancestor_subset(const std::set<int> & subset_a, const std::set<int> & subset_b)
{
    int min_length = digraph.size();
    int sca = digraph.size();
    for (const auto & ai : subset_a) {
        for (const auto & bi : subset_b) {
            int len = length(ai, bi);
            if (len < min_length) {
                min_length = len;
                sca = ancestor(ai, bi);
            }
        }
    }
    return sca;
}

//------------

// WordNet Class
WordNet::Synset::Synset(const std::vector<std::reference_wrapper<const std::string>> & synset, const std::string & gloss)
    : synset(synset)
    , gloss(gloss)
{
}

void WordNet::read_synsets_fn(const std::string & synsets_fn)
{
    std::ifstream synsets_file(synsets_fn);
    if (!synsets_file.is_open()) {
        throw std::ifstream::failure("Error while opening the file with synsets");
    }
    std::string line;
    while (std::getline(synsets_file, line)) {
        digraph.addNode();
        std::istringstream s(line);
        std::string field;
        std::getline(s, field, ','); // id
        int id = std::stoi(field);
        std::getline(s, field, ','); // synset
        std::istringstream words_str(field);
        std::string synonym;
        std::vector<std::reference_wrapper<const std::string>> synonyms_vector;
        while (std::getline(words_str, synonym, ' ')) {
            synonyms_vector.push_back(std::ref(words.try_emplace(synonym).first->first));
            words[synonym].insert(id);
        }
        std::getline(s, field, ','); // gloss
        synsets.emplace_back(synonyms_vector, field);
    }
}

void WordNet::read_hypernyms_fn(const std::string & hypernyms_fn)
{

    std::ifstream hypernyms_file(hypernyms_fn);
    if (!hypernyms_file.is_open()) {
        throw std::ifstream::failure("Error while opening the file with hypernyms");
    }
    std::string line;
    while (std::getline(hypernyms_file, line)) {
        std::istringstream s(line);
        std::string field;
        std::getline(s, field, ','); // id
        int id = std::stoul(field);
        while (std::getline(s, field, ',')) {
            digraph.addEdge(id, std::stoul(field));
        }
    }
}

WordNet::WordNet(const std::string & synsets_fn, const std::string & hypernyms_fn)
    : digraph()
    , scaFinder(digraph)
{
    read_synsets_fn(synsets_fn);
    read_hypernyms_fn(hypernyms_fn);
}
bool WordNet::is_noun(const std::string & word) const
{
    return words.find(word) != words.end();
}

WordNet::iterator WordNet::nouns()
{
    return iterator(words.begin());
}

WordNet::iterator WordNet::begin()
{
    return nouns();
}

WordNet::iterator WordNet::end()
{
    return iterator(words.end());
}

const std::set<int> & WordNet::getWordSynsets(const std::string & word) const
{
    return words.find(word)->second;
}
std::string WordNet::sca(const std::string & noun1, const std::string & noun2)
{
    if (!is_noun(noun1) || !is_noun(noun2))
        return "";
    int sca = scaFinder.ancestor_subset(getWordSynsets(noun1), getWordSynsets(noun2));
    return synsets[sca].gloss;
}
int WordNet::distance(const std::string & noun1, const std::string & noun2)
{
    if (!is_noun(noun1) || !is_noun(noun2))
        return digraph.size();
    return scaFinder.length_subset(getWordSynsets(noun1), getWordSynsets(noun2));
}

WordNet::iterator::iterator(const umapIter & dataIter)
    : dataIter(dataIter)
{
}

WordNet::iterator & WordNet::iterator::operator++()
{
    dataIter++;
    return *this;
}

WordNet::iterator WordNet::iterator::operator++(int)
{
    iterator res = *this;
    ++(*this);
    return res;
}

const WordNet::iterator::value_type & WordNet::iterator::operator*() const
{
    return dataIter->first;
}

const WordNet::iterator::value_type * WordNet::iterator::operator->() const
{
    return &dataIter->first;
}

bool WordNet::iterator::operator==(const iterator & that) const
{
    return dataIter == that.dataIter;
}

bool WordNet::iterator::operator!=(const iterator & that) const
{
    return !(*this == that);
}
//--------------

// Outcast class
Outcast::Outcast(WordNet & wordnet)
    : wordnet(wordnet)
{
}

int Outcast::distanceToVector(const std::string & word, const std::vector<std::string> & wordsVector) const
{
    int result = 0;
    for (const auto & w : wordsVector) {
        if (word != w) {
            result += wordnet.distance(word, w);
        }
    }
    return result;
}

std::string Outcast::outcast(const std::vector<std::string> & nouns)
{
    int maxDistance = 0;
    bool isOutcast = true;
    std::string outcast;
    for (const auto & word : nouns) {
        int distance = distanceToVector(word, nouns);
        //        if (word[word.length() - 1] == 's') {
        //            int nonPluralDistance = distanceToVector(word.substr(0, word.length() - 1), nouns);
        //            distance = distance < nonPluralDistance ? distance : nonPluralDistance;
        //        }
        if (distance == maxDistance) {
            isOutcast = false;
        }
        else if (distance > maxDistance) {
            isOutcast = true;
            maxDistance = distance;
            outcast = word;
        }
    }
    return isOutcast ? outcast : "";
}
// ------
