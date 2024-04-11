#include "CellMorphologyData.h"
#include "Application.h"

#include "Util/Icon.h"

#include <util/Serialization.h>

#include <QtCore>
#include <QPainter>

#include <set>

Q_PLUGIN_METADATA(IID "studio.manivault.CellMorphologyData")

using namespace mv;
using namespace mv::util;

CellMorphologyData::~CellMorphologyData(void)
{
    
}

void CellMorphologyData::init()
{
}

/**
 * Create a new dataset linking back to the original raw data
 */
Dataset<DatasetImpl> CellMorphologyData::createDataSet(const QString& guid /*= ""*/) const
{
    return new CellMorphologies(getName(), guid);
}

std::vector<CellMorphology>& CellMorphologyData::getData()
{
    return _cellMorphologies;
}

void CellMorphologyData::setCellIdentifiers(const QStringList& cellIds)
{
    _cellIds = cellIds;
}

void CellMorphologyData::setData(const std::vector<CellMorphology>& cellMorphologies)
{
    _cellMorphologies = cellMorphologies;
}


void CellMorphologyData::fromVariantMap(const QVariantMap& variantMap)
{
    WidgetAction::fromVariantMap(variantMap);

    const auto dataMap = variantMap["Data"].toMap();

    variantMapMustContain(dataMap, "IdsRawData");
    variantMapMustContain(dataMap, "PositionsRawData");
    variantMapMustContain(dataMap, "TypesRawData");
    variantMapMustContain(dataMap, "RadiiRawData");
    variantMapMustContain(dataMap, "ParentsRawData");
    variantMapMustContain(dataMap, "IdMapKeysRawData");
    variantMapMustContain(dataMap, "IdMapValuesRawData");
    variantMapMustContain(dataMap, "NumberOfIds");
    variantMapMustContain(dataMap, "NumberOfPositions");
    variantMapMustContain(dataMap, "NumberOfTypes");
    variantMapMustContain(dataMap, "NumberOfRadii");
    variantMapMustContain(dataMap, "NumberOfParents");
    variantMapMustContain(dataMap, "NumberOfIdMapKeys");
    variantMapMustContain(dataMap, "NumberOfIdMapValues");

    // Packed ids for all cell morphologies
    std::vector<std::int32_t> packedIds;
    std::vector<mv::Vector3f> packedPositions;
    std::vector<std::int32_t> packedTypes;
    std::vector<float> packedRadii;
    std::vector<std::int32_t> packedParents;
    std::vector<std::int32_t> packedIdMapKeys;
    std::vector<std::int32_t> packedIdMapValues;

    packedIds.resize(dataMap["NumberOfIds"].toInt());
    packedPositions.resize(dataMap["NumberOfPositions"].toInt());
    packedTypes.resize(dataMap["NumberOfTypes"].toInt());
    packedRadii.resize(dataMap["NumberOfRadii"].toInt());
    packedParents.resize(dataMap["NumberOfParents"].toInt());
    packedIdMapKeys.resize(dataMap["NumberOfIdMapKeys"].toInt());
    packedIdMapValues.resize(dataMap["NumberOfIdMapValues"].toInt());

    // Convert raw data to original vectors
    populateDataBufferFromVariantMap(dataMap["IdsRawData"].toMap(), (char*)packedIds.data());
    populateDataBufferFromVariantMap(dataMap["PositionsRawData"].toMap(), (char*)packedPositions.data());
    populateDataBufferFromVariantMap(dataMap["TypesRawData"].toMap(), (char*)packedTypes.data());
    populateDataBufferFromVariantMap(dataMap["RadiiRawData"].toMap(), (char*)packedRadii.data());
    populateDataBufferFromVariantMap(dataMap["ParentsRawData"].toMap(), (char*)packedParents.data());
    populateDataBufferFromVariantMap(dataMap["IdMapKeysRawData"].toMap(), (char*)packedIdMapKeys.data());
    populateDataBufferFromVariantMap(dataMap["IdMapValuesRawData"].toMap(), (char*)packedIdMapValues.data());

    if (dataMap.contains("CellMorphologyRawData"))
    {
        QByteArray byteArray;

        QDataStream dataStream(&byteArray, QIODevice::ReadOnly);

        const auto cellMorphologyRawDataSize = dataMap["CellMorphologyRawDataSize"].toInt();

        byteArray.resize(cellMorphologyRawDataSize);

        populateDataBufferFromVariantMap(dataMap["CellMorphologyRawData"].toMap(), (char*)byteArray.data());

        QVariantList cms;

        dataStream >> cms;

        _cellMorphologies.resize(cms.count());

        long cmsIndex = 0;

        int idsOffset = 0;
        int positionsOffset = 0;
        int typesOffset = 0;
        int radiiOffset = 0;
        int parentsOffset = 0;
        int idMapKeysOffset = 0;
        int idMapValuesOffset = 0;

        for (const auto& cellMorphologyVariant : cms)
        {
            const auto cellMorphologyMap = cellMorphologyVariant.toMap();

            CellMorphology& cellMorphology = _cellMorphologies[cmsIndex];

            const auto numberOfIds = cellMorphologyMap["NumberOfIds"].toInt();
            const auto numberOfPositions = cellMorphologyMap["NumberOfPositions"].toInt();
            const auto numberOfTypes = cellMorphologyMap["NumberOfTypes"].toInt();
            const auto numberOfRadii = cellMorphologyMap["NumberOfRadii"].toInt();
            const auto numberOfParents = cellMorphologyMap["NumberOfParents"].toInt();
            const auto numberOfIdMapKeys = cellMorphologyMap["NumberOfIdMapKeys"].toInt();
            const auto numberOfIdMapValues = cellMorphologyMap["NumberOfIdMapValues"].toInt();

            cellMorphology.ids = std::vector<std::int32_t>(packedIds.begin() + idsOffset, packedIds.begin() + idsOffset + numberOfIds);
            cellMorphology.positions = std::vector<mv::Vector3f>(packedPositions.begin() + positionsOffset, packedPositions.begin() + positionsOffset + numberOfPositions);
            cellMorphology.types = std::vector<std::int32_t>(packedTypes.begin() + typesOffset, packedTypes.begin() + typesOffset + numberOfTypes);
            cellMorphology.radii = std::vector<float>(packedRadii.begin() + radiiOffset, packedRadii.begin() + radiiOffset + numberOfRadii);
            cellMorphology.parents = std::vector<std::int32_t>(packedParents.begin() + parentsOffset, packedParents.begin() + parentsOffset + numberOfParents);

            std::vector<int> idMapKeys(numberOfIdMapKeys);
            std::vector<int> idMapValues(numberOfIdMapValues);

            idMapKeys = std::vector<std::int32_t>(packedIdMapKeys.begin() + idMapKeysOffset, packedIdMapKeys.begin() + idMapKeysOffset + numberOfIdMapKeys);
            idMapValues = std::vector<std::int32_t>(packedIdMapValues.begin() + idMapValuesOffset, packedIdMapValues.begin() + idMapValuesOffset + numberOfIdMapValues);

            // Fill ID map
            for (size_t i = 0; i != idMapKeys.size(); ++i) {
                cellMorphology.idMap[idMapKeys[i]] = idMapValues[i];
            }

            idsOffset += numberOfIds;
            positionsOffset += numberOfPositions;
            typesOffset += numberOfTypes;
            radiiOffset += numberOfRadii;
            parentsOffset += numberOfParents;
            idMapKeysOffset += numberOfIdMapKeys;
            idMapValuesOffset += numberOfIdMapValues;

            ++cmsIndex;
        }
    }
}

QVariantMap CellMorphologyData::toVariantMap() const
{
    auto variantMap = WidgetAction::toVariantMap();

    std::vector<int32_t> ids;
    std::vector<mv::Vector3f> positions;
    std::vector<int32_t> types;
    std::vector<float> radii;
    std::vector<int32_t> parents;

    std::vector<int32_t> idMapKeys;
    std::vector<int32_t> idMapValues;

    for (const CellMorphology& cm : _cellMorphologies)
    {
        ids.insert(ids.end(), cm.ids.begin(), cm.ids.end());
        positions.insert(positions.end(), cm.positions.begin(), cm.positions.end());
        types.insert(types.end(), cm.types.begin(), cm.types.end());
        radii.insert(radii.end(), cm.radii.begin(), cm.radii.end());
        parents.insert(parents.end(), cm.parents.begin(), cm.parents.end());

        std::vector<int32_t> keys;
        std::vector<int32_t> values;
        for (auto const& p : cm.idMap) { keys.push_back(p.first); values.push_back(p.second); }
        idMapKeys.insert(idMapKeys.end(), keys.begin(), keys.end());
        idMapValues.insert(idMapValues.end(), values.begin(), values.end());
    }

    QVariantMap idsRawData = rawDataToVariantMap((char*)ids.data(), ids.size() * sizeof(int32_t), true);
    QVariantMap positionsRawData = rawDataToVariantMap((char*)positions.data(), positions.size() * sizeof(mv::Vector3f), true);
    QVariantMap typesRawData = rawDataToVariantMap((char*)types.data(), types.size() * sizeof(int32_t), true);
    QVariantMap radiiRawData = rawDataToVariantMap((char*)radii.data(), radii.size() * sizeof(float), true);
    QVariantMap parentsRawData = rawDataToVariantMap((char*)parents.data(), parents.size() * sizeof(int32_t), true);
    QVariantMap idMapKeysRawData = rawDataToVariantMap((char*)idMapKeys.data(), idMapKeys.size() * sizeof(int32_t), true);
    QVariantMap idMapValuesRawData = rawDataToVariantMap((char*)idMapValues.data(), idMapValues.size() * sizeof(int32_t), true);

    QVariantList cms;

    cms.reserve(_cellMorphologies.size());

    for (const CellMorphology& cm : _cellMorphologies) {
        cms.push_back(QVariantMap({
            { "NumberOfIds", QVariant::fromValue(cm.ids.size()) },
            { "NumberOfPositions", QVariant::fromValue(cm.positions.size()) },
            { "NumberOfTypes", QVariant::fromValue(cm.types.size()) },
            { "NumberOfRadii", QVariant::fromValue(cm.radii.size()) },
            { "NumberOfParents", QVariant::fromValue(cm.parents.size()) },
            { "NumberOfIdMapKeys", QVariant::fromValue(cm.idMap.size())},
            { "NumberOfIdMapValues", QVariant::fromValue(cm.idMap.size()) },
        }));
    }

    // https://stackoverflow.com/questions/19537186/serializing-qvariant-through-qdatastream
    QByteArray byteArray;
    QDataStream clustersDataStream(&byteArray, QIODevice::WriteOnly);

    clustersDataStream << cms;

    QVariantMap cmsRawData = rawDataToVariantMap((char*)byteArray.data(), byteArray.size(), true);

    variantMap.insert({
        { "CellMorphologyRawData", cmsRawData },
        { "CellMorphologyRawDataSize", byteArray.size() },
        { "IdsRawData", idsRawData },
        { "PositionsRawData", positionsRawData },
        { "TypesRawData", typesRawData },
        { "RadiiRawData", radiiRawData },
        { "ParentsRawData", parentsRawData },
        { "IdMapKeysRawData", idMapKeysRawData },
        { "IdMapValuesRawData", idMapValuesRawData },
        { "NumberOfIds", QVariant::fromValue(ids.size()) },
        { "NumberOfPositions", QVariant::fromValue(positions.size()) },
        { "NumberOfTypes", QVariant::fromValue(types.size()) },
        { "NumberOfRadii", QVariant::fromValue(radii.size()) },
        { "NumberOfParents", QVariant::fromValue(parents.size()) },
        { "NumberOfIdMapKeys", QVariant::fromValue(idMapKeys.size()) },
        { "NumberOfIdMapValues", QVariant::fromValue(idMapValues.size()) },
    });

    return variantMap;
}

CellMorphologies::CellMorphologies(QString dataName, const QString& guid /*= ""*/) :
    DatasetImpl(dataName, true, guid)
{
}

CellMorphologies::~CellMorphologies()
{
}

Dataset<DatasetImpl> CellMorphologies::createSubsetFromSelection(const QString& guiName, const Dataset<DatasetImpl>& parentDataSet /*= Dataset<DatasetImpl>()*/, const bool& visible /*= true*/) const
{
    return mv::data().createSubsetFromSelection(getSelection<CellMorphologies>(), const_cast<CellMorphologies*>(this), guiName, parentDataSet, visible);
}

Dataset<DatasetImpl> CellMorphologies::copy() const
{
    auto copySet = new CellMorphologies(getRawDataName());

    copySet->_indices = _indices;

    return copySet;
}

void CellMorphologies::setCellIdentifiers(const QStringList& cellIds)
{
    getRawData<CellMorphologyData>()->setCellIdentifiers(cellIds);
}

void CellMorphologies::setData(const std::vector<CellMorphology>& cellMorphologies)
{
    getRawData<CellMorphologyData>()->setData(cellMorphologies);
}

QIcon CellMorphologies::getIcon(const QColor& color /*= Qt::black*/) const
{
    return QIcon(":/Icon_64.png");
}

std::vector<std::uint32_t>& CellMorphologies::getSelectionIndices()
{
    return _indices;
}

void CellMorphologies::setSelectionIndices(const std::vector<std::uint32_t>& indices)
{
    _indices = indices;
}

bool CellMorphologies::canSelect() const
{
    return getRawData<CellMorphologyData>()->getData().size() >= 1;
}

bool CellMorphologies::canSelectAll() const
{
    return canSelect() && (getSelectionSize() < getRawData<CellMorphologyData>()->getData().size());
}

bool CellMorphologies::canSelectNone() const
{
    return canSelect() && (getSelectionSize() >= 1);
}

bool CellMorphologies::canSelectInvert() const
{
    return canSelect();
}

void CellMorphologies::selectAll()
{
    // Get reference to selection indices
    auto& selectionIndices = getSelectionIndices();

    // Clear and resize
    selectionIndices.clear();
    selectionIndices.resize(getRawData<CellMorphologyData>()->getData().size());

    // Generate cluster selection indices
    std::iota(selectionIndices.begin(), selectionIndices.end(), 0);

    // Notify others that the selection changed
    events().notifyDatasetDataSelectionChanged(this);
}

void CellMorphologies::selectNone()
{
    // Clear selection indices
    getSelectionIndices().clear();

    // Notify others that the selection changed
    events().notifyDatasetDataSelectionChanged(this);
}

void CellMorphologies::selectInvert()
{
    // Get reference to selection indices
    auto& selectionIndices = getSelectionIndices();

    // Create set of selected indices
    std::set<std::uint32_t> selectionSet(selectionIndices.begin(), selectionIndices.end());

    // Get number of items
    const auto numberOfItems = getRawData<CellMorphologyData>()->getData().size();

    // Clear and resize
    selectionIndices.clear();
    selectionIndices.reserve(numberOfItems - selectionSet.size());

    // Do the inversion
    for (std::uint32_t i = 0; i < numberOfItems; i++) {
        if (selectionSet.find(i) == selectionSet.end())
            selectionIndices.push_back(i);
    }

    // Notify others that the selection changed
    events().notifyDatasetDataSelectionChanged(this);
}

void CellMorphologies::fromVariantMap(const QVariantMap& variantMap)
{
    DatasetImpl::fromVariantMap(variantMap);

    getRawData<CellMorphologyData>()->fromVariantMap(variantMap);

    events().notifyDatasetDataChanged(this);
}

QVariantMap CellMorphologies::toVariantMap() const
{
    auto variantMap = DatasetImpl::toVariantMap();

    variantMap["Data"] = getRawData<CellMorphologyData>()->toVariantMap();

    return variantMap;
}

QIcon CellMorphologyDataFactory::getIcon(const QColor& color /*= Qt::black*/) const
{
    return QIcon(":/Icon_64.png");
}

plugin::RawData* CellMorphologyDataFactory::produce()
{
    return new CellMorphologyData(this);
}
