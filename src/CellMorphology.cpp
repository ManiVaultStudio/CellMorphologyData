#include "CellMorphology.h"

#include <iostream>

using namespace mv;

CellMorphology::CellMorphology() :
    somaPosition(0, 0, 0),
    centroid(0, 0, 0),
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
    for (int i = 0; i < positions.size(); i++)
    {
        Type type = TypeFromInt(types[i]);

        bool newlyCreated = !extents.contains(type);
        Extent& extent = extents[type];
        if (newlyCreated)
        {
            extent.emin = Vector3f(std::numeric_limits<float>::max());
            extent.emax = Vector3f(-std::numeric_limits<float>::max());
        }
        
        const auto& pos = positions[i];
        if (pos.x < extent.emin.x) extent.emin.x = pos.x;
        if (pos.y < extent.emin.y) extent.emin.y = pos.y;
        if (pos.z < extent.emin.z) extent.emin.z = pos.z;
        if (pos.x > extent.emax.x) extent.emax.x = pos.x;
        if (pos.y > extent.emax.y) extent.emax.y = pos.y;
        if (pos.z > extent.emax.z) extent.emax.z = pos.z;
    }

    for (auto it = extents.begin(); it != extents.end(); ++it)
    {
        it.value().center = (it.value().emin + it.value().emax) / 2;
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

//void CellMorphology::rescale()
//{
//    // Find cell position extents
//    findExtents();
//
//    Vector3f range = (maxRange - minRange);
//    float maxRange = std::max(std::max(range.x, range.y), range.z);
//    // Rescale positions
//    for (auto& pos : positions)
//        pos /= maxRange;
//
//    //std::cout << minV.str() << " " << maxV.str() << std::endl;
//}

void CellMorphology::Extent::Extend(CellMorphology::Extent extent)
{
    emin = min(emin, extent.emin);
    emax = max(emax, extent.emax);
    center = (emin + emax) / 2;
}

QVariantMap CellMorphology::Extent::toVariantMap() const
{
    QVariantMap map;
    map["min"] = emin.toVariantMap();
    map["max"] = emax.toVariantMap();
    map["center"] = center.toVariantMap();
    return map;
}

void CellMorphology::Extent::fromVariantMap(const QVariantMap& map)
{
    emin.fromVariantMap(map["min"].toMap());
    emax.fromVariantMap(map["max"].toMap());
    center.fromVariantMap(map["center"].toMap());
}
