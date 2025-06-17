/*
  ==============================================================================

    colour_model.h
    Created: 9 Aug 2023 12:34:31am
    Author:  Vincenzo

  ==============================================================================
*/

#ifndef LUMATONE_COLOR_MODEL_H
#define LUMATONE_COLOR_MODEL_H

#include "../data/lumatone_editor_common.h"
#include "interpolation.h"

#define MAX_INCREMENT 16

class LumatoneColourModel
{
private:
    using uint8 = unsigned char;
    using ColourTable = uint8[MAX_INCREMENT][MAX_INCREMENT][MAX_INCREMENT][3];

    struct TrilinearInterpolationParams
    {
        Interpolate::TrilinearParams red;
        Interpolate::TrilinearParams green;
        Interpolate::TrilinearParams blue;

        TrilinearInterpolationParams(Interpolate::TrilinearParams r, Interpolate::TrilinearParams g, Interpolate::TrilinearParams b)
            : red(r), green(g), blue(b) { }
    };

public:
    enum class Type
    {
        RAW,
        ADJUSTED
    };

public:

    LumatoneColourModel();
    ~LumatoneColourModel();

    juce::String getName() const { return name; }
    LumatoneColourModel::Type getType() const { return currentType; }

    void setType(Type typeIn);

    juce::Colour getModelColour(juce::Colour colour);

    // LumatoneColour getLumatoneColour(juce::Colour colour);

private:

    juce::Colour calculateModelColour(const juce::Colour& colour);

    void readTable(const juce::var& tableVar, ColourTable& table);

    TrilinearInterpolationParams getInterpolationParams(const juce::Colour& c);

protected:

    void parseTable();

private:
    juce::String name;

    int increment = MAX_INCREMENT;

    ColourTable raw;
    ColourTable adjusted;

    Type currentType;
    ColourTable* currentTable;

    std::unique_ptr<juce::HashMap<LumatoneEditor::ColourHash,juce::Colour>> cache;
};

#endif LUMATONE_COLOR_MODEL_H
