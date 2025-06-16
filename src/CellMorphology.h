#pragma once

#include "CellMorphologyData_export.h"

#include "graphics/Vector3f.h"

#include <vector>
#include <unordered_map>

class CELLMORPHOLOGYDATA_EXPORT CellMorphology
{
public:
    enum class Type
    {
        Undefined = 0,
        Soma = 1,
        Axon = 2,
        BasalDendrite = 3,
        ApicalDendrite = 4,
        Custom = 5,
        UnspecifiedNeurite = 6,
        GliaProcesses = 7
    };

public:
    CellMorphology();

    void findCentroid();
    void findExtents();
    void center();
    void rescale();

    std::vector<int> ids;
    std::unordered_map<int, int> idMap;
    std::vector<mv::Vector3f> positions;
    std::vector<int> types;
    std::vector<float> radii;
    std::vector<int> parents;

    mv::Vector3f somaPosition;
    mv::Vector3f centroid;
    mv::Vector3f minRange;
    mv::Vector3f maxRange;

    mv::Vector3f cellTypeColor;

private:

};
