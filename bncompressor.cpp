/***

Caravan compressor using a bayesian network and arithmetic coding.

g++ bncompressor.cpp -std=c++11 -o bncompressor.out -lgmp

compress
./bncompressor.out c caravan.dat caravan.sdat compressed.out decompressed.out 5822
decompress
./bncompressor.out d caravan.dat caravan.sdat compressed.out decompressed.out 5822

todo: encode network, improve running time (now compresses in 3min, decompresses in 6min)

***/

#include <gmpxx.h>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <cassert>
#include <map>
#include <set>

int DEBUG_CARAVAN_LINES = 500;

class Distribution
{
public:
	Distribution(const std::vector<int>& counts) : probabilities(), cumulativeProbabilities()
	{
		int sum = 0;
		for (int i = 0; i < counts.size(); i++)
		{
			sum += counts[i];
		}
		int cumulativeCounts = 0;
		for (size_t i = 0; i < counts.size(); i++)
		{
			cumulativeCounts += counts[i];
			probabilities.emplace_back(counts[i], sum);
			cumulativeProbabilities.emplace_back(cumulativeCounts, sum);
		}
	};
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
private:
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
		for (size_t i = 0; i < dist.size(); i++)
		{
			if (location >= dist.lowerBound(i) && location < dist.upperBound(i))
			{
				location -= dist.lowerBound(i);
				location /= dist.getProbability(i);
				return i;
			}
		}
	};
private:
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

class BayesParameters
{
public:
	BayesParameters() {};
	BayesParameters(std::map<std::vector<int>, Distribution> params) : parameters(params) {};
	BayesParameters(const ColumnSet& data, std::vector<int> parents, int thisNode) : parameters()
	{
		learn(data, parents, thisNode);
	};
	const Distribution& getDistribution(const std::vector<int>& parents)
	{
		auto found = parameters.find(parents);
		if (found != parameters.end())
		{
			return found->second;
		}
		assert(false);
	}
	void learn(const ColumnSet& data, std::vector<int> parents, int thisNode)
	{
		parents.insert(parents.begin(), thisNode);
		ColumnSet relevantColumns = data.getColumns(parents);
		parameters = learnDistributions(relevantColumns);
	}
private:
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
	std::map<std::vector<int>, Distribution> parameters;
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
			nodes[i].parameters.learn(datas, nodes[i].parents, nodes[i].ownVariable);
		}
	}
	std::vector<unsigned char> encodeDataset(const ColumnSet& datas)
	{
		ArithmeticEncoder encoder;
		for (int i = 0; i < nodes.size(); i++)
		{
			encoder.encode(encodeNode(datas, i));
		}
		return encoder.getBytes();
	}
	ColumnSet decodeBytes(const std::vector<unsigned char>& bytes, int rows)
	{
		ColumnSet ret {nodes.size(), rows};
		ArithmeticDecoder decoder {bytes};
		for (int i = 0; i < nodes.size(); i++)
		{
			decodeNode(ret, decoder, i);
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
private:
	void decodeNode(ColumnSet& out, ArithmeticDecoder& decoder, int node)
	{
		std::vector<int> columns = nodes[node].parents;
		ColumnSet limitedSet = out.getColumns(columns);
		for (int i = 0; i < out.rows(); i++)
		{
			std::vector<int> values;
			for (int a = 0; a < limitedSet.columns(); a++)
			{
				values.push_back(limitedSet[a][i]);
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
		for (int i = 0; i < datas.rows(); i++)
		{
			std::vector<int> values;
			for (int a = 1; a < limitedSet.columns(); a++)
			{
				values.push_back(limitedSet[a][i]);
			}
			encoder.encode(nodes[node].parameters.getDistribution(values), limitedSet[0][i]);
		}
		return encoder;
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

int testMode(char** argv)
{
	if (*argv[1] == 'c')
	{
		std::cerr << argv[2]<< "\n";
		ColumnSet testData = readTestData(argv[2], 4, 2000);

		BayesParameters node{testData, {1, 2, 3}, 0};

		ArithmeticEncoder encoder;

		for (int i = 0; i < testData.rows(); i++)
		{
			encoder.encode(node.getDistribution({testData[1][i], testData[2][i], testData[3][i]}), testData[0][i]);
		}

		std::vector<unsigned char> bytes = encoder.getBytes();

		std::ofstream out {argv[4], std::ios::binary};
		uint32_t size = bytes.size();
		std::cerr << size << "\n";
		out.write((char*)&size, 4);
		bytes = encoder.getBytes();
		for (int i = 0; i < 15; i++)
		{
			std::cerr << (int)bytes[i] << "\n";
		}
		out.write((char*)bytes.data(), bytes.size());
	}
	else
	{
		ColumnSet testData = readTestData(argv[2], 4, 2000);

		BayesParameters node{testData, {1, 2, 3}, 0};

		std::vector<unsigned char> bytes = getFileBytes(argv[3]);
		uint32_t size;
		memcpy(&size, bytes.data(), 4);
		std::cerr << size << "\n";
		bytes.erase(bytes.begin(), bytes.begin()+4);
		for (int i = 0; i < 15; i++)
		{
			std::cerr << (int)bytes[i] << "\n";
		}

		ArithmeticDecoder decoder(bytes);

		std::ofstream out {argv[4]};

		for (int i = 0; i < testData.rows(); i++)
		{
			unsigned char thisNode;
			thisNode = decoder.getSymbol(node.getDistribution({testData[1][i], testData[2][i], testData[3][i]}));
			out << (int)thisNode << "\n";
		}
	}
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

BayesNetwork makeDefaultNetwork(ColumnSet& data)
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
		links.emplace_back(i+21, mainType);
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
	links.emplace_back(21, 71); //farmer vs farmer equipment insurance
	links.emplace_back(21, 72);
	links.emplace_back(21, 73);
	links.emplace_back(21, 74);
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
		order.push_back(i+26);
		order.push_back(i);
	}

	for (int i = 0; i < data.columns(); i++)
	{
		links.emplace_back(0, i);
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
	for (int i = data.columns(); i >= 0; i--)
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

int caravanMode(char** argv)
{
	DEBUG_CARAVAN_LINES = atoi(argv[6]);
	if (*argv[1] == 'c')
	{
		ColumnSet data = readCaravan(argv[2], argv[3]);
		BayesNetwork net = makeDefaultNetwork(data);

		std::vector<unsigned char> bytes = net.encodeDataset(data);
		std::ofstream out {argv[4], std::ios::binary};
		uint32_t size = bytes.size();
		out.write((char*)&size, 4);
		out.write((char*)bytes.data(), bytes.size());
	}
	else
	{
		ColumnSet data = readCaravan(argv[2], argv[3]);
		BayesNetwork net = makeDefaultNetwork(data);

		std::vector<unsigned char> bytes = getFileBytes(argv[4]);
		uint32_t size;
		memcpy(&size, bytes.data(), 4);
		bytes.erase(bytes.begin(), bytes.begin()+4);

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
	caravanMode(argv);
}