#include <deque>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

class Digraph
{
private:
    std::vector<std::vector<int>> adjacencyList;

public:
    Digraph();
    void addNode();
    void addEdge(int from, int to);
    std::size_t size() const;
    const std::vector<int> & operator[](std::size_t idx) const;
};

class ShortestCommonAncestor
{
private:
    Digraph & digraph;

    std::vector<int> bfs(int node) const;

public:
    ShortestCommonAncestor(Digraph & dg);

    // calculates length of shortest common ancestor path from node with id 'v' to node with id 'w'
    int length(int v, int w);

    // returns node id of shortest common ancestor of nodes v and w
    int ancestor(int v, int w);

    // calculates length of shortest common ancestor path from node subset 'subset_a' to node subset 'subset_b'
    int length_subset(const std::set<int> & subset_a, const std::set<int> & subset_b);

    // returns node id of shortest common ancestor of node subset 'subset_a' and node subset 'subset_b'
    int ancestor_subset(const std::set<int> & subset_a, const std::set<int> & subset_b);
};

class WordNet
{
private:
    void read_synsets_fn(const std::string & synsets_fn);
    void read_hypernyms_fn(const std::string & hypernyms_fn);
    const std::set<int> & getWordSynsets(const std::string & word) const;

private:
    struct Synset
    {
        const std::vector<std::reference_wrapper<const std::string>> synset;
        const std::string gloss;
        Synset(const std::vector<std::reference_wrapper<const std::string>> & synset, const std::string & gloss);
    };
    Digraph digraph;
    ShortestCommonAncestor scaFinder;
    std::vector<Synset> synsets;
    std::unordered_map<std::string, std::set<int>> words;

public:
    WordNet(const std::string & synsets_fn, const std::string & hypernyms_fn);

    class iterator
    {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = std::string;
        using difference_type = std::ptrdiff_t;
        using reference = value_type &;
        using pointer = value_type *;
        using umap = std::unordered_map<std::string, std::set<int>>;
        using umapIter = umap::iterator;

        iterator() = default;

        iterator(const umapIter & dataIter);

        iterator & operator++();

        iterator operator++(int);

        const value_type & operator*() const;

        const value_type * operator->() const;

        bool operator==(const iterator & that) const;

        bool operator!=(const iterator & that) const;

    private:
        umapIter dataIter;
    };

    // get iterator to list all nouns stored in WordNet
    iterator nouns();
    iterator begin();
    iterator end();

    // returns 'true' iff 'word' is stored in WordNet
    bool is_noun(const std::string & word) const;

    // returns gloss of "shortest common ancestor" of noun1 and noun2
    std::string sca(const std::string & noun1, const std::string & noun2);

    // calculates distance between noun1 and noun2
    int distance(const std::string & noun1, const std::string & noun2);
};

class Outcast
{
private:
    WordNet & wordnet;

    int distanceToVector(const std::string & word, const std::vector<std::string> & wordsVector) const;

public:
    Outcast(WordNet & wordnet);

    // returns outcast word
    std::string outcast(const std::vector<std::string> & nouns);
};
