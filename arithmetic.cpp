#include <gmpxx.h>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <fstream>

std::vector<mpq_class> counts;
std::vector<mpq_class> cumulativeCounts;
mpq_class totalCount;

std::vector<char> bytes;

void initTotals()
{
    mpq_class cumulativeTotal;
    for (size_t i = 0; i < counts.size(); i++)
    {
        cumulativeCounts.push_back(cumulativeTotal);
        cumulativeTotal += counts[i];
    }
    totalCount = cumulativeTotal;
}

void initLimits(std::ifstream& in)
{
	bool used[256];
	for (int i = 0; i < 256; i++)
	{
		used[i] = false;
	}
	std::vector<size_t> countsCalc;
	countsCalc.resize(256);
	unsigned char a = in.get();
	while (in.good())
	{
		if (!used[a])
		{
			used[a] = true;
		}
		countsCalc[a]++;
		a = in.get();
	}

	for (int i = 0; i < 256; i++)
	{
		if (used[i])
		{
			bytes.push_back(i);
			counts.push_back(countsCalc[i]);
		}
	}
	initTotals();
}

std::tuple<mpq_class, mpq_class, uint32_t> encode(std::istream& in)
{
    std::unordered_map<char, mpq_class> limits;
    std::unordered_map<char, mpq_class> widths;
    for (size_t i = 0; i < counts.size(); i++)
    {
        widths[bytes[i]] = counts[i]/totalCount;
        limits[bytes[i]] = cumulativeCounts[i]/totalCount;
    }
    mpq_class low = 0;
    mpq_class width = 1;
    uint32_t read = 0;
    char a = in.get();
    while (in.good())
    {
    	read++;
        low += limits[a]*width;
        width *= widths[a];
        a = in.get();
    }
    return std::tuple<mpq_class, mpq_class, uint32_t>(low, width, read);
}

void mpqToBytes(mpq_class pos, mpq_class width, std::ostream& out)
{
	pos += width*mpq_class(255, 256.0);
	while (width < 256)
	{
		pos *= 256;
		unsigned char write = pos.get_d();
		pos -= write;
		out.write((char*)&write, 1);
		width *= 256;
	}
}

void writeLimits(std::ostream& out)
{
	unsigned char used = bytes.size();
	out.write((char*)&used, 1);
	for (int i = 0; i < bytes.size(); i++)
	{
		out.write(&bytes[i], 1);
		uint32_t count = counts[i].get_num().get_si();
		out.write((char*)&count, 4);
	}
}

void compress(std::istream& in, std::ostream& out)
{
	writeLimits(out);
	auto x = encode(in);
	out.write((char*)&std::get<2>(x), 4);
	mpqToBytes(std::get<0>(x), std::get<1>(x), out);
}

void readLimits(std::istream& in)
{
	unsigned char used;
	in.read((char*)&used, 1);
	for (int i = 0; i < used; i++)
	{
		char a = in.get();
		uint32_t count;
		in.read((char*)&count, 4);
		bytes.push_back(a);
		counts.push_back(count);
	}
	initTotals();
}

void decode(mpq_class low, size_t numBytes, std::ostream& out)
{
    std::vector<mpq_class> limits = cumulativeCounts;
    limits.push_back(totalCount);
    for (mpq_class& l : limits)
    {
    	l /= totalCount;
    }
    for (size_t x = 0; x < numBytes; x++)
    {
        for (size_t i = 0; i < limits.size(); i++)
        {
            if (low >= limits[i] && low < limits[i+1])
            {
                out.put(bytes[i]);
                low -= limits[i];
                mpq_class diff = limits[i+1]-limits[i];
                low /= diff;
                break;
            }
        }
    }
}

mpq_class bytesToMpq(std::istream& in)
{
	mpq_class ret;
	mpq_class width = 1.0/256.0;
	unsigned char a = in.get();
	while (in.good())
	{
		ret += a*width;
		width /= 256;
		a = in.get();
	}
	return ret;
}

void decompress(std::istream& in, std::ostream& out)
{
	uint32_t len;
	in.read((char*)&len, 4);
	mpq_class num = bytesToMpq(in);
	decode(num, len, out);
}

int main(int argc, char** argv)
{
    std::ifstream in{argv[2], std::ios_base::in | std::ios_base::binary};
    std::ofstream out{argv[3], std::ios_base::out | std::ios_base::binary};
    if (*argv[1] == 'c')
    {
    	std::ifstream readCounts{argv[2], std::ios_base::in | std::ios_base::binary};
	    initLimits(readCounts);
    	compress(in, out);
    }
    else
    {
    	readLimits(in);
    	decompress(in, out);
    }
}
