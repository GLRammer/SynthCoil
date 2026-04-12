#include "shapeGen.h"

void shapeGen::generate()
{
    vertecies.clear();
    indexes.clear();
    vertecies.reserve(segCnt * segCnt);
    indexes.reserve(segCnt * segCnt * 6);
    float outterRad = scale;
    float u, cu, su, v, dv, x, y, z;
    int i, j, nextI, nextJ;

    for (i = 0; i < segCnt; i++)
    {
        u = (float)i / (float)segCnt * 2.0f * M_PI;
        cu = std::cos(u);
        su = std::sin(u);
        nextI = (i + 1) % segCnt;
        for (j = 0; j < segCnt; j++)
        {
            v = (float)j / (float)segCnt;
            dv = v*outterRad;
            x = dv * cu;
            z = dv * su;
            y=0.0f;

            shapeVertex vert;
            vert.pos[0] = x;
            vert.pos[1] = y;
            vert.pos[2] = z;
            vert.alpha=0;
            vertecies.push_back(vert);
            nextJ = (j + 1) % segCnt;
            if (nextJ!=0){
                indexes.push_back(i * segCnt + j);
                indexes.push_back(nextI * segCnt + j);
                indexes.push_back(nextI * segCnt + nextJ);
                indexes.push_back(i * segCnt + j);
                indexes.push_back(nextI * segCnt + nextJ);
                indexes.push_back(i * segCnt + nextJ);
            }
        }
    }
}

// Barebones initializing
shapeGen::shapeGen()
{
    errString = "";
    segCnt = 360;
    vertecies.clear();
    indexes.clear();
    generate();
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

float shapeGen::getScale(){return scale;}

void shapeGen::setScale(float newS){
    if(newS>1){
        scale=newS;
    }else{
        scale=1;
    }
}

std::string shapeGen::getErr()
{
    return errString;
}

shapeGen::~shapeGen()
{
}
