#pragma once
#include <vector>
#include <cmath>
#include <string>

// Store vertex data
struct shapeVertex
{
    // [x,y,z]
    float pos[3];
    // Generated according to distance from peak frequencies. Range [0,1]
    float alpha;
};

// Class for generating custom object utilizing frequencies
class shapeGen
{
private:
    // Store current shape
    std::vector<shapeVertex> vertecies;
    std::vector<int> indexes;
    float scale=5.0;

    // Expected complexity of shape, final shape expected to have segCnt*segCnt vertecies
    int segCnt;

    // Error storage
    std::string errString;

public:
    /// @brief Initialize generator
    shapeGen();
    /// @brief Generate a fresh shape
    void generate();
    /// @brief Change number of generated segments, min 20
    /// @param count    New segment count, default to 360
    void setSegCnt(int count=360);
    /// @brief Overwrite given vectors with current shape
    /// @param vertBuff     Buffer for vertecies
    /// @param indBuff      Buffer for Indexes
    /// @return             Returns false on failure or empty data. Call getErr() for more info.
    bool getLatestShape(std::vector<shapeVertex>& vertBuff,std::vector<int>& indBuff);
    /// @brief Get current size of vertex vector.
    /// @return     vertecies.size()
    int getVertCnt();
    float getScale();
    void setScale(float newS=10.0);
    /// @brief Fetch latest error
    /// @return     Error string
    std::string getErr();
    ~shapeGen();
};

