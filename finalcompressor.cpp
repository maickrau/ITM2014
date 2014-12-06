#include <cmath>
#include <fstream>
#include <cstring>
#include <iostream>
#include <vector>

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
		if (y > 0.45)
		{
			return 1;
		}
		return 0;
	}
	return 2;
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

void findOptimalMode(char** argv)
{
	std::cout << "load data\n";
	auto data = loadData(argv[1], argv[2]);
	std::vector<std::vector<double>> topData;
	std::vector<std::vector<double>> middleData;
	std::vector<std::vector<double>> bottomData;
	BezierSurface topSurface;
	BezierSurface middleSurface;
	BezierSurface bottomSurface;
	std::cout << "split\n";
	for (int i = 0; i < data.size(); i++)
	{
		int type = classify(data[i][0], data[i][1], data[i][2]);
		if (type == 0)
		{
			topData.push_back(data[i]);
		}
		else if (type == 1)
		{
			middleData.push_back(data[i]);
		}
		else
		{
			bottomData.push_back(data[i]);
		}
	}
	std::cout << "cull\n";
	auto topDataPart = topData;
	topDataPart.erase(topDataPart.begin()+500, topDataPart.end());
	auto middleDataPart = middleData;
	middleDataPart.erase(middleDataPart.begin()+500, middleDataPart.end());
	auto bottomDataPart = bottomData;
	bottomDataPart.erase(bottomDataPart.begin()+500, bottomDataPart.end());
	std::cout << "fit top\n";
	topSurface.fit(topDataPart, 1000, 1, 0.99);
	std::cout << "fit middle\n";
	middleSurface.fit(middleDataPart, 1000, 1, 0.99);
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
	std::cout << middleSurface.getScore(middleData) << "\n";
	std::cout << middleSurface.maxOffset(middleData) << "\n";
	for (int i = 0; i < 9; i++)
	{
		std::cout << middleSurface.height[i] << "\n";
	}
	std::cout << "\n";
	std::cout << bottomSurface.getScore(bottomData) << "\n";
	std::cout << bottomSurface.maxOffset(bottomData) << "\n";
	for (int i = 0; i < 9; i++)
	{
		std::cout << bottomSurface.height[i] << "\n";
	}
	std::cout << "\n";
}

int main(int argc, char** argv)
{
	findOptimalMode(argv);
}