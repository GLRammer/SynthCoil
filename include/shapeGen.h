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

    // Store latest <freqCount> loudest frequencies with their magnitude.
    // Values are normalized to range [0,9]
    std::vector<std::pair<float, float>> freqVec;

    // Number of frequencies to account for during shape generation
    int freqCount;

    // Max value for frequency inputs, used for normalizing values
    int freqRange;

    // Expected complexity of shape, final shape expected to have segCnt*segCnt vertecies
    int segCnt;

    // Bump smoothing range
    float smooth;

    // Error storage
    std::string errString;

    // Internal shape generation
    void generate();

    // Helper functions for shape math
    float bumper(float x);
    float alpha(float x);

public:
    /// @brief Initialize generator
    /// @param count    Optional number of points to grab from vectors, default to 5
    /// @param range    Optional frequency range, default to 2205
    shapeGen(int count = 5, int range = 2205);
    /// @brief Initialize with data
    /// @param freqs    Vector of frequencies
    /// @param mags     Vector of magnitudes matching the vector of frequencies
    /// @param count    Optional number of points to grab from vectors, default to 5
    /// @param range    Optional frequency range, default to 2205
    shapeGen(std::vector<float> freqs, std::vector<float> mags, int count = 5, int range = 2205);
    /// @brief Input fresh data to generate a new shape
    /// @param freqs    Vector of frequencies
    /// @param mags     Vector of magnitudes matching the vector of frequencies
    /// @return         Returns false on failure or empty data. Call getErr() for more info.
    bool generate(std::vector<float> freqs, std::vector<float> mags);
    /// @brief Change number of generated segments, min 20
    /// @param count    New segment count, default to 360
    void setSegCnt(int count = 360);
    /// @brief Change shape smoothing value, range of [0,1] inclusive.
    /// @param newsmooth    New smoothing value, default of .5
    /// @return             Return false for input out of range.
    bool setSmooth(float newsmooth = .5);
    /// @brief Change number of points to grab from vectors
    /// @param count    Number of points to grab from vectors
    void setFreqCnt(int count);
    /// @brief Overwrite given vectors with current shape
    /// @param vertBuff     Buffer for vertecies
    /// @param indBuff      Buffer for Indexes
    /// @return             Returns false on failure or empty data. Call getErr() for more info.
    bool getLatestShape(std::vector<shapeVertex> &vertBuff, std::vector<int> &indBuff);
    /// @brief Get current size of vertex vector.
    /// @return     vertecies.size()
    int getVertCnt();
    float getSmooth();
    /// @brief Fetch latest error
    /// @return     Error string
    std::string getErr();
    ~shapeGen();
};
