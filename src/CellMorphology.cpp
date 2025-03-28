#include "CellMorphology.h"

#include <iostream>

using namespace mv;

CellMorphology::CellMorphology() :
    somaPosition(0, 0, 0),
    centroid(0, 0, 0),
    minRange(std::numeric_limits<float>::max()),
    maxRange(-std::numeric_limits<float>::max()),
    cellTypeColor(0.706f, 0.22f, 0.38f)
{

}

void CellMorphology::findCentroid()
{
    centroid.set(0, 0, 0);
    for (const auto& pos : positions)
        centroid += pos;
    centroid /= positions.size();
}

void CellMorphology::findExtents()
{
    // Find cell position extents
    minRange = Vector3f(std::numeric_limits<float>::max());
    maxRange = Vector3f(-std::numeric_limits<float>::max());
    for (const auto& pos : positions)
    {
        if (pos.x < minRange.x) minRange.x = pos.x;
        if (pos.y < minRange.y) minRange.y = pos.y;
        if (pos.z < minRange.z) minRange.z = pos.z;
        if (pos.x > maxRange.x) maxRange.x = pos.x;
        if (pos.y > maxRange.y) maxRange.y = pos.y;
        if (pos.z > maxRange.z) maxRange.z = pos.z;
    }
}

void CellMorphology::center()
{
    // Find centroid
    findCentroid();

    // Center cell positions
    for (auto& pos : positions)
        pos -= centroid;
}

void CellMorphology::rescale()
{
    // Find cell position extents
    findExtents();

    Vector3f range = (maxRange - minRange);
    float maxRange = std::max(std::max(range.x, range.y), range.z);
    // Rescale positions
    for (auto& pos : positions)
        pos /= maxRange;

    //std::cout << minV.str() << " " << maxV.str() << std::endl;
}
