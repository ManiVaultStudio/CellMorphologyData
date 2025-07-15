#pragma once

#include "CellMorphologyData_export.h"

#include "graphics/Vector3f.h"

#include <QHash>
#include <QVariantMap>

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

    static Type TypeFromInt(int value)
    {
        switch (value)
        {
        case 1: return Type::Soma;
        case 2: return Type::Axon;
        case 3: return Type::BasalDendrite;
        case 4: return Type::ApicalDendrite;
        case 5: return Type::Custom;
        case 6: return Type::UnspecifiedNeurite;
        case 7: return Type::GliaProcesses;
        default: return Type::UnspecifiedNeurite;
        }
    }

    class CELLMORPHOLOGYDATA_EXPORT Extent
    {
    public:
        void Extend(Extent extent);

    public:
        QVariantMap toVariantMap() const;
        void fromVariantMap(const QVariantMap& map);

    public:
        mv::Vector3f emin;
        mv::Vector3f emax;
        mv::Vector3f center;
    };

public:
    CellMorphology();

    void findCentroid();
    void findExtents();
    void center();
    //void rescale();

    std::vector<int> ids;
    std::unordered_map<int, int> idMap;
    std::vector<mv::Vector3f> positions;
    std::vector<int> types;
    std::vector<float> radii;
    std::vector<int> parents;

    mv::Vector3f somaPosition;
    mv::Vector3f centroid;

    QHash<Type, Extent> extents;

    mv::Vector3f cellTypeColor;

private:

};
