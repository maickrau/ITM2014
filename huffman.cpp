#include <vector>
#include <unordered_map>
#include <iostream>
#include <algorithm>
#include <fstream>

namespace std
{
	template <>
	struct hash<pair<char, char> > {
	public:
	        size_t operator()(pair<char, char> x) const throw() {
	             size_t h = get<0>(x) ^ get<1>(x);
	             return h;
	        }
	};
}

template <typename ValueType>
class HuffmanTree
{
private:
	class Node
	{
	public:
		Node(const ValueType& v, double probability) : value(v), probability(probability), zero(nullptr), one(nullptr), parent(nullptr), isZero() {};
		Node(Node* zero, Node* one) : value(), probability(), zero(zero), one(one), parent(nullptr), isZero()
		{
			one->parent = this;
			one->isZero = false;
			zero->parent = this;
			zero->isZero = true;
			probability = one->probability+zero->probability;
		};
		ValueType value;
		double probability;
		Node* zero;
		Node* one;
		Node* parent;
		bool isZero;
	};
	std::vector<Node> nodes;
	std::unordered_map<ValueType, Node*> mapping;
	Node* root;
public:
	HuffmanTree(std::vector<std::pair<ValueType, double>> initialNodes) : nodes(), mapping(), root(nullptr) 
	{
		makeTree(initialNodes);
	};
	HuffmanTree(const HuffmanTree&) = delete;
	HuffmanTree& operator=(const HuffmanTree&) = delete;

	ValueType decode(std::vector<bool> bitString) const
	{
		std::vector<ValueType> values;
		size_t loc = 0;
		while (loc < bitString.size())
		{
			Node* target = root;
			while (target->zero != nullptr)
			{
				if (bitString[loc])
				{
					target = target->one;
				}
				else
				{
					target = target->zero;
				}
				loc++;
			}
			values.push_back(target->value);
		}
		return values;
	};

	std::vector<bool> encode(const ValueType& v) const
	{
		std::vector<bool> ret;
		encodeTo(ret, v);
		return ret;
	};

	std::vector<bool> encode(const std::vector<ValueType>& values) const
	{
		std::vector<bool> ret;
		for (auto v = values.begin(); v != values.end(); v++)
		{
			encodeTo(ret, *v);
		}
		return ret;
	};
private:
	void makeTree(const std::vector<std::pair<ValueType, double>>& initialNodes)
	{
		std::vector<Node*> nodesLeft;
		nodes.reserve(initialNodes.size()*2);
		for (auto pair = initialNodes.begin(); pair != initialNodes.end(); pair++)
		{
			nodes.push_back(Node(std::get<0>(*pair), std::get<1>(*pair)));
			mapping[std::get<0>(*pair)] = &(nodes.back());
		}
		for (int i = 0; i < nodes.size(); i++)
		{
			nodesLeft.push_back(&(nodes[i]));
		}
		while (nodesLeft.size() > 1)
		{
			std::stable_sort(nodesLeft.begin(), nodesLeft.end(), [](Node* a, Node* b) { return a->probability < b->probability;});
			nodes.emplace_back(nodesLeft[0], nodesLeft[1]);
			nodesLeft.erase(nodesLeft.begin(), nodesLeft.begin()+2);
			nodesLeft.push_back(&(nodes.back()));
		}
		root = nodesLeft[0];
	};

	void encodeTo(std::vector<bool>& bits, const ValueType& v) const
	{
		std::vector<bool> reversed;
		Node* target = mapping.at(v);
		while (target->parent != nullptr)
		{
			reversed.push_back(target->isZero);
			target = target->parent;
		}
		bits.insert(bits.end(), reversed.rbegin(), reversed.rend());
	};
};

std::ostream& operator<<(std::ostream& out, const std::vector<bool>& vec)
{
	for (auto i : vec)
	{
		out << (((bool)i) ? "1" : "0");
	}
	return out;
}

std::vector<std::pair<char, double>> learnProbabilities(const std::string& fileName)
{
	std::unordered_map<char, int> count;
	for (int a = 0; a < 256; a++)
	{
		count[a] = 0;
	}
	std::ifstream file {fileName, std::ios::binary};
	int totalCount = 0;
	while (!file.eof())
	{
		char a = file.get();
		count[a]++;
		totalCount++;
	}
	file.close();
	std::vector<std::pair<char, double>> ret;
	for (auto i : count)
	{
		if (std::get<1>(i) > 0)
		{
			ret.emplace_back(std::get<0>(i), ((double)std::get<1>(i))/totalCount);
		}
	}
	return ret;
}

std::vector<char> readFile(const std::string& fileName)
{
	std::vector<char> ret;
	std::ifstream file {fileName, std::ios::binary};
	while (!file.eof())
	{
		char a = file.get();
		if (!file.eof())
		{
			ret.push_back(a);
		}
	}
	return ret;
}

void printSymbols(const HuffmanTree<char>& tree, const std::vector<char>& vec)
{
	std::unordered_map<char, bool> used;
	for (size_t i = 0; i < vec.size(); i++)
	{
		if (used.find(vec[i]) == used.end())
		{
			used[vec[i]] = true;
			std::cout << vec[i] << ":" << tree.encode(vec[i]) << "\n";
		}
	}
}

int main(int argc, char** argv)
{
	std::cerr << "count probabilities\n";
	auto probs = learnProbabilities(argv[1]);
	std::cerr << "make tree\n";
	HuffmanTree<char> tree {probs};
	std::cerr << "read pairs\n";
	auto pairs = readFile(argv[1]);
	std::cerr << "print symbols\n";
	printSymbols(tree, pairs);
}