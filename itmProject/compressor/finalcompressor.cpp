#include <cmath>
#include <fstream>
#include <cstring>
#include <iostream>
#include <vector>
#include <cassert>

class BitWriter
{
public:
	BitWriter(FILE* s) : totalWritten(0), stream(s), bitLoc(0), nextBits(0) {};
	~BitWriter()
	{
		flushWrite();
	};
	void writeBits(char* target, int bits);
	size_t written()
	{
		return totalWritten;
	};
private:
	size_t totalWritten;
	void flushWrite();
	FILE* stream;
	int bitLoc;
	char nextBits;
};

class BitReader
{
public:
	BitReader(FILE* s) : stream(s), bitLoc(0), nextBits(0) {};
	void readBits(char* target, int bits);
	bool eof()
	{
		return feof(stream);
	};
private:
	FILE* stream;
	int bitLoc;
	int nextBits;
};

void BitReader::readBits(char* target, int bits)
{
	int written = 0;
	while (written < bits)
	{
		if (bitLoc == 0)
		{
			nextBits = fgetc(stream);
		}
		if (written % 8 == 0)
		{
			*target = 0;
		}
		*target |= (nextBits&(1<<bitLoc) ? 1 : 0) << written%8;
		bitLoc++;
		bitLoc %= 8;
		written++;
		if (written % 8 == 0)
		{
			target++;
		}
	}
}

void BitWriter::writeBits(char* target, int bits)
{
	totalWritten += bits;
	int read = 0;
	while (read < bits)
	{
		if (bitLoc == 0)
		{
			nextBits = 0;
		}
		nextBits |= ((*target)&(1<<(read%8)) ? 1 : 0) << bitLoc;
		bitLoc++;
		bitLoc %= 8;
		if (bitLoc == 0)
		{
			fputc(nextBits, stream);
		}
		read++;
		if (read % 8 == 0)
		{
			target++;
		}
	}
}

void BitWriter::flushWrite()
{
	if (bitLoc != 0)
	{
		fputc(nextBits, stream);
	}
}

class BezierSurface
{
public:
	BezierSurface()
	{
		for (int i = 0; i < 9; i++)
		{
			height[i] = 0;
		}
	}
	double getValue(double xAbsolute, double yAbsolute)
	{
		double ret = 0;
		double x = (xAbsolute+4)/8;
		double y = (yAbsolute+4)/5.35;
		ret += (1-x)*(1-x)*(1-y)*(1-y)*height[0];
		ret += 2*(x)*(1-x)*(1-y)*(1-y)*height[1];
		ret += (x)*(x)*(1-y)*(1-y)*height[2];
		ret += 2*(1-x)*(1-x)*(y)*(1-y)*height[3];
		ret += 2*2*(x)*(1-x)*(y)*(1-y)*height[4];
		ret += 2*(x)*(x)*(y)*(1-y)*height[5];
		ret += (1-x)*(1-x)*(y)*(y)*height[6];
		ret += 2*(x)*(1-x)*(y)*(y)*height[7];
		ret += (x)*(x)*(y)*(y)*height[8];
		return ret;
	};
	int getValueInt(double xAbsolute, double yAbsolute)
	{
		return getValue(xAbsolute, yAbsolute)*1000;
	}
	void fit(const std::vector<std::vector<double>>& values, int iterations, double wildness, double decay)
	{
		for (int iter = 0; iter < iterations; iter++)
		{
			for (int point = 0; point < 9; point++)
			{
				double scoreStart = getScore(values);
				height[point] += wildness;
				double scoreComp = getScore(values);
				if (scoreComp < scoreStart)
				{
					height[point] -= wildness*2;
				}
			}
			wildness *= decay;
		}
	};
	double getScore(const std::vector<std::vector<double>>& values)
	{
		double ret = 0;
		for (int i = 0; i < values.size(); i++)
		{
			double diff = getValue(values[i][0], values[i][1])-values[i][2];
			ret -= diff*diff;
		}
		return ret;
	}
	double maxOffset(const std::vector<std::vector<double>>& values)
	{
		double diff = getValue(values[0][0], values[0][1])-values[0][2];
		double ret = std::abs(diff);
		for (int i = 1; i < values.size(); i++)
		{
			diff = getValue(values[i][0], values[i][1])-values[i][2];
			if (std::abs(diff) > ret)
			{
				ret = std::abs(diff);
			}
		}
		return ret;
	}

	std::vector<double> differences(const std::vector<std::vector<double>>& values)
	{
		std::vector<double> ret;
		for (int i = 0; i < values.size(); i++)
		{
			double diff = getValue(values[i][0], values[i][1])-values[i][2];
			ret.push_back(diff);
		}
		return ret;
	}

	double height[9];
};

bool divide(double x, double y, double z)
{
	double t = (x+4.0)/8.0;
	double cutoff = t*t*14.0+2.0*t*(1.0-t)*(-16.0)+(1.0-t)*(1.0-t)*(-7.6);
	return z > cutoff;
}

int classify(double x, double y, double z)
{
	if (divide(x, y, z))
	{
		return 0;
	}
	return 1;
}

std::vector<std::vector<double>> loadData(std::string dataFile, std::string sideFile)
{
	std::ifstream side {sideFile};
	std::ifstream data {dataFile};

	std::vector<std::vector<double>> ret;

	for (int i = 0; i < 20000; i++)
	{
		double x, y, z;
		side >> x;
		side >> y;
		data >> z;
		ret.emplace_back();
		ret.back().push_back(x);
		ret.back().push_back(y);
		ret.back().push_back(z);
	}

	return ret;
}

std::vector<int> loadDataExact(std::string dataFile)
{
	std::ifstream data {dataFile};
	std::vector<int> ret;
	for (int i = 0; i < 20000; i++)
	{
		int sign = 1;
		while (data.peek() != '-' && (data.peek() < '0' || data.peek() > '9'))
		{
			data.get();
		}
		if (data.peek() == '-')
		{
			sign = -1;
			data.get();
		}
		int left = 0, right = 0;
		data >> left;
		if (data.peek() == '.')
		{
			char a = data.get();
			a = data.get();
			right = 100*(a-'0');
			if (data.peek() != '\n')
			{
				a = data.get();
				right += 10*(a-'0');
				if (data.peek() != '\n')
				{
					a = data.get();
					right += (a-'0');
				}
			}
		}
		if (sign < 0)
		{
			left *= -1;
			right *= -1;
		}
		ret.push_back(left*1000+right);
	}
	return ret;
}

std::vector<std::vector<double>> loadSideData(std::string sideFile)
{
	std::ifstream side {sideFile};

	std::vector<std::vector<double>> ret;

	for (int i = 0; i < 20000; i++)
	{
		double x, y;
		side >> x;
		side >> y;
		ret.emplace_back();
		ret.back().push_back(x);
		ret.back().push_back(y);
	}

	return ret;
}

void findOptimalMode(char** argv)
{
	std::cout << "load data\n";
	auto data = loadData(argv[1], argv[2]);
	std::vector<std::vector<double>> topData;
	std::vector<std::vector<double>> bottomData;
	BezierSurface topSurface;
	BezierSurface bottomSurface;
	std::cout << "split\n";
	for (int i = 0; i < data.size(); i++)
	{
		int type = classify(data[i][0], data[i][1], data[i][2]);
		if (type == 0)
		{
			topData.push_back(data[i]);
		}
		else
		{
			bottomData.push_back(data[i]);
		}
	}
	std::cout << "cull\n";
	auto topDataPart = topData;
	topDataPart.erase(topDataPart.begin()+500, topDataPart.end());
	auto bottomDataPart = bottomData;
	bottomDataPart.erase(bottomDataPart.begin()+500, bottomDataPart.end());
	std::cout << "fit top\n";
	topSurface.fit(topDataPart, 1000, 1, 0.99);
	std::cout << "fit bottom\n";
	bottomSurface.fit(bottomDataPart, 1000, 1, 0.99);
	std::cout << "scores\n";
	std::cout << topSurface.getScore(topData) << "\n";
	std::cout << topSurface.maxOffset(topData) << "\n";
	for (int i = 0; i < 9; i++)
	{
		std::cout << topSurface.height[i] << "\n";
	}
	std::cout << "\n";
	std::cout << bottomSurface.getScore(bottomData) << "\n";
	std::cout << bottomSurface.maxOffset(bottomData) << "\n";
	for (int i = 0; i < 9; i++)
	{
		std::cout << bottomSurface.height[i] << "\n";
	}
	std::cout << "\n";

	std::ofstream out{argv[3]};
	auto diffs = topSurface.differences(topData);
	for (int i = 0; i < diffs.size(); i++)
	{
		out << diffs[i] << "\n";
	}
	diffs = bottomSurface.differences(bottomData);
	for (int i = 0; i < diffs.size(); i++)
	{
		out << diffs[i] << "\n";
	}
}

BezierSurface fit(std::vector<std::vector<double>> data, int points)
{
	data.erase(data.begin()+points, data.end());
	BezierSurface surface;
	surface.fit(data, 1000, 1, 0.99);
	return surface;
}

void encodeDifferencesMode(char** argv)
{
	std::vector<std::vector<double>> topData;
	std::vector<std::vector<double>> bottomData;
	std::vector<int> dataExact = loadDataExact(argv[1]);
	auto data = loadData(argv[1], argv[2]);
	for (int i = 0; i < data.size(); i++)
	{
		int type = classify(data[i][0], data[i][1], data[i][2]);
		if (type == 0)
		{
			topData.push_back(data[i]);
		}
		else
		{
			bottomData.push_back(data[i]);
		}
	}
	BezierSurface top = fit(topData, 1000);
	BezierSurface bottom = fit(bottomData, 1000);

	FILE* writeFile = fopen(argv[3], "wb");
	BitWriter writer {writeFile};

	for (int i = 0; i < 9; i++)
	{
		writer.writeBits((char*)&top.height[i], sizeof(double)*8);
		writer.writeBits((char*)&bottom.height[i], sizeof(double)*8);
	}

	for (int i = 0; i < data.size(); i++)
	{
		int classification = classify(data[i][0], data[i][1], data[i][2]);
		int measure;
		if (classification == 0)
		{
			measure = top.getValueInt(data[i][0], data[i][1]);
		}
		else
		{
			measure = bottom.getValueInt(data[i][0], data[i][1]);
		}
		int dataHeight = dataExact[i];
		int diff = dataHeight-measure;
		writer.writeBits((char*)&classification, 1);
		int sign = diff < 0;
		writer.writeBits((char*)&sign, 1);
		if (diff < 0)
		{
			diff *= -1;
		}
		assert(diff < 4096);
		writer.writeBits((char*)&diff, 12);
	}
	fclose(writeFile);
}

void decodeDifferencesMode(char** argv)
{
	BezierSurface top;
	BezierSurface bottom;
	FILE* readFile = fopen(argv[3], "rb");
	BitReader reader {readFile};

	auto sideData = loadSideData(argv[2]);

	for (int i = 0; i < 9; i++)
	{
		reader.readBits((char*)&top.height[i], sizeof(double)*8);
		reader.readBits((char*)&bottom.height[i], sizeof(double)*8);
	}

	for (int i = 0; i < 20000; i++)
	{
		int classification = 0;
		int sign = 0;
		int n = 0;
		reader.readBits((char*)&classification, 1);
		reader.readBits((char*)&sign, 1);
		reader.readBits((char*)&n, 12);
		if (sign)
		{
			n *= -1;
		}
		int measure;
		if (classification == 0)
		{
			measure = top.getValueInt(sideData[i][0], sideData[i][1]);
		}
		else
		{
			measure = bottom.getValueInt(sideData[i][0], sideData[i][1]);
		}
		int height = measure+n;
		double heightD = height/1000.0;
		printf("%1.3f\n", heightD);
	}
	fclose(readFile);
}

int main(int argc, char** argv)
{
	if (*argv[4] == 'c')
	{
		encodeDifferencesMode(argv);
	}
	else
	{
		decodeDifferencesMode(argv);
	}
}