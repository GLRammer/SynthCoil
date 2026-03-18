#include "shapeGen.h"

void shapeGen::generate()
{
    vertecies.clear();
    indexes.clear();
    vertecies.reserve(segCnt * segCnt);
    indexes.reserve(segCnt * segCnt * 6);
    float innerRad = 1, outterRad = 10;
    float u, cu, su, v, cv, sv, x, y, z;
    int i, j, nextI, nextJ;
    for (i = 0; i < segCnt; i++)
    {
        u = (float)i / (float)segCnt * 2.0f * M_PI;
        cu = std::cos(u);
        su = std::sin(u);
        nextI = (i + 1) % segCnt;
        for (j = 0; j < segCnt; j++)
        {
            v = (float)j / (float)segCnt * 2.0f * M_PI;
            cv = std::cos(v);
            sv = std::sin(v);
            x = (outterRad + innerRad * cv) * cu;
            y = (outterRad + innerRad * cv) * su;
            //  z=innerRad*su;
            z = bumper(x);

            shapeVertex vert;
            vert.pos[0] = x;
            vert.pos[1] = y;
            vert.pos[2] = z;
            vert.alpha = alpha(x);
            vertecies.push_back(vert);
            nextJ = (j + 1) % segCnt;
            indexes.push_back(i * segCnt + j);
            indexes.push_back(nextI * segCnt + j);
            indexes.push_back(nextI * segCnt + nextJ);
            indexes.push_back(i * segCnt + j);
            indexes.push_back(nextI * segCnt + nextJ);
            indexes.push_back(i * segCnt + nextJ);
        }
    }
}

// Use Gaussian bump to shape z axis
float shapeGen::bumper(float x)
{
    float y = 0;
    for (auto &&point : freqVec)
    {
        float dx = x - point.first;
        float dy = point.second * std::exp(-(dx * dx) / smooth);
        if (dy > y)
        {
            y = dy;
        }
    }
    return y;
}

// Use gaussian bump for aplha channels on vertecies
float shapeGen::alpha(float x)
{
    float y = 0;
    for (auto &&point : freqVec)
    {
        float dx = x - point.first;
        float dy = 255 * std::exp(-(dx * dx) / smooth);
        if (dy > y)
        {
            y = dy;
        }
    }
    return y;
}

// Barebones initializing
shapeGen::shapeGen(int count, int range)
{
    freqCount = count;
    errString = "";
    freqRange = range;
    segCnt = 360;
    smooth = .5;
    vertecies.clear();
    indexes.clear();
}

// Initialize and immediately generate a new shape
shapeGen::shapeGen(std::vector<float> freqs, std::vector<float> mags, int count, int range)
{
    freqCount = count;
    errString = "";
    freqRange = range;
    segCnt = 360;
    smooth = .5;
    generate(freqs, mags);
}

// Public generate call for the user to input fresh values
bool shapeGen::generate(std::vector<float> freqs, std::vector<float> mags)
{
    int i;

    // Check input size
    if (freqs.size() < freqCount)
    {
        errString = "Vectors are too small!";
        return false;
    }

    // Check if inputs match
    if (freqs.size() != mags.size())
    {
        errString = "Vector sizes don't match!";
        return false;
    }

    // Temporary storage for loudest frequencies indexes
    std::vector<int> tempind(freqCount);

    // Find <freqCount> loudest frequencies and store their indexes
    for (i = 0; i < freqs.size(); i++)
    {
        if (tempind.size() < freqCount)
        {
            tempind.push_back(i);
            continue;
        }
        for (int j = 0; j < freqCount; j++)
        {
            if (mags[i] >= mags[tempind[j]])
            {
                tempind[j] = i;
            }
        }
    }

    // Clean freqVec for fresh data
    freqVec.clear();
    freqVec.reserve(freqCount);

    // Push fresh data to freqVec
    for (i = 0; i < freqCount; i++)
    {
        freqVec.push_back(std::pair<float, float>(9 * (freqs[tempind[i]] / freqRange), 9 * mags[tempind[i]]));
    }

    // Call internal generate() to build shape
    generate();
}

void shapeGen::setFreqCnt(int count)
{
    freqCount = count;
}

void shapeGen::setSegCnt(int count)
{
    if (count < 20)
    {
        segCnt = 20;
    }
    else
    {
        segCnt = count;
    }
}

bool shapeGen::setSmooth(float newsmooth)
{
    if (newsmooth > 1 || newsmooth < 0)
    {
        errString = "New smooth value out of range";
        return false;
    }
    smooth = newsmooth;
    return true;
}

bool shapeGen::getLatestShape(std::vector<shapeVertex> &vertBuff, std::vector<int> &indBuff)
{
    if (vertecies.size() <= 0)
    {
        errString = "No data available!";
        return false;
    }
    vertBuff = std::vector<shapeVertex>(vertecies);
    indBuff = std::vector<int>(indexes);
    return true;
}

int shapeGen::getVertCnt() { return vertecies.size(); }

float shapeGen::getSmooth() { return smooth; }

std::string shapeGen::getErr()
{
    return errString;
}

shapeGen::~shapeGen()
{
}
