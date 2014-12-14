/***

Caravan compressor using a bayesian network and arithmetic coding.

g++ bncompressor.cpp -std=c++11 -o bncompressor.out -lgmp

compress
./bncompressor.out c caravan.dat caravan.sdat compressed.out decompressed.out 5822 network.out
decompress
./bncompressor.out d caravan.dat caravan.sdat compressed.out decompressed.out 5822 network.out

todo: figure out a better BN network

Decompression is very slow compared to compression. Slowness is caused by the arbitrary-precision arithmetic used in the arithmetic encoder/decoder.
Arithmetic encoder encodes the symbols recursively, time is about 20s. Decoder doesn't (can't?) use this method, and takes about 6min.

***/

#include <gmpxx.h>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <cassert>
#include <map>
#include <set>
#include <ctime>
#include <sstream>
#include <cmath>
#include <algorithm>

int DEBUG_CARAVAN_LINES = 500;

int wroteBytesDistribution = 0;
int wroteBytesParameters = 0;
int wroteBytesNetwork = 0;

class Distribution
{
public:
	Distribution() {};
	Distribution(const std::vector<int>& counts) : probabilities(), cumulativeProbabilities()
	{
		makeDistribution(counts);
	};
	Distribution(const std::vector<unsigned char>& bytes, int& loc)
	{
		int nonZeroes = bytes[loc];
		loc += 1;
		std::vector<int> counts;
		for (int i = 0; i < nonZeroes; i++)
		{
			int pos = bytes[loc]*256+bytes[loc+1];
			loc += 2;
			int count = bytes[loc];
			loc += 1;
			if (counts.size() <= pos)
			{
				counts.resize(pos+1);
			}
			counts[pos] = count;
		}
		makeDistribution(counts);
	}
	mpq_class lowerBound(int symbol) const
	{
		if (symbol == 0)
		{
			return 0;
		}
		return cumulativeProbabilities[symbol-1];
	};
	mpq_class upperBound(int symbol) const
	{
		return cumulativeProbabilities[symbol];
	};
	mpq_class getProbability(int symbol) const
	{
		return probabilities[symbol];
	};
	size_t size() const
	{
		return probabilities.size();
	}
	int originalCount(int i) const
	{
		return originalCounts[i];
	}
	void encodeBytes(std::vector<unsigned char>& bytes)
	{
		int nonZeroes = 0;
		for (int i = 0; i < originalCounts.size(); i++)
		{
			if (originalCounts[i] > 0)
			{
				nonZeroes++;
			}
		}
		assert(nonZeroes < 256);
		bytes.push_back(nonZeroes);
		for (int i = 0; i < originalCounts.size(); i++)
		{
			if (originalCounts[i] > 0)
			{
				assert(i < 65536);
				bytes.push_back(i/256);
				bytes.push_back(i%256);
				assert(originalCounts[i] < 256);
				bytes.push_back(originalCounts[i]);
			}
		}
		wroteBytesDistribution += nonZeroes*3+1;
	}
	int maxCount;
private:
	void makeDistribution(std::vector<int> counts)
	{
		int largest = 0;
		for (int i = 0; i < counts.size(); i++)
		{
			if (counts[i] > largest)
			{
				largest = counts[i];
			}
		}
		for (int i = 0; i < counts.size(); i++)
		{
			if (counts[i] > 0)
			{
				counts[i] = ((double)counts[i]/(double)largest)*254+1;
			}
		}
		originalCounts = counts;
		int sum = 0;
		maxCount = 0;
		for (int i = 0; i < counts.size(); i++)
		{
			if (counts[i] > maxCount)
			{
				maxCount = counts[i];
			}
			sum += counts[i];
		}
		int cumulativeCounts = 0;
		for (size_t i = 0; i < counts.size(); i++)
		{
			cumulativeCounts += counts[i];
			probabilities.emplace_back(counts[i], sum);
			cumulativeProbabilities.emplace_back(cumulativeCounts, sum);
		}
	}
	std::vector<int> originalCounts;
	std::vector<mpq_class> probabilities;
	std::vector<mpq_class> cumulativeProbabilities;
};

class ArithmeticEncoder
{
public:
	ArithmeticEncoder() : location(0), width(1) {};
	void encode(mpq_class lowProbability, mpq_class highProbability)
	{
		location += lowProbability*width;
		width *= highProbability-lowProbability;
	};
	void encode(const Distribution& dist, int symbol)
	{
		encode(dist.lowerBound(symbol), dist.upperBound(symbol));
	};
	void encode(const ArithmeticEncoder& second)
	{
		encode(second.location, second.location+second.width);
	};
	std::vector<unsigned char> getBytes() const
	{
		mpq_class encodeLocation = location+(width*255)/256;
		mpq_class encodeWidth = width;
		std::vector<unsigned char> ret;
		encodeLocation *= 256;
		encodeWidth *= 256;
		while (encodeWidth < 256)
		{
			unsigned char writeByte = encodeLocation.get_d();
			ret.push_back(writeByte);
			encodeLocation -= writeByte;
			encodeLocation *= 256;
			encodeWidth *= 256;
		}
		return ret;
	};
	int getLength() const
	{
		int ret = 0;
		mpq_class encodeWidth = width;
		while (encodeWidth < 256)
		{
			ret++;
			encodeWidth *= 256;
		}
		return ret;
	}
private:
	mpq_class location;
	mpq_class width;
};

class ArithmeticDecoder
{
public:
	ArithmeticDecoder(const std::vector<unsigned char>& bytes) : location(0)
	{
		mpq_class width {1, 256};
		for (size_t i = 0; i < bytes.size(); i++)
		{
			location += bytes[i]*width;
			width /= 256;
		}
	};
	int getSymbol(const Distribution& dist)
	{
		int index = findSymbol(dist);
		location -= dist.lowerBound(index);
		location /= dist.getProbability(index);
		return index;
	};
private:
	int findSymbol(const Distribution& dist)
	{
		for (int i = 0; i < dist.size(); i++)
		{
			if (location < dist.upperBound(i))
			{
				return i;
			}
		}
	}
	mpq_class location;
};

class ColumnSet
{
public:
	ColumnSet(int columns, int rows) : columnss(columns), rowss(rows)
	{
		for (int i = 0; i < columnss; i++)
		{
			cells.emplace_back();
			cells.back().resize(rowss);
		}
	};
	int get(int column, int row) const
	{
		return cells[column][row];
	}
	ColumnSet getColumns(const std::vector<int>& columns) const
	{
		ColumnSet ret {columns.size(), rowss};
		for (int i = 0; i < columns.size(); i++)
		{
			for (int a = 0; a < rowss; a++)
			{
				ret.cells[i][a] = cells[columns[i]][a];
			}
		}
		return ret;
	}
	int addGrouper(const std::vector<int>& columns)
	{
		cells.emplace_back();
		cells.back().resize(rowss);
		columnss++;
		std::map<std::vector<int>, int> used;
		int current = 0;
		for (int i = 0; i < rowss; i++)
		{
			std::vector<int> found;
			for (int a = 0; a < columns.size(); a++)
			{
				found.push_back(cells[columns[a]][i]);
			}
			if (used.find(found) == used.end())
			{
				cells[columnss-1][i] = current;
				used[found] = current;
				current++;
			}
			else
			{
				cells[columnss-1][i] = used[found];
			}
		}
		return columnss-1;
	};
	double getEntropy()
	{
		std::map<std::vector<int>, int> counts;
		for (int i = 0; i < rowss; i++)
		{
			std::vector<int> test;
			for (int a = 0; a < columnss; a++)
			{
				test.push_back(cells[a][i]);
			}
			counts[test]++;
		}
		double ret = 0;
		for (auto iter = counts.begin(); iter != counts.end(); iter++)
		{
			double prob = (double)iter->second/(double)rowss;
			ret -= prob*log2(prob);
		}
		return ret;
	}
	std::vector<int>& operator[](int n)
	{
		return cells[n];
	};
	const std::vector<int>& operator[](int n) const
	{
		return cells[n];
	};
	int columns() const
	{
		return columnss;
	};
	int rows() const
	{
		return rowss;
	};
private:
	int columnss;
	int rowss;
	//first is column, second is row.
	//(0,0) (1,0)
	//(0,1) (1,1)
	std::vector<std::vector<int>> cells;
};

int fac(int n)
{
	if (n == 0)
	{
		return 1;
	}
	return n*fac(n-1);
}

class BayesParameters
{
public:
	BayesParameters() {};
	BayesParameters(const ColumnSet& data, std::vector<int> parents, int thisNode) : nodes()
	{
		learn(data, parents, thisNode);
	};
	BayesParameters(const std::vector<unsigned char>& bytes, int& loc)
	{
		int size = bytes[loc]*256+bytes[loc+1];
		loc += 2;
		for (int i = 0; i < size; i++)
		{
			nodes.emplace_back();
			int childrenSize = bytes[loc]*256+bytes[loc+1];
			loc += 2;
			if (childrenSize == 0)
			{
				nodes.back().leaf = Distribution(bytes, loc);
			}
			else
			{
				for (int a = 0; a < childrenSize; a++)
				{
					int value = bytes[loc]*256+bytes[loc+1];
					loc += 2;
					int child = bytes[loc]*256+bytes[loc+1];
					loc += 2;
					nodes.back().children.emplace(value, child);
				}
				nodes.back().testsVariable = bytes[loc];
				loc++;
			}
		}
	}
	const Distribution& getDistribution(const std::vector<int>& parents)
	{
		int node = 0;
		while (!nodes[node].children.empty())
		{
			node = nodes[node].children[parents[nodes[node].testsVariable]];
		}
		return nodes[node].leaf;
	}
	void learn(const ColumnSet& data, std::vector<int> parents, int thisNode)
	{
		parents.insert(parents.begin(), thisNode);
		ColumnSet relevantColumns = data.getColumns(parents);
		std::map<std::vector<int>, Distribution> parameters = learnDistributions(relevantColumns);
		makeTree(parameters, permutation(parameters.begin()->first.size(), 0));
		// makeBestTree(parameters);
	}
	int maxCount()
	{
		int maxC = 0;
		for (auto i = 0; i < nodes.size(); i++)
		{
			if (nodes[i].leaf.maxCount > maxC)
			{
				maxC = nodes[i].leaf.maxCount;
			}
		}
		return maxC;
	}
	int maxSize()
	{
		return nodes.size();
	}
	int maxChildren()
	{
		int maxC = 0;
		for (int i = 0; i < nodes.size(); i++)
		{
			if (nodes[i].children.size() > maxC)
			{
				maxC = nodes[i].children.size();
			}
		}
		return maxC;
	}
	void encodeBytes(std::vector<unsigned char>& bytes)
	{
		assert(nodes.size() < 65536);
		bytes.push_back(nodes.size()/256);
		bytes.push_back(nodes.size()%256);
		wroteBytesParameters += 2;
		for (auto i = 0; i < nodes.size(); i++)
		{
			assert(nodes[i].children.size() < 65536);
			bytes.push_back(nodes[i].children.size()/256);
			bytes.push_back(nodes[i].children.size()%256);
			wroteBytesParameters += 2;
			if (nodes[i].children.size() == 0)
			{
				nodes[i].leaf.encodeBytes(bytes);
			}
			else
			{
				for (auto iter = nodes[i].children.begin(); iter != nodes[i].children.end(); iter++)
				{
					wroteBytesParameters += 4;
					assert(iter->first < 65536);
					bytes.push_back(iter->first/256);
					bytes.push_back(iter->first%256);
					assert(iter->second < 65536);
					bytes.push_back(iter->second/256);
					bytes.push_back(iter->second%256);
				}
				wroteBytesParameters += 1;
				assert(nodes[i].testsVariable < 256);
				bytes.push_back(nodes[i].testsVariable);
			}
		}
	}
private:
	int getSize()
	{
		return nodes.size();
	}
	void makeBestTree(const std::map<std::vector<int>, Distribution>& parameters)
	{
		int nodes = parameters.begin()->first.size();
		int smallestIndex = 0;
		int smallestSize = 0;
		makeTree(parameters, permutation(nodes, 0));
		smallestSize = getSize();
		for (int i = 1; i < fac(nodes); i++)
		{
			makeTree(parameters, permutation(nodes, i));
			int newSize = getSize();
			if (newSize < smallestSize)
			{
				smallestSize = newSize;
				smallestIndex = i;
			}
		}
		makeTree(parameters, permutation(nodes, smallestIndex));
	}
	std::vector<int> permutation(int maxNum, int number)
	{
		std::vector<int> ret;
		std::vector<int> numbersLeft;
		for (int i = 0; i < maxNum; i++)
		{
			numbersLeft.push_back(i);
		}
		int moduloNumber = maxNum;
		for (int i = 0; i < maxNum; i++)
		{
			ret.push_back(numbersLeft[number%moduloNumber]);
			numbersLeft.erase(numbersLeft.begin()+(number%moduloNumber));
			number /= moduloNumber;
			moduloNumber--;
		}
		return ret;
	}
	void makeTree(const std::map<std::vector<int>, Distribution>& parameters, const std::vector<int>& parameterOrder)
	{
		nodes.clear();
		nodes.emplace_back();
		makeDecisionTreeRec(parameters, 0, 0, parameterOrder);
	}
	std::map<std::vector<int>, Distribution> filterParameters(const std::map<std::vector<int>, Distribution>& oldParams, int paramNum, int paramValue)
	{
		std::map<std::vector<int>, Distribution> ret;
		for (auto iter = oldParams.begin(); iter != oldParams.end(); iter++)
		{
			if (iter->first[paramNum] == paramValue)
			{
				ret.emplace(iter->first, iter->second);
			}
		}
		return ret;
	}
	void makeDecisionTreeRec(const std::map<std::vector<int>, Distribution>& parameters, int nodeNum, int parameterNum, const std::vector<int>& parameterOrder)
	{
		if (parameters.size() == 1)
		{
			nodes[nodeNum].leaf = parameters.begin()->second;
			return;
		}
		nodes[nodeNum].testsVariable = parameterOrder[parameterNum];
		std::set<int> usedValues;
		for (auto iter = parameters.begin(); iter != parameters.end(); iter++)
		{
			if (usedValues.count(iter->first[parameterOrder[parameterNum]]) == 0)
			{
				usedValues.emplace(iter->first[parameterOrder[parameterNum]]);
				nodes.emplace_back();
				nodes.back().testsVariable = parameterOrder[parameterNum+1];
				nodes[nodeNum].children.emplace(iter->first[parameterOrder[parameterNum]], nodes.size()-1);
				auto newParams = filterParameters(parameters, parameterOrder[parameterNum], iter->first[parameterOrder[parameterNum]]);
				makeDecisionTreeRec(newParams, nodes.size()-1, parameterNum+1, parameterOrder);
			}
		}
	}
	std::map<std::vector<int>, Distribution> learnDistributions(const ColumnSet& data)
	{
		std::map<std::vector<int>, std::vector<int>> counts;
		for (int i = 0; i < data.rows(); i++)
		{
			std::vector<int> findDistribution;
			for (int a = 1; a < data.columns(); a++)
			{
				findDistribution.push_back(data[a][i]);
			}
			std::vector<int>& currentDistribution = counts[findDistribution];
			if (currentDistribution.size() <= data[0][i])
			{
				currentDistribution.resize(data[0][i]+1);
			}
			currentDistribution[data[0][i]]++;
		}

		std::map<std::vector<int>, Distribution> ret;
		for (auto iter = counts.begin(); iter != counts.end(); iter++)
		{
			ret.emplace(iter->first, Distribution(iter->second));
		}
		return ret;
	}
	class ParameterNode
	{
	public:
		int testsVariable;
		std::map<int, int> children;
		Distribution leaf;
	};
	std::vector<ParameterNode> nodes;
};

class BayesNode
{
public:
	BayesNode() {};
	BayesNode(const ColumnSet& datas, int ownNode, std::vector<int> parents) : ownVariable(ownNode), parents(parents), parameters(datas, parents, ownNode) {};
	int ownVariable;
	std::vector<int> parents;
	BayesParameters parameters;
};

class BayesNetwork
{
public:
	BayesNetwork(const ColumnSet& datas, std::vector<std::pair<int, int>> links, std::vector<int> order)
	{
		std::vector<int> deOrder;
		deOrder.resize(order.size());
		for (int i = 0; i < order.size(); i++)
		{
			deOrder[order[i]] = i;
		}
		nodes.resize(order.size());
		for (int i = 0; i < nodes.size(); i++)
		{
			// std::cerr << "making node " << i << "/" << nodes.size() << " ";
			nodes[i].ownVariable = order[i];
			for (int a = 0; a < links.size(); a++)
			{
				if (links[a].first == order[i] && deOrder[links[a].second] < deOrder[links[a].first])
				{
					nodes[i].parents.push_back(links[a].second);
				}
				else if (links[a].second == order[i] && deOrder[links[a].first] < deOrder[links[a].second])
				{
					nodes[i].parents.push_back(links[a].first);
				}
			}
			std::set<int> removeDoubles;
			for (int a = 0; a < nodes[i].parents.size(); a++)
			{
				removeDoubles.insert(nodes[i].parents[a]);
			}
			nodes[i].parents = {};
			for (auto a = removeDoubles.begin(); a != removeDoubles.end(); a++)
			{
				nodes[i].parents.push_back(*a);
			}
			// std::cerr << "(" << nodes[i].parents.size() << " parents)";
			int startTime = clock();
			nodes[i].parameters.learn(datas, nodes[i].parents, nodes[i].ownVariable);
			int endTime = clock();
			// std::cerr << " " << (endTime-startTime) << " ms\n";
		}
	}
	BayesNetwork(const std::vector<unsigned char>& bytes, int loc)
	{
		int size = bytes[loc];
		loc++;
		nodes.resize(size);
		for (int i = 0; i < size; i++)
		{
			nodes[i].ownVariable = bytes[loc];
			loc++;
			int numParents = bytes[loc];
			nodes[i].parents.resize(numParents);
			loc++;
			for (int a = 0; a < numParents; a++)
			{
				nodes[i].parents[a] = bytes[loc];
				loc++;
			}
			nodes[i].parameters = BayesParameters{bytes, loc};
		}
	}
	std::vector<unsigned char> encodeDataset(const ColumnSet& datas)
	{
		ArithmeticEncoder encoder = encodeDatasetRec(datas, 0, nodes.size());
		// std::cerr << "encode bytes\n";
		return encoder.getBytes();
	}
	ColumnSet decodeBytes(const std::vector<unsigned char>& bytes, int rows)
	{
		ColumnSet ret {nodes.size(), rows};
		ArithmeticDecoder decoder {bytes};
		for (int i = 0; i < nodes.size(); i++)
		{
			// std::cerr << "decode " << i << "/" << nodes.size() << " ";
			unsigned int timeStart = clock();
			decodeNode(ret, decoder, i);
			unsigned int timeEnd = clock();
			// std::cerr << timeEnd-timeStart << " ms\n";
		}
		return ret;
	}
	void debugPrint(std::ostream& out)
	{
		for (int i = 0; i < nodes.size(); i++)
		{
			out << nodes[i].ownVariable;
			for (int a = 0; a < nodes[i].parents.size(); a++)
			{
				out << "\n\t" << nodes[i].parents[a];
			}
			out << "\n\n";
		}
	}
	int maxCount()
	{
		int maxC = 0;
		for (int i = 0; i < nodes.size(); i++)
		{
			int compare = nodes[i].parameters.maxCount();
			if (compare > maxC)
			{
				maxC = compare;
			}
		}
		return maxC;
	}
	int maxSize()
	{
		int maxS = 0;
		for (int i = 0; i < nodes.size(); i++)
		{
			if (nodes[i].parameters.maxSize() > maxS)
			{
				maxS = nodes[i].parameters.maxSize();
			}
		}
		return maxS;
	}
	int maxChildren()
	{
		int maxC = 0;
		for (int i = 0; i < nodes.size(); i++)
		{
			int compare = nodes[i].parameters.maxChildren();
			if (compare > maxC)
			{
				maxC = compare;
			}
		}
		return maxC;
	}
	std::vector<unsigned char> encodeBytes()
	{
		std::vector<unsigned char> ret;
		assert(nodes.size() < 256);
		ret.push_back(nodes.size());
		wroteBytesNetwork += 1;
		for (int i = 0; i < nodes.size(); i++)
		{
			wroteBytesNetwork += 2;
			assert(nodes[i].ownVariable < 256);
			ret.push_back(nodes[i].ownVariable);
			assert(nodes[i].parents.size() < 256);
			ret.push_back(nodes[i].parents.size());
			for (int a = 0; a < nodes[i].parents.size(); a++)
			{
				wroteBytesNetwork += 1;
				assert(nodes[i].parents[a] < 256);
				ret.push_back(nodes[i].parents[a]);
			}
			nodes[i].parameters.encodeBytes(ret);
		}
		return ret;
	}
	int getDataEncodeLength(const ColumnSet& datas)
	{
		ArithmeticEncoder encoder = encodeDatasetRec(datas, 0, nodes.size());
		return encoder.getLength();
	}
private:
	ArithmeticEncoder encodeDatasetRec(const ColumnSet& datas, int startNode, int endNode)
	{
		if (endNode == startNode)
		{
			return ArithmeticEncoder();
		}
		if (endNode == startNode+1)
		{
			// std::cerr << "encode node " << startNode << "\n";
			return encodeNode(datas, startNode);
		}
		int middle = (endNode-startNode)/2+startNode;
		ArithmeticEncoder left = encodeDatasetRec(datas, startNode, middle);
		ArithmeticEncoder right = encodeDatasetRec(datas, middle, endNode);
		left.encode(right);
		return left;
	}
	void decodeNode(ColumnSet& out, ArithmeticDecoder& decoder, int node)
	{
		std::vector<int> columns = nodes[node].parents;
		ColumnSet limitedSet = out.getColumns(columns);
		std::vector<int> values;
		values.resize(limitedSet.columns());
		for (int i = 0; i < out.rows(); i++)
		{
			for (int a = 0; a < limitedSet.columns(); a++)
			{
				values[a] = limitedSet[a][i];
			}
			int* columnsP = columns.data();
			int* vals = values.data();
			int sizes = values.size();
			out[nodes[node].ownVariable][i] = decoder.getSymbol(nodes[node].parameters.getDistribution(values));
		}
	}
	ArithmeticEncoder encodeNode(const ColumnSet& datas, int node)
	{
		ArithmeticEncoder encoder;
		std::vector<int> columns = nodes[node].parents;
		columns.insert(columns.begin(), nodes[node].ownVariable);
		ColumnSet limitedSet = datas.getColumns(columns);
		return encodeNodeRec(limitedSet, node, 0, datas.rows());
	}
	ArithmeticEncoder encodeNodeRec(const ColumnSet& datas, int node, int startRow, int endRow)
	{
		if (startRow+1 == endRow)
		{
			ArithmeticEncoder encoder;
			std::vector<int> values;
			for (int a = 1; a < datas.columns(); a++)
			{
				values.push_back(datas[a][startRow]);
			}
			encoder.encode(nodes[node].parameters.getDistribution(values), datas[0][startRow]);
			return encoder;
		}
		if (startRow == endRow)
		{
			return ArithmeticEncoder();
		}
		int middle = (endRow-startRow)/2+startRow;
		ArithmeticEncoder left = encodeNodeRec(datas, node, startRow, middle);
		ArithmeticEncoder right = encodeNodeRec(datas, node, middle, endRow);
		left.encode(right);
		return left;
	}
	std::vector<BayesNode> nodes;
};

ColumnSet readCaravan(std::string fileName, std::string sideInfoName)
{
	std::ifstream file {fileName};
	std::ifstream side {sideInfoName};
	ColumnSet ret {86, DEBUG_CARAVAN_LINES};
	int n;
	for (int i = 0; i < ret.rows(); i++)
	{
		for (int a = 1; a < ret.columns(); a++)
		{
			file >> n;
			ret[a][i] = n;
		}
		side >> n;
		ret[0][i] = n;
	}
	return ret;
}

ColumnSet readTestData(std::string fileName, int columns, int rows)
{
	std::ifstream file {fileName};
	ColumnSet ret {columns, rows};
	assert(file.good());
	for (int i = 0; i < rows; i++)
	{
		for (int a = 0; a < columns; a++)
		{
			int n;
			assert(file.good());
			file >> n;
			ret[a][i] = n;
		}
	}
	return ret;
}

std::vector<unsigned char> getFileBytes(std::string fileName)
{
	std::ifstream file {fileName, std::ios::binary};
	std::vector<unsigned char> bytes;
	unsigned char a = file.get();
	while (file.good())
	{
		bytes.push_back(a);
		a = file.get();
	}
	return bytes;
}

void addClique(std::vector<std::pair<int, int>>& links, std::vector<int> parts)
{
	for (int a = 0; a < parts.size(); a++)
	{
		for (int b = a+1; b < parts.size(); b++)
		{
			links.emplace_back(parts[a], parts[b]);
		}
	}
}

BayesNetwork selectiveNetwork(ColumnSet& data)
{
	//network found by entropyMode. 
	//Links are the 40 pairs with highest I(x, y)/min(E(X), E(Y))
	//Order is taken from the links so highly connected variables are closely ordered
	std::vector<int> order {
		45,66,
		53,74,
		58,79,
		64,85,
		56,77,
		49,70,
		63,84,
		46,67,
		62,83,
		1,5,
		51,72,
		48,69,
		57,78,
		35,36,
		30,31,
		44,65,
		54,75,
		61,82,
		52,73,
		1,43,
		55,76,
		59,80,
		47,68,
		74,80,
		53,80,
		51,53,
		51,74,
		53,72,
		72,74,
		56,74,
		53,56,
		46,74,
		46,53,
		74,77,
		53,77,
		52,74,
		52,53,
		21,74,
		21,53,
		1,53,
		1,74,
		10,12,
		53,73,
		73,74,
		67,74,
		53,67,
		5,43
	};
	std::reverse(order.begin(), order.end());
	std::vector<std::pair<int, int>> links {
		{45, 66}
		,{53, 74}
		,{58, 79}
		,{64, 85}
		,{56, 77}
		,{49, 70}
		,{63, 84}
		,{46, 67}
		,{62, 83}
		,{1, 5}
		,{51, 72}
		,{48, 69}
		,{57, 78}
		,{35, 36}
		,{30, 31}
		,{44, 65}
		,{54, 75}
		,{61, 82}
		,{52, 73}
		,{1, 43}
		,{55, 76}
		,{59, 80}
		,{47, 68}
		,{74, 80}
		,{53, 80}
		,{51, 53}
		,{51, 74}
		,{53, 72}
		,{72, 74}
		,{56, 74}
		,{53, 56}
		,{46, 74}
		,{46, 53}
		,{74, 77}
		,{53, 77}
		,{52, 74}
		,{52, 53}
		,{21, 74}
		,{21, 53}
		,{1, 53}
	};
	for (int i = 0; i < data.columns(); i++)
	{
		bool found = false;
		for (int a = 0; a < order.size(); a++)
		{
			if (order[a] == i)
			{
				found = true;
				break;
			}
		}
		if (!found)
		{
			order.push_back(i);
		}
	}
	for (int i = data.columns()-1; i >= 0; i--)
	{
		for (int a = 0; a < i; a++)
		{
			if (order[a] == order[i])
			{
				order.erase(order.begin()+i);
				a = i;
			}
		}
	}
	return BayesNetwork{data, links, order};
}

BayesNetwork minimalKnowledgeNetwork(ColumnSet& data)
{
	std::vector<int> order {0, 5, 1};
	std::vector<std::pair<int, int>> links {{0, 1}, {1, 5}, {0, 5}};

	for (int i = 2; i < 44; i++)
	{
		order.push_back(i);
	}
	for (int i = 44; i < 65; i++)
	{
		order.push_back(i+21);
		order.push_back(i);
	}
	for (int i = 2; i < 37; i++)
	{
		if (i != 5)
		{
			links.emplace_back(5, i);
		}
	}
	for (int i = 44; i < 65; i++)
	{
		links.emplace_back(5, i+21);
		links.emplace_back(i, i+21);
	}

	for (int i = 0; i < data.columns(); i++)
	{
		bool found = false;
		for (int a = 0; a < order.size(); a++)
		{
			if (order[a] == i)
			{
				found = true;
				break;
			}
		}
		if (!found)
		{
			order.push_back(i);
		}
	}
	for (int i = data.columns()-1; i >= 0; i--)
	{
		for (int a = 0; a < i; a++)
		{
			if (order[a] == order[i])
			{
				order.erase(order.begin()+i);
				a = i;
			}
		}
	}

	return BayesNetwork{data, links, order};
}

BayesNetwork maximumEntropyNetwork(ColumnSet& data)
{
	std::vector<int> order;
	for (int i = 0; i < data.columns(); i++)
	{
		order.push_back(i);
	}
	return BayesNetwork {data, {}, order};
}

BayesNetwork defaultNetwork(ColumnSet& data)
{
	const int age = 4;
	const int income = 42;
	const int mainType = 5;
	const int subType = 1;
	std::vector<std::pair<int, int>> links = {
		{mainType, subType},
		{subType, age},
		{age, income},
		{2, income}, //income vs n. of houses
		{43, income}, //income vs purchasing power
	};
	int religion = data.addGrouper({6, 7, 8, 9});
	addClique(links, {religion, 6, 7, 8, 9}); //religion
	int maritalStatus = data.addGrouper({10, 11, 12, 13});
	addClique(links, {maritalStatus, 10, 11, 12, 13}); //marital status
	int children = data.addGrouper({14, 15});
	addClique(links, {children, 14, 15}); //children
	int education = data.addGrouper({16, 17, 18});
	addClique(links, {education, 16, 17, 18}); //education
	int job = data.addGrouper({19, 20, 21, 22, 23, 24});
	addClique(links, {job, 19, 20, 21, 22, 23, 24}); //job
	int socialClass = data.addGrouper({25, 26, 27, 28, 29});
	addClique(links, {socialClass, 25, 26, 27, 28, 29}); //social class
	int home = data.addGrouper({30, 31});
	addClique(links, {home, 30, 31}); //home ownership
	int car = data.addGrouper({32, 33, 34});
	addClique(links, {car, 32, 33, 34}); //car ownership
	int insurance = data.addGrouper({35, 36});
	addClique(links, {insurance, 35, 36}); //health insurance
	addClique(links, {income, 37, 38, 39, 40, 41}); //income
	//insurance number vs contribution
	for (int i = 44; i < 65; i++)
	{
		links.emplace_back(i, i+21);
		links.emplace_back(i+21, subType);
		// links.emplace_back(i+21, mainType);
	}
	links.emplace_back(age, religion);
	links.emplace_back(age, maritalStatus);
	links.emplace_back(subType, maritalStatus);
	links.emplace_back(subType, religion);
	links.emplace_back(subType, education);
	links.emplace_back(children, maritalStatus);
	links.emplace_back(religion, education);
	links.emplace_back(job, education);
	links.emplace_back(job, socialClass);
	links.emplace_back(income, home);
	links.emplace_back(income, socialClass);
	links.emplace_back(income, car);
	links.emplace_back(68, car); //car vs car insurance
	// links.emplace_back(21, 71); //farmer vs farmer equipment insurance
	// links.emplace_back(21, 72);
	// links.emplace_back(21, 73);
	// links.emplace_back(21, 74);
	std::vector<int> order = {
		0, //side data
		5, //main type
		1, //subtype
		4, //age
		income,
		religion,
		maritalStatus,
		children,
		education,
		job,
		socialClass,
		home,
		car,
		insurance,
		21 //farmer
	};
	for (int i = 44; i < 65; i++)
	{
		order.push_back(i+21);
		order.push_back(i);
	}

	// for (int i = 0; i < data.columns(); i++)
	// {
	// 	links.emplace_back(0, i);
	// }
	for (int i = 0; i < data.columns(); i++)
	{
		bool found = false;
		for (int a = 0; a < order.size(); a++)
		{
			if (order[a] == i)
			{
				found = true;
				break;
			}
		}
		if (!found)
		{
			order.push_back(i);
		}
	}
	for (int i = data.columns()-1; i >= 0; i--)
	{
		for (int a = 0; a < i; a++)
		{
			if (order[a] == order[i])
			{
				order.erase(order.begin()+i);
				a = i;
			}
		}
	}
	BayesNetwork net {data, links, order};
	return net;
}

int sizeTestMode(char** argv)
{
	DEBUG_CARAVAN_LINES = atoi(argv[6]);

	ColumnSet data = readCaravan(argv[2], argv[3]);
	std::cerr << "make network\n";
	BayesNetwork net = selectiveNetwork(data);

	std::cerr << "net max count: " << net.maxCount() << "\n";
	std::cerr << "net max decision tree size: " << net.maxSize() << "\n";
	std::cerr << "net max children: " << net.maxChildren() << "\n";

	std::vector<unsigned char> netBytes = net.encodeBytes();
	std::cerr << "got network bytes " << netBytes.size() << "\n";

	int dataSize = net.getDataEncodeLength(data);
	std::cerr << "data size: " << dataSize << "\n";

	std::ofstream out2 {argv[7], std::ios::binary};
	out2.write((char*)netBytes.data(), netBytes.size());
}

int entropyMode(char** argv)
{
	ColumnSet data = readCaravan(argv[2], argv[3]);
	std::vector<double> entropy;
	std::vector<std::tuple<int, double>> entropies;
	for (int i = 0; i < data.columns(); i++)
	{
		ColumnSet part = data.getColumns({i});
		entropy.push_back(part.getEntropy());
		entropies.emplace_back(i, entropy[i]);
		// std::cout << i << ": " << entropy[i] << "\n";
	}
	std::vector<std::tuple<int, int, double>> coefficients;
	for (int i = 0; i < data.columns(); i++)
	{
		for (int a = 0; a < i; a++)
		{
			ColumnSet part = data.getColumns({i, a});
			double jointEntropy = part.getEntropy();
			double mutualInformation = (entropy[i]+entropy[a]-jointEntropy);
			double relationCoefficient = -1;
			if (entropy[i] != 0 && entropy[a] != 0)
			{
				relationCoefficient = mutualInformation/std::min(entropy[i], entropy[a]);
			}
			coefficients.emplace_back(a, i, relationCoefficient);
		}
	}
	std::sort(entropies.begin(), entropies.end(),
			[](std::tuple<int, double> a, std::tuple<int, double> b)
			{
				return std::get<1>(a) > std::get<1>(b);
			});
	std::sort(coefficients.begin(), coefficients.end(), 
			[](std::tuple<int, int, double> a, std::tuple<int, int, double> b) 
			{
				return std::get<2>(a) > std::get<2>(b) || 
						(std::get<2>(a) == std::get<2>(b) && std::get<1>(a) > std::get<1>(b)) || 
						(std::get<2>(a) == std::get<2>(b) && std::get<1>(a) == std::get<1>(b) && std::get<0>(a) > std::get<0>(b));
			});
	for (int i = 0; i < entropies.size(); i++)
	{
		std::cout << std::get<0>(entropies[i]) << ": " << std::get<1>(entropies[i]) << "\n";
	}
	for (int i = 0; i < coefficients.size(); i++)
	{
		std::cout << std::get<0>(coefficients[i]) << ", " << std::get<1>(coefficients[i]) << ": " << std::get<2>(coefficients[i]) << "\n";
	}
}

int caravanMode(char** argv)
{
	DEBUG_CARAVAN_LINES = atoi(argv[6]);
	if (*argv[1] == 'c')
	{
		ColumnSet data = readCaravan(argv[2], argv[3]);
		std::cerr << "make network\n";
		BayesNetwork net = defaultNetwork(data);

		std::cerr << "net max count: " << net.maxCount() << "\n";
		std::cerr << "net max size: " << net.maxSize() << "\n";
		std::cerr << "net max children: " << net.maxChildren() << "\n";

		std::vector<unsigned char> netBytes = net.encodeBytes();
		std::cerr << "got network bytes (" << netBytes.size() << ")\n";

		std::cerr << "wrote network bytes: " << wroteBytesNetwork << "\n";
		std::cerr << "wrote parameter bytes: " << wroteBytesParameters << "\n";
		std::cerr << "wrote distribution bytes: " << wroteBytesDistribution << "\n";

		std::vector<unsigned char> bytes = net.encodeDataset(data);
		std::cerr << "got probability bytes (" << bytes.size() << ")\n";
		std::cerr << "write bytes\n";
		std::ofstream out {argv[4], std::ios::binary};
		out.write((char*)bytes.data(), bytes.size());

		std::ofstream out2 {argv[7], std::ios::binary};
		out2.write((char*)netBytes.data(), netBytes.size());
	}
	else
	{
		std::cerr << "get network bytes\n";
		std::vector<unsigned char> networkBytes = getFileBytes(argv[7]);
		std::cerr << "make network\n";
		BayesNetwork net {networkBytes, 0};

		std::cerr << "get data bytes\n";
		std::vector<unsigned char> bytes = getFileBytes(argv[4]);
		std::cerr << "got bytes\n";

		ColumnSet outData = net.decodeBytes(bytes, DEBUG_CARAVAN_LINES);
		std::ofstream out {argv[5]};
		for (int i = 0; i < outData.rows(); i++)
		{
			out << outData[1][i];
			for (int a = 2; a < 86; a++)
			{
				out << "\t" << outData[a][i];
			}
			if (i != outData.rows()-1)
			{
				out << "\n";
			}
		}
	}
}

int main(int argc, char** argv)
{
	// entropyMode(argv);
	// caravanMode(argv);
	sizeTestMode(argv);
}