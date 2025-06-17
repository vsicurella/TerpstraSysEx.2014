/*
  ==============================================================================

    colour_model.cpp
    Created: 9 Aug 2023 12:34:31am
    Author:  Vincenzo

  ==============================================================================
*/

#include "colour_model.h"

LumatoneColourModel::LumatoneColourModel()
{
    cache.reset(new juce::HashMap<LumatoneEditor::ColourHash, juce::Colour>(280));
    parseTable();
    setType(LumatoneColourModel::Type::ADJUSTED);
}

LumatoneColourModel::~LumatoneColourModel()
{
    cache = nullptr;
}

juce::Colour LumatoneColourModel::getModelColour(juce::Colour colour)
{
    if (colour.isTransparent())
        colour = juce::Colours::black;

    LumatoneEditor::ColourHash hash = LumatoneEditor::getColourHash(colour);
    auto cached = (*cache)[hash];
    if (cached != juce::Colours::transparentBlack)
        return cached;

    auto modelColour = calculateModelColour(colour);
    cache->set(hash, modelColour);
    return modelColour;
}

juce::Colour LumatoneColourModel::calculateModelColour(const juce::Colour& colour)
{
    auto params = getInterpolationParams(colour);

    float ri = Interpolate::trilinear(params.red);
    float gi = Interpolate::trilinear(params.green);
    float bi = Interpolate::trilinear(params.blue);

    juce::uint8 r = LumatoneEditor::roundToUint8(ri);
    juce::uint8 g = LumatoneEditor::roundToUint8(gi);
    juce::uint8 b = LumatoneEditor::roundToUint8(bi);

    auto modeled = juce::Colour(r, g, b);

    float hue = modeled.getHue();
    float saturation = modeled.getSaturation();
    float br = modeled.getBrightness();

    auto blackFiltered = juce::Colour::fromHSV(hue, saturation, 1.0f, br);
    if (br > 0.5)
        blackFiltered = blackFiltered.withMultipliedLightness(1.09f);

    return blackFiltered;
}

void LumatoneColourModel::readTable(const juce::var& tableVar, ColourTable& table)
{
    auto matrix1 = tableVar.getArray();
    for (int i = 0; i < increment; i++)
    {
        auto matrix2 = matrix1->getUnchecked(i).getArray();
        for (int j = 0; j < increment; j++)
        {
            auto matrix3 = matrix2->getUnchecked(j).getArray();
            for (int k = 0; k < increment; k++)
            {
                auto vec = matrix3->getUnchecked(k).getArray();
                for (int c = 0; c < 3; c++)
                {
                    table[i][j][k][c] = (int)vec->getUnchecked(c);
                }
            }
        }
    }
}

void LumatoneColourModel::parseTable()
{
    juce::var dataVar;
    auto result = juce::JSON::parse(BinaryData::model_json, dataVar);
    auto table = dataVar.getDynamicObject();

    auto names = table->getProperties();

    if (names.contains("increment"))
        increment = (int)table->getProperty("increment");

    if (names.contains("raw"))
        readTable(table->getProperty("raw"), raw);

    if (names.contains("adjusted"))
        readTable(table->getProperty("adjusted"), adjusted);

    return;
}

void LumatoneColourModel::setType(Type typeIn)
{
    if (currentType == typeIn)
        return;

    currentType = typeIn;
    switch (currentType)
    {
    case LumatoneColourModel::Type::RAW:
        currentTable = &raw;
        name = "model_raw";
        break;
    case LumatoneColourModel::Type::ADJUSTED:
    default:
        currentTable = &adjusted;
        name = "model_adjusted";
        break;
    }
}


LumatoneColourModel::TrilinearInterpolationParams LumatoneColourModel::getInterpolationParams(const juce::Colour& colour)
{
    float ri = colour.getFloatRed() * (increment - 1);
    int r0 = ri;
    int r1 = r0 + 1;

    float gi = colour.getFloatGreen() * (increment - 1);
    int g0 = gi;
    int g1 = g0 + 1;

    float bi = colour.getFloatBlue() * (increment - 1);
    int b0 = bi;
    int b1 = b0 + 1;

    uint8* n000 = (*currentTable)[r0][g0][b0];
    uint8* n100 = (*currentTable)[r1][g0][b0];
    uint8* n010 = (*currentTable)[r0][g1][b0];
    uint8* n110 = (*currentTable)[r1][g1][b0];
    uint8* n001 = (*currentTable)[r0][g0][b1];
    uint8* n101 = (*currentTable)[r1][g0][b1];
    uint8* n011 = (*currentTable)[r0][g1][b1];
    uint8* n111 = (*currentTable)[r1][g1][b1];

    float rt = ri - r0;
    float gt = gi - g0;
    float bt = bi - b0;

    auto redParams = Interpolate::TrilinearParams(
        n000[0], n100[0], n010[0], n110[0],
        n001[0], n101[0], n011[0], n111[0],
        rt, gt, bt
    );
    auto greenParams = Interpolate::TrilinearParams(
        n000[1], n100[1], n010[1], n110[1],
        n001[1], n101[1], n011[1], n111[1],
       rt, gt, bt
    );
    auto blueParams = Interpolate::TrilinearParams(
        n000[2], n100[2], n010[2], n110[2],
        n001[2], n101[2], n011[2], n111[2],
        rt, gt, bt
    );

    return LumatoneColourModel::TrilinearInterpolationParams(redParams, greenParams, blueParams);
}
