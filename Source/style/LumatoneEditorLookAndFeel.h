/*
  ==============================================================================

    LumatoneEditorLookAndFeel.h
    Created: 13 Nov 2020 7:25:04pm
    Author:  Vincenzo Sicurella

    Asset loading and minimal implementation of LookAndFeel_V4 for the Lumatone Editor application.

    Some components also need to be passed into its respective "Setup" function
    to apply default colours or other properties.

  ==============================================================================
*/

#pragma once

#include "LumatoneEditorStyleCommon.h"
#include "LumatoneEditorFontLibrary.h"

class LumatoneEditorLookAndFeel : public juce::LookAndFeel_V4
{
public:

    LumatoneEditorLookAndFeel(const LumatoneEditorFontLibrary& appFontsIn, bool setupAssets = false)
        : appFonts(appFontsIn)
        , compactWindowStyle(this)
    {
        setupDefaultColours();

        if (setupAssets)
        {
            cacheImages();
            cacheIcons();
        }

    }

    juce::Font getAppFont(LumatoneEditorFont fontIdIn, float heightIn=12.0f) const
    {
        return appFonts.getFont(fontIdIn, heightIn);
    }

public:
    // Helpers for static style definition that don't need component references

    juce::Font getLabelFont(float height=12.0f) const
    {
        return getAppFont(LumatoneEditorFont::FranklinGothic, height);
    }

    juce::Font getTabBarFont(float height=12.0f) const
    {
        return getAppFont(LumatoneEditorFont::GothamNarrowMedium, height);
    }

    juce::Font getSliderTextBoxFont(float height=12.0f) const
    {
        return getAppFont(LumatoneEditorFont::GothamNarrowMedium, height);
    }

    float getRoundedRectCornerToAppHeightRatio() const
    {
        return 0.00555556f;
    }

public:

    juce::ColourGradient getColourGradient(LumatoneEditorColourGradients gradientId, const juce::Rectangle<float>& area) const
    {
        switch (gradientId)
        {
            default:
                break;

            case LumatoneEditorColourGradients::ExpressionSlider:
                return juce::ColourGradient::horizontal(
                                        findColour(LumatoneEditorColourIDs::ExprRotaryGradientMin),
                                        findColour(LumatoneEditorColourIDs::ExprRotaryGradientMax),
                                        area);
            case LumatoneEditorColourGradients::BrightnessSlider:
                return juce::ColourGradient::horizontal(
                                        findColour(LumatoneEditorColourIDs::BrightnessRotaryGradientMin),
                                        findColour(LumatoneEditorColourIDs::BrightnessRotaryGradientMax),
                                        area);

            case LumatoneEditorColourGradients::HueSlider:
            {
                juce::ColourGradient g = juce::ColourGradient(findColour(LumatoneEditorColourIDs::HueRotaryGradientMin), area.getTopLeft(),
                                                              findColour(LumatoneEditorColourIDs::HueRotaryGradientMax), area.getTopRight(),
                                                              false);
                g.addColour(0.5f, findColour(LumatoneEditorColourIDs::HueRotaryGradientMid));
                return g;
            }

            case LumatoneEditorColourGradients::TemperatureSlider:
            {
                juce::ColourGradient g = juce::ColourGradient(findColour(LumatoneEditorColourIDs::TempRotaryGradientMin), area.getTopLeft(),
                                                              findColour(LumatoneEditorColourIDs::TempRotaryGradientMax), area.getTopRight(),
                                                              false);
                g.addColour(0.5f, findColour(LumatoneEditorColourIDs::TempRotaryGradientMid));
                return g;
            }
        }

        return juce::ColourGradient();
    }


public:
    //==================================================================
    // LookAndFeel_V4 Implementation

    //==================================================================
    //
    // DOCUMENT WINDOW METHODS
    //
    //==================================================================

    void drawDocumentWindowTitleBar(juce::DocumentWindow& window, juce::Graphics& g,
        int w, int h, int /*titleSpaceX*/, int /*titleSpaceW*/, const juce::Image* /*icon*/, bool /*drawTitleTextOnLeft*/) override
    {
        g.fillAll(findColour(LumatoneEditorColourIDs::MediumBackground));

        const float fontHeight = h * 0.5f;
        const juce::Font font = getAppFont(LumatoneEditorFont::UniviaProBold, fontHeight);
        g.setFont(font);

        const float margin = (h - fontHeight) * 0.5f;

        g.setColour(juce::Colours::white);
        g.drawFittedText(window.getName(), 0, margin - font.getDescent(), w, h - margin, juce::Justification::centred, 1, 1.0f);
    }


    juce::Button* createDocumentWindowButton(int buttonType) override
    {
        juce::Colour btnColour = findColour(LumatoneEditorColourIDs::MediumBackground);

        // Mostly pulled from  LookAndFeel_V4::createDocumentWindowButton();
        juce::Path shape;
        auto crossThickness = 0.15f;

        const float btnSize = 96.0f;
        const float vectorSize = btnSize * 0.333333f;
        const float vecHalfSize = vectorSize * 0.5f;

        juce::ImageButton* btn = new juce::ImageButton();
        juce::Image btnImage(juce::Image::PixelFormat::RGB, btnSize, btnSize, false);

        juce::Graphics g(btnImage);

        g.setColour(btnColour);
        g.fillAll();

        if (buttonType == juce::DocumentWindow::closeButton)
        {
            shape.addLineSegment({ 0.0f, 0.0f, vectorSize, vectorSize }, crossThickness);
            shape.addLineSegment({ vectorSize, 0.0f, 0.0f, vectorSize }, crossThickness);

            g.setColour(findColour(LumatoneEditorColourIDs::DisconnectedRed));
        }

        if (buttonType == juce::DocumentWindow::minimiseButton)
        {
            shape.addLineSegment({ 0.0f, vecHalfSize, vectorSize, vecHalfSize}, crossThickness);
            g.setColour(findColour(LumatoneEditorColourIDs::CurveGradientMin));
        }

        // So far not going to be used for Lumatone Editor
        if (buttonType == juce::DocumentWindow::maximiseButton)
        {
            shape.addLineSegment({ vecHalfSize, 0.0f, vecHalfSize, vectorSize }, crossThickness);
            shape.addLineSegment({ 0.0f, vecHalfSize, vectorSize, vecHalfSize }, crossThickness);

            g.setColour(findColour(LumatoneEditorColourIDs::ConnectedGreen));
        }

        float margin = (btnSize - vectorSize) * 0.5f;
        g.strokePath(shape, juce::PathStrokeType(3.0f), juce::AffineTransform::translation(margin, margin));

        btn->setImages(false, true, true,
            btnImage, 1.0f, juce::Colour(),
            btnImage, 1.0f, juce::Colours::white.withAlpha(0.1f),
            btnImage, 1.0f, juce::Colours::black.withAlpha(0.1f)
        );

        return btn;
    }

    // Primarily used for palette pop-up
    class LumatoneEditorCompactWindow : public juce::LookAndFeel_V4
    {
    public:
        LumatoneEditorCompactWindow(const LumatoneEditorLookAndFeel* parentIn) : parent(parentIn) {};

        void drawDocumentWindowTitleBar(juce::DocumentWindow& window, juce::Graphics& g,
            int w, int h, int /*titleSpaceX*/, int /*titleSpaceW*/, const juce::Image* /*icon*/, bool /*drawTitleTextOnLeft*/) override
        {
            g.fillAll(parent->findColour(LumatoneEditorColourIDs::HeaderBackground));

            g.setColour(parent->findColour(LumatoneEditorColourIDs::DescriptionText));
            g.setFont(parent->getAppFont(LumatoneEditorFont::GothamNarrowMedium, h * 0.75f));
            g.drawFittedText(window.getName(), 0, 0, w, h, juce::Justification::centred, 1, 1.0f);
        }

        juce::Button* createDocumentWindowButton(int buttonType) override
        {
            // Pulled from LookAndFeel_V4::createDocumentWindowButton();
            juce::Path shape;
            auto crossThickness = 0.15f;

            const float btnSize = 96.0f;
            const float vectorSize = btnSize * 0.333333f;
//            const float vecHalfSize = vectorSize * 0.5f;

            juce::ImageButton* btn = new juce::ImageButton();
            juce::Image btnImage(juce::Image::PixelFormat::RGB, btnSize, btnSize, false);

            juce::Graphics g(btnImage);
            g.fillAll(parent->findColour(LumatoneEditorColourIDs::HeaderBackground));

            if (buttonType == juce::DocumentWindow::closeButton)
            {
                shape.addLineSegment({ 0.0f, 0.0f, vectorSize, vectorSize }, crossThickness);
                shape.addLineSegment({ vectorSize, 0.0f, 0.0f, vectorSize }, crossThickness);

                g.setColour(parent->findColour(LumatoneEditorColourIDs::DescriptionText));
            }

            float margin = (btnSize - vectorSize) * 0.5f;
            g.strokePath(shape, juce::PathStrokeType(3.0f), juce::AffineTransform::translation(margin, margin));

            btn->setImages(false, true, true,
                btnImage, 1.0f, juce::Colour(),
                btnImage, 1.0f, juce::Colours::white.withAlpha(0.1f),
                btnImage, 1.0f, juce::Colours::black.withAlpha(0.1f)
            );

            return btn;
        }

    private:

        const LumatoneEditorLookAndFeel* parent;
    };

    //==================================================================
    //
    // LABEL METHODS
    //
    //==================================================================

    juce::Font getLabelFont(juce::Label& l) override
    {
        // For some reason ComboBox labels aren't scaling like other components do
        auto parent = l.getParentComponent();
        if (parent)
        {
            auto comboBox = dynamic_cast<juce::ComboBox*>(parent);
            if (comboBox)
                return getComboBoxFont(*comboBox);
        }

        juce::Font font = l.getFont();//.withHeight(l.getHeight());

        if (parent)
        {
            // Kludge - override font size for ColourSelector label so that it doesn't get cut off, and set TextEditor background colour
            // Probably need to make a bunch of changes to the way LookAndFeel is used to avoid this
            auto parentParent = parent->getParentComponent();
            auto colourPicker = dynamic_cast<juce::ColourSelector*>(parentParent);
            if (colourPicker)
            {
                auto stringWidth = font.getStringWidth(l.getText() + "_");
                auto labelScalar = (float)l.getWidth() / stringWidth;
                font.setHeight(l.getHeight() * labelScalar);
                l.setColour(juce::Label::ColourIds::backgroundWhenEditingColourId, colourPicker->getCurrentColour());
            }
        }

        juce::NamedValueSet& properties = l.getProperties();
        if (properties.contains(LumatoneEditorStyleIDs::fontOverride))
        {
            int overrideIndex = properties[LumatoneEditorStyleIDs::fontOverride];
            font = getAppFont((LumatoneEditorFont) overrideIndex).withHeight(l.getHeight());
        }

        if (properties.contains(LumatoneEditorStyleIDs::fontOverrideTypefaceStyle))
        {
            font.setTypefaceStyle(properties[LumatoneEditorStyleIDs::fontOverrideTypefaceStyle]);
        }

        if (properties.contains(LumatoneEditorStyleIDs::fontHeightScalar))
        {
            font.setHeight(l.proportionOfHeight(properties[LumatoneEditorStyleIDs::fontHeightScalar]));
        }

        return font;
    }

    juce::BorderSize<int> getLabelBorderSize(juce::Label&) override
    {
        return juce::BorderSize<int>(0);
    }

    void drawLabel(juce::Graphics& g, juce::Label& l) override
    {
        juce::Colour bkgdColour, textColour, outlineColour;

        auto labelBounds = l.getBounds().toFloat();
        juce::Path roundedBounds = getConnectedRoundedRectPath(labelBounds, l.getHeight() * comboBoxRoundedCornerScalar, 0);
        bkgdColour = l.findColour(juce::Label::ColourIds::backgroundColourId);
        g.setColour(bkgdColour);
        g.fillPath(roundedBounds);

        textColour = l.findColour(juce::Label::ColourIds::textColourId);
        if (!l.isEnabled())
            textColour.darker(0.4f);

        g.setColour(textColour);

        juce::Font font = getLabelFont(l);
        float fontScalar = 1.0f;
        int maxLines = l.getProperties().contains(LumatoneEditorStyleIDs::labelMaximumLineCount)
            ? (int)l.getProperties()[LumatoneEditorStyleIDs::labelMaximumLineCount]
            : 1;

        if (maxLines == 1)
        {
            fontScalar *= scalarToFitString(l);
        }

        font.setHeight(font.getHeight() * fontScalar * GLOBALFONTSCALAR);
        g.setFont(font);

        g.drawFittedText(l.getText(), l.getLocalBounds(), l.getJustificationType(), maxLines, 0.0f);

        outlineColour = l.findColour(juce::Label::ColourIds::outlineColourId);
        if (outlineColour.getAlpha() > (juce::uint8)0)
        {
            g.setColour(outlineColour);
            g.drawRect(l.getLocalBounds());
        }
    }

    //==================================================================
    //
    // BUTTON METHODS
    //
    //==================================================================

    void drawButtonBackground(juce::Graphics& g, juce::Button& btn, const juce::Colour& backgroundColour, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        // If it has Hyperlink flag, only draw a line under text if mouse is over
        if (btn.getProperties().contains(LumatoneEditorStyleIDs::textButtonHyperlinkFlag))
        {
            if (shouldDrawButtonAsHighlighted)
            {
                g.setColour(btn.findColour(juce::TextButton::ColourIds::textColourOffId));
                float halfTextLength = getTextButtonFont(static_cast<juce::TextButton&>(btn), btn.getHeight()).getStringWidthFloat(btn.getButtonText()) * 0.5f;
                float xCenter = btn.getWidth() * 0.5f;
                float yLine = btn.proportionOfHeight(0.9f);
                g.drawLine(juce::roundToInt(xCenter - halfTextLength), yLine, juce::roundToInt(xCenter + halfTextLength), yLine);
            }

            return;
        }

        juce::Colour colour = (btn.getToggleState()) ? btn.findColour(juce::TextButton::ColourIds::buttonOnColourId) : backgroundColour;

        if (!btn.isEnabled() && colour.isOpaque())
            colour = colour.overlaidWith(findColour(LumatoneEditorColourIDs::DisabledOverlay));

        // Do not highlight if the button is a part of a radio group and is already toggled on
        if (shouldDrawButtonAsHighlighted && !(btn.getRadioGroupId() > 0 && btn.getToggleState()))
            colour = colour.brighter(0.075f);

        g.setColour(colour);
        g.fillPath(getButtonShape(btn));

        if (btn.isColourSpecified(LumatoneEditorColourIDs::OutlineColourId))
        {
            g.setColour(btn.findColour(LumatoneEditorColourIDs::OutlineColourId));
            g.strokePath(getButtonShape(btn), juce::PathStrokeType(1.5f));
        }
    }

    void drawButtonText(juce::Graphics& g, juce::TextButton& btn, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        juce::NamedValueSet properties = btn.getProperties();

        if (properties.contains(LumatoneEditorStyleIDs::textButtonIconHashCode))
        {
            int bkgdColourId = (shouldDrawButtonAsDown) ? juce::TextButton::ColourIds::buttonOnColourId : juce::TextButton::ColourIds::buttonColourId;
            juce::Colour bkgdColour = btn.findColour(bkgdColourId);
            drawButtonBackground(g, btn, bkgdColour, shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);

            int colourId = shouldDrawButtonAsDown ? juce::TextButton::ColourIds::textColourOnId : juce::TextButton::ColourIds::textColourOffId;
            juce::Colour textColour = btn.findColour(colourId);

            if (!btn.isEnabled())
                textColour = findColour(LumatoneEditorColourIDs::InactiveText);

            else if (shouldDrawButtonAsHighlighted)
                textColour = textColour.brighter();

                juce::Font font = getTextButtonFont(btn, btn.getHeight() * GLOBALFONTSCALAR);
            int margin = font.getStringWidth("  ");
            int textWidth = font.getStringWidth(btn.getButtonText());

            auto iconCode = (int)btn.getProperties()[LumatoneEditorStyleIDs::textButtonIconHashCode];

            int iconH = font.getHeight();// *0.9f;
            int iconW = iconH;
            int iconY = juce::roundToInt((btn.getHeight() - iconH) * 0.5f);

            // Icon specific adjustments
            switch (iconCode)
            {
            case LumatoneEditorIcon::LoadIcon:
                iconW *= 1.125f;
                iconH *= 1.125f;
                break;
            case LumatoneEditorIcon::ArrowUp:
                margin = font.getStringWidth(" ");
                break;
            }

            if (iconW < 1) iconW = 1;
            if (iconH < 1) iconH = 1;

            // Set up positions of icon and text
            int lineStart = juce::roundToInt((btn.getWidth() - textWidth - margin - iconW) * 0.5f);

            int iconX = 0;
            juce::Rectangle<int> textBounds;

            bool iconOnRight = (bool)properties[LumatoneEditorStyleIDs::textButtonIconPlacement];
            if (iconOnRight)
            {
                iconX = lineStart + textWidth + margin;
                textBounds = btn.getLocalBounds().withLeft(lineStart).withRight(iconX - margin);
            }
            else
            {
                iconX = lineStart;
                textBounds = btn.getLocalBounds().withLeft(iconX + iconW + margin);
            }

            // Needs special case for "transparent" arrow
            if (iconCode == LumatoneEditorIcon::LoadIcon)
            {
                // Make a tad bit bigger
                auto boundsAdj = juce::Rectangle<float>(iconX, iconY, iconW, iconH);
                boundsAdj.expand(iconW * 0.1f, iconH * 0.1f);
                drawFolderIconAt(g, boundsAdj.getX(), boundsAdj.getY(), boundsAdj.getWidth(), boundsAdj.getHeight(), textColour, btn.findColour(bkgdColourId));
            }
            else
            {
                juce::Path iconPath;
                juce::PathStrokeType stroke(1.5f);
                stroke.setEndStyle(juce::PathStrokeType::EndCapStyle::rounded);
                stroke.setJointStyle(juce::PathStrokeType::JointStyle::curved);
                juce::Colour iconColour = textColour;
                float iconScale = 1.0f;
                int iconXOffset = 0.0;
                switch (LumatoneEditorIcon(iconCode))
                {
                // Using X for now
                //case LumatoneEditorIcon::Checkmark:
                //    iconPath = Path(tickBoxPath);
                //    break;
                case LumatoneEditorIcon::ArrowUp:
                {
                    iconPath = arrowUpIconPath;
                    break;
                }
                case LumatoneEditorIcon::ArrowDown:
                {
                    iconPath = arrowDownIconPath;
                    break;
                }
                case LumatoneEditorIcon::SaveIcon:
                {
                    iconPath = saveIconPath;
                    break;
                }
                case LumatoneEditorIcon::ColourPicker:
                {
                    iconPath = colourPickerPath;
                    iconScale = 0.04f;
                    iconXOffset = iconW * 0.3333f;
                    iconColour = bkgdColour.contrasting();
                    stroke.setStrokeThickness(1.0f);
                    break;
                }
                default:
                    break;
                }

                auto transform = juce::AffineTransform::scale(iconW * iconScale, iconH * iconScale)
                     .followedBy(juce::AffineTransform::translation(iconX + iconXOffset, iconY));
                iconPath.applyTransform(transform);
                g.setColour(iconColour);
                g.strokePath(iconPath, stroke);
            }

            g.setColour(textColour);
            g.setFont(font);
            g.drawFittedText(btn.getButtonText(), textBounds, juce::Justification::left, 1);
        }
        else
        {
            int bkgdColourId = (btn.getToggleState()) ? juce::TextButton::ColourIds::buttonOnColourId : juce::TextButton::ColourIds::buttonColourId;
            drawButtonBackground(g, btn, btn.findColour(bkgdColourId), shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);

            juce::Font font = getTextButtonFont(btn, btn.getHeight());
            int colourId = (btn.getToggleState()) ? juce::TextButton::ColourIds::textColourOnId : juce::TextButton::ColourIds::textColourOffId;

            juce::Colour textColour = btn.findColour(colourId);

            if ((btn.getClickingTogglesState() && !btn.getToggleState()) || !shouldDrawButtonAsDown)
            {
                if (!btn.isEnabled())
                    textColour = findColour(LumatoneEditorColourIDs::InactiveText);

                if (shouldDrawButtonAsHighlighted)
                    textColour = textColour.brighter();
            }

            g.setColour(textColour);
            g.setFont(font);
            g.drawFittedText(btn.getButtonText(), btn.getLocalBounds(), juce::Justification::centred, 1);
        }
    }

    juce::Font getTextButtonFont(juce::TextButton& btn, int buttonHeight) override
    {
        float fontHeight = (btn.getProperties().contains(LumatoneEditorStyleIDs::textButtonHyperlinkFlag))
            ? buttonHeight
            : buttonHeight / 1.75f;

        juce::Font font = getAppFont(LumatoneEditorFont::GothamNarrowMedium, fontHeight);

        juce::NamedValueSet& properties = btn.getProperties();
        if (properties.contains(LumatoneEditorStyleIDs::fontOverride))
        {
            int overrideIndex = properties[LumatoneEditorStyleIDs::fontOverride];
            font = getAppFont((LumatoneEditorFont)overrideIndex, btn.getHeight() * CONTROLBOXFONTHEIGHTSCALAR);
        }

        if (properties.contains(LumatoneEditorStyleIDs::fontOverrideTypefaceStyle))
        {
            font.setTypefaceStyle(properties[LumatoneEditorStyleIDs::fontOverrideTypefaceStyle]);
        }

        if (properties.contains(LumatoneEditorStyleIDs::fontHeightScalar))
        {
            font.setHeight(font.getHeight() * (float)properties[LumatoneEditorStyleIDs::fontHeightScalar]);
        }

        return font;
    }

    void drawTickBox(juce::Graphics& g, juce::Component& c, float x, float y, float w, float h, bool ticked, bool isEnabled, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        juce::Path shape;
        shape.addRoundedRectangle(x, y, w, h, w * 0.1f);

        juce::Colour boxColour = juce::Colours::white;
        juce::Colour tickColour = juce::Colours::black;

        if (!c.isEnabled())
        {
            boxColour = boxColour.overlaidWith(findColour(LumatoneEditorColourIDs::DisabledOverlay));
            tickColour = findColour(LumatoneEditorColourIDs::InactiveText);
        }

        g.setColour(boxColour);
        g.fillPath(shape);

        // TODO: Proper "tick" drawing
        if (ticked)
        {
            g.setColour(tickColour);
            g.setFont(getAppFont(LumatoneEditorFont::GothamNarrowBold, h).withHorizontalScale(1.333333f));
            g.drawFittedText("X", x, y, w, h, juce::Justification::centred, 1, 1.0f);
        }
    }

    juce::Font getToggleButtonFont(juce::ToggleButton& btn, float fontHeight = 0)
    {
    #if JUCE_MAC
        float fontScalar = 1.0675f;
    #else
        float fontScalar = 1.125f;
    #endif

        if (fontHeight <= 0)
            fontHeight = btn.getHeight();
        return getAppFont(LumatoneEditorFont::GothamNarrowMedium, fontHeight * fontScalar);
    }

    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& btn, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        drawTickBox(g, btn, 0, 0, btn.getHeight(), btn.getHeight(), btn.getToggleState(), btn.isEnabled(),
            shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);

        juce::Colour textColour = btn.findColour(juce::ToggleButton::ColourIds::textColourId);

        if (shouldDrawButtonAsDown)
            textColour = textColour.darker();

        if (!btn.isEnabled())
            textColour = findColour(LumatoneEditorColourIDs::InactiveText);

        else if (shouldDrawButtonAsHighlighted)
            textColour = textColour.brighter(0.1f);

        g.setColour(textColour);

        g.setFont(getToggleButtonFont(btn));
        g.drawFittedText(btn.getButtonText(), btn.getLocalBounds().withLeft(btn.getHeight() * 1.5f), juce::Justification::centredLeft, 1);
    }


    //==================================================================
    //
    // SLIDER METHODS
    //
    //==================================================================

    void drawIncDecButtonsBackground(juce::Graphics& g, int x, int y, int width, int height, float sliderPos, float minSliderPos, float maxSliderPos, juce::Slider& sld)
    {

        //juce::Rectangle<int> bounds(x, y, width, height);
        //g.setColour(juce::Colours::red);
        //g.fillRect(bounds);

        //g.setColour(juce::Colours::blue.withAlpha(0.5f));
        //g.fillRect(bounds.withLeft(minSliderPos));

        //g.setColour(juce::Colours::yellow.withAlpha(0.5f));
        //g.fillRect(bounds.withLeft(maxSliderPos));
    }

    void drawLinearSliderBackground(juce::Graphics& g, int x, int y, int width, int height, float sliderPos, float minSliderPos, float maxSliderPos, const juce::Slider::SliderStyle style, juce::Slider& sld) override
    {
        if (style == juce::Slider::SliderStyle::IncDecButtons)
        {
            drawIncDecButtonsBackground(g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, sld);
        }
        else
        {
            juce::LookAndFeel_V4::drawLinearSliderBackground(g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, sld);
        }
    }

    juce::Button* createSliderButton(juce::Slider& sld, bool isIncrement) override
    {
        auto btn = new TextButtonMouseHighlight(juce::Colours::white.withAlpha(0.2f));

        btn->setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colour());
        btn->setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colour());

        juce::Colour textColour = findColour(LumatoneEditorColourIDs::DescriptionText);
        btn->setColour(juce::TextButton::ColourIds::textColourOnId, textColour);
        btn->setColour(juce::TextButton::ColourIds::textColourOffId, textColour);

        if (isIncrement)
            btn->setButtonText("+");
        else
            btn->setButtonText("-");

        btn->getProperties().set(LumatoneEditorStyleIDs::fontHeightScalar, 1.6f);

        return btn;
    }

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle, juce::Slider& sld) override
    {
        const float halfPi = juce::MathConstants<float>::pi * 0.5f;
        const float arcThickness = 0.15f;
        const float endAngleNorm = rotaryAngleEnd - halfPi;

        const int w = sld.getWidth();
        const int h = juce::roundToInt(sld.getHeight() * (1.5 - 0.5 * sinf(endAngleNorm))); // translated to compensate for dial radius

        const float dialRadiusFactor = 1.1f;
        const float size = juce::jmin(w, h) / dialRadiusFactor;
        const float radiusInner = size * (1 - arcThickness);

        const int dialMargin = (size * dialRadiusFactor) - size;
        juce::Rectangle<float> outerBounds = juce::Rectangle<float>((w - size) * 0.5f, dialMargin, size, size);
        juce::Rectangle<float> innerBounds = outerBounds.reduced(size * arcThickness);
        juce::Point<float> center = outerBounds.getCentre();

        juce::Path ring;
        addArcToPath(ring, outerBounds, rotaryAngleStart, rotaryAngleEnd, true);
        ring.lineTo(center.x + cosf(endAngleNorm) * radiusInner, center.y + sinf(endAngleNorm) * radiusInner);
        addArcToPath(ring, innerBounds, rotaryAngleEnd, rotaryAngleStart, false);
        ring.closeSubPath();

        LumatoneEditorColourGradients gradientId = LumatoneEditorColourGradients((int)sld.getProperties()[LumatoneEditorStyleIDs::sliderRotaryColourGradient]);
        juce::ColourGradient grad = getColourGradient(gradientId, outerBounds);
        g.setGradientFill(grad);
        g.fillPath(ring);

        juce::Colour dialColour = juce::Colours::white;
        g.setColour(dialColour);

        float dialRadius = size * dialRadiusFactor * 0.5f;
        float dialThickness = size * 0.025f;
        float dialAng = (rotaryAngleEnd - rotaryAngleStart) * sliderPosProportional + rotaryAngleStart - halfPi;
        g.drawLine(juce::Line<float>(center, { center.x + cosf(dialAng) * dialRadius , center.y + sinf(dialAng) * dialRadius }), dialThickness);

        // TODO: Make text box positioning more consistent when manipulating the bounds' aspect ratio
    }

    juce::Label* createSliderTextBox(juce::Slider& sld) override
    {
        juce::Label* label = new juce::Label(sld.getName() + "_ValueLabel");

        label->setText(juce::String(sld.getValue()), juce::dontSendNotification);
        label->setJustificationType(juce::Justification::centred);
        label->setFont(getSliderTextBoxFont());

        if (sld.getSliderStyle() >= juce::Slider::SliderStyle::Rotary && sld.getSliderStyle() < juce::Slider::SliderStyle::IncDecButtons)
        {
            juce::Colour textColour = (sld.isEnabled())
                ? findColour(LumatoneEditorColourIDs::DescriptionText)
                : findColour(LumatoneEditorColourIDs::InactiveText);

            label->setColour(juce::Label::ColourIds::textColourId, textColour);

            int sliderSize = juce::jmin(sld.getWidth(), sld.getHeight());
            sld.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, false,
                juce::roundToInt(sliderSize * 0.5f),
                juce::roundToInt((sld.getHeight() - sliderSize * 0.5f) * 0.75f)
            );

            return label;
        }
        else if (sld.getSliderStyle() == juce::Slider::SliderStyle::IncDecButtons)
        {
            float fontHeightScalar = 1.0f;
            if (sld.getProperties().contains(LumatoneEditorStyleIDs::fontHeightScalar))
            {
                auto scalarProperty = sld.getProperties()[LumatoneEditorStyleIDs::fontHeightScalar];
                fontHeightScalar = (float)scalarProperty;
                label->getProperties().set(LumatoneEditorStyleIDs::fontHeightScalar, scalarProperty);
            }

            juce::Colour backgroundColour = findColour(LumatoneEditorColourIDs::ControlBoxBackground);
            juce::Colour textColour = findColour(LumatoneEditorColourIDs::DescriptionText);

            if (!sld.isEnabled())
            {
                backgroundColour = backgroundColour.overlaidWith(findColour(LumatoneEditorColourIDs::DisabledOverlay));
                textColour = findColour(LumatoneEditorColourIDs::InactiveText);
            }

            label->setColour(juce::Label::ColourIds::backgroundColourId, backgroundColour);
            label->setColour(juce::Label::ColourIds::backgroundWhenEditingColourId, backgroundColour);
            label->setColour(juce::Label::ColourIds::textColourId, textColour);
            // auto sliderRange = sld.getRange();
            // juce::String minValue = juce::String(juce::roundToInt(sliderRange.getStart()));
            // juce::String maxValue = juce::String(juce::roundToInt(sliderRange.getEnd()));
            // juce::String longestValue = (minValue.length() > maxValue.length()) ? minValue : maxValue;

            // // +1 for extra margin
            // int numPlaces = longestValue.length() + sld.getNumDecimalPlacesToDisplay() + 1;
            // for (int n = 0; n < numPlaces; n++)
            // {
            //     longestValue += "_";
            // }

            // float fontHeight = sld.getHeight() * fontHeightScalar * 0.8f;

            // juce::Font textBoxFont = getSliderTextBoxFont(fontHeight);

            // int labelMaxWidth = textBoxFont.getStringWidth(longestValue);

            // sld.setTextBoxStyle(sld.getTextBoxPosition(), false, labelMaxWidth, sld.getHeight());
            sld.setTextBoxStyle(sld.getTextBoxPosition(), false, sld.getTextBoxWidth(), sld.getHeight());

            return label;
        }
        else
        {
            return juce::LookAndFeel_V4::createSliderTextBox(sld);
        }
    }

    //==================================================================
    //
    // TEXTEDITOR METHODS
    //
    //==================================================================

    void fillTextEditorBackground(juce::Graphics& g, int width, int height, juce::TextEditor& editor) override
    {
        juce::Colour backgroundColour = editor.findColour(juce::TextEditor::ColourIds::backgroundColourId);

        if (!backgroundColour.isTransparent())
        {
            if (!editor.isEnabled())
                backgroundColour = backgroundColour.withMultipliedSaturation(0.33f);

            // This will assume all edges are connected (default TextEditor) if not "connectedEdgeFlag" property is set

            juce::Path boxShape;
            if ((int)editor.getProperties()[LumatoneEditorStyleIDs::connectedEdgeFlags] < 15) // 15 = All edges connected
            {
                boxShape = getConnectedRoundedRectPath(juce::Rectangle<float>(0, 0, width, height), juce::roundToInt(height * comboBoxRoundedCornerScalar), editor.getProperties()[LumatoneEditorStyleIDs::connectedEdgeFlags]);
            }
            else
            {
                boxShape.addRectangle(0, 0, width, height);
            }

            g.setColour(backgroundColour);
            g.fillPath(boxShape);
            return;
        }

        g.fillAll(backgroundColour);
    }

    void drawTextEditorOutline(juce::Graphics&, int, int, juce::TextEditor&) override {}



    //==================================================================
    //
    // COMBOBOX METHODS
    //
    //==================================================================

    void drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown, int buttonX, int buttonY, int buttonW, int buttonH, juce::ComboBox& box) override
    {
        int margin = juce::roundToInt(height * comboBoxRoundedCornerScalar);

        juce::Colour backgroundColour = box.findColour(juce::ComboBox::ColourIds::backgroundColourId);
        juce::Colour textColour = findColour(LumatoneEditorColourIDs::DescriptionText);

        if (!box.isEnabled())
        {
            backgroundColour = backgroundColour.withMultipliedSaturation(0.33f);
            textColour = findColour(LumatoneEditorColourIDs::InactiveText);
        }

        else if (box.isMouseOver(true))
        {
            backgroundColour = backgroundColour.withMultipliedSaturation(1.5f);// .brighter(0.05f);
            textColour       = textColour.brighter(0.1f);
        }

        g.setColour(backgroundColour);

        // auto properties = box.getProperties();
        // juce::Path boxShape;

        // if (properties.contains(LumatoneEditorStyleIDs::roundedDiagonalCorners))
        // {
        //     boxShape = getDiagonalRoundedCornersPath(box.getLocalBounds().toFloat(),
        //         juce::roundToInt(height * comboBoxRoundedCornerScalar),
        //         (int)properties[LumatoneEditorStyleIDs::roundedDiagonalCorners],
        //         box.isPopupActive()
        //     );
        // }
        // else
        // {
        //     boxShape = getConnectedRoundedRectPath(box.getLocalBounds().toFloat(),
        //         juce::roundToInt(height * comboBoxRoundedCornerScalar),
        //         (box.isPopupActive()) ? juce::Button::ConnectedEdgeFlags::ConnectedOnBottom : 0
        //     );
        // }
        // g.fillPath(boxShape);

        int roundedRectSize = height * comboBoxRoundedCornerScalar;
        g.fillRoundedRectangle(juce::Rectangle<float>(0, 0, width, height), roundedRectSize);

        int realButtonX = juce::jmax(margin, box.getWidth() - box.getHeight());

        if (buttonW > 0)
        {
            g.setColour(textColour);
            g.setFont(getAppFont(LumatoneEditorFont::GothamNarrowLight, buttonH * 0.5f).withHorizontalScale(1.5f));
            g.drawFittedText("v", realButtonX, 0, box.getHeight(), box.getHeight(), juce::Justification::centred, 1);
        }

        if (box.getProperties().contains(LumatoneEditorStyleIDs::comboBoxRenderColourPreview))
        {
            juce::Colour previewColour = juce::Colour::fromString(box.getProperties()[LumatoneEditorStyleIDs::comboBoxRenderColourPreview].toString());
            if (previewColour.isOpaque())
            {
                float previewSize = height - roundedRectSize * 2;
                float previewX = realButtonX - roundedRectSize;
                float previewY = (height - previewSize) * 0.5f;
                juce::Rectangle<float> previewBounds = juce::Rectangle<float>(previewX, previewY, previewSize, previewSize);
                g.setColour(previewColour);
                g.fillRect(previewBounds);
                g.setColour(previewColour.contrasting(backgroundColour, previewColour));
                g.drawRect(previewBounds, 1.0f);
            }
        }
    }

    juce::Font getComboBoxFont(juce::ComboBox& box) override
    {
        juce::Font font = getAppFont(LumatoneEditorFont::GothamNarrowMedium, box.getHeight() * CONTROLBOXFONTHEIGHTSCALAR);

        juce::NamedValueSet& properties = box.getProperties();
        if (properties.contains(LumatoneEditorStyleIDs::fontOverride))
        {
            int overrideIndex = properties[LumatoneEditorStyleIDs::fontOverride];
            font = getAppFont((LumatoneEditorFont)overrideIndex, box.getHeight() * CONTROLBOXFONTHEIGHTSCALAR);
        }

        if (properties.contains(LumatoneEditorStyleIDs::fontOverrideTypefaceStyle))
        {
            font.setTypefaceStyle(properties[LumatoneEditorStyleIDs::fontOverrideTypefaceStyle]);
        }

        if (properties.contains(LumatoneEditorStyleIDs::fontHeightScalar))
        {
            font.setHeight(font.getHeight() * (float)properties[LumatoneEditorStyleIDs::fontHeightScalar]);
        }

        return font;
    }

    juce::Label* createComboBoxTextBox(juce::ComboBox& box) override
    {
        juce::Label* l = new juce::Label(box.getName(), box.getText());

        juce::Colour textColour = box.findColour(juce::ComboBox::ColourIds::textColourId);

        if (!box.isEnabled())
            textColour = textColour.overlaidWith(findColour(LumatoneEditorColourIDs::DisabledOverlay));

        l->setColour(juce::Label::ColourIds::textColourId, textColour);
        l->setColour(juce::Label::ColourIds::backgroundColourId, box.findColour(juce::ComboBox::ColourIds::backgroundColourId));

        for (auto prop : box.getProperties())
            l->getProperties().set(prop.name, prop.value);

        auto name = box.getName();
        l->setBounds(box.getLocalBounds());
        l->setFont(getComboBoxFont(box)); // Any style overrides should have been passed to Label

        juce::String textEditChars = box.getProperties()[LumatoneEditorStyleIDs::comboBoxEditorRestrictedChars].toString();
        int textEditLength = (int)box.getProperties()[LumatoneEditorStyleIDs::comboBoxEditorRestrictedLength];

        if (textEditChars.length() > 0 || textEditLength > 0)
        {
            l->onEditorShow = [l, textEditChars, textEditLength]()
            {
                auto editor = l->getCurrentTextEditor();
                if (editor)
                    editor->setInputRestrictions(textEditLength, textEditChars);
            };
        }

        return l;
    }

    void positionComboBoxText(juce::ComboBox& box, juce::Label& labelToPosition) override
    {
        int margin = box.proportionOfHeight(comboBoxRoundedCornerScalar);
        float fontHeight = labelToPosition.getFont().getHeight();

        labelToPosition.setBounds(
            margin
            , (box.getHeight() - fontHeight) * 0.5f
            , box.getWidth() - box.getHeight() - margin /* leave room for down arrow glyph */
            , fontHeight
        );
    }

    void drawComboBoxTextWhenNothingSelected(juce::Graphics& g, juce::ComboBox& box, juce::Label& l) override
    {
        juce::String text = (box.getNumItems()) ? box.getTextWhenNothingSelected() : box.getTextWhenNoChoicesAvailable();
        l.setText(text, juce::NotificationType::dontSendNotification);
    }

    juce::PopupMenu::Options getOptionsForComboBoxPopupMenu(juce::ComboBox& box, juce::Label& label) override
    {
        int numColumns = juce::jmax(1, (int)box.getProperties()[LumatoneEditorStyleIDs::popupMenuMaxColumns]);

        return juce::PopupMenu::Options()
            .withTargetComponent(box)
            .withMinimumWidth(box.getWidth())
            .withMinimumNumColumns(numColumns)
            .withMaximumNumColumns(numColumns)
            .withStandardItemHeight(label.getFont().getHeight() * CONTROLBOXFONTHEIGHTSCALAR)
            .withPreferredPopupDirection(juce::PopupMenu::Options::PopupDirection::downwards);
    }

    //==================================================================
    //
    // POPUPMENU METHODS
    //
    //==================================================================


	void drawPopupMenuBackgroundWithOptions(juce::Graphics& g, int width, int height, const juce::PopupMenu::Options& options) override
    {
        auto target = options.getTargetComponent();

        int targetWidth = (target != nullptr && target->getProperties()[LumatoneEditorStyleIDs::popupMenuTargetWidth])
            ? target->getWidth()
            : width;

        int targetMargin = (target != nullptr)
            ? target->getHeight() * comboBoxRoundedCornerScalar
            : 0;

        juce::Colour targetColour = (target != nullptr && target->getProperties().contains(LumatoneEditorStyleIDs::popupMenuBackgroundColour))
            ? juce::Colour::fromString(target->getProperties()[LumatoneEditorStyleIDs::popupMenuBackgroundColour].toString()).withMultipliedSaturation(1.5f) // Box colour will always be highlighted
            : findColour(LumatoneEditorColourIDs::MenuBarBackground);

        g.setColour(targetColour);

		// juce::Path menuShape = getConnectedRoundedRectPath(juce::Rectangle<float>(0, 0, targetWidth, height), targetMargin, juce::Button::ConnectedEdgeFlags::ConnectedOnTop);
		// g.fillPath(menuShape);
        g.fillRoundedRectangle(juce::Rectangle<float>(0, 0, targetWidth, height), targetMargin);
    }

    static float getPopupMenuColourItemSize(const juce::Font font)
    {
        return font.getStringWidth("DD") * 0.67f;
    }

    void drawPopupMenuItemWithOptions(
        juce::Graphics& g, const juce::Rectangle<int>& area, bool isHighlighted, const juce::PopupMenu::Item& item, const juce::PopupMenu::Options& options) override
    {
        int width, height;
        getIdealPopupMenuItemSizeWithOptions(item.text, false, area.getHeight(), width, height, options);

        juce::Rectangle<int> areaToUse = area;

        auto target = dynamic_cast<juce::ComboBox*>(options.getTargetComponent());
        juce::Font font;
        juce::Colour textColour;
        int margin = 0;
        bool renderColour = false;
        if (target)
        {
            font = getComboBoxFont(*target);
            textColour = target->findColour(juce::ComboBox::ColourIds::textColourId);
            margin = target->proportionOfHeight(comboBoxRoundedCornerScalar);
            areaToUse = areaToUse.withWidth(width);

            auto properties = target->getProperties();
            if (properties[LumatoneEditorStyleIDs::comboBoxRenderColourItems])
            {
                renderColour = true;
            }
        }
        else
        {
            float fontScalar = scalarToFitString(item.text, font, width);
            font = getPopupMenuFont().withHeight(area.getHeight() * fontScalar * CONTROLBOXFONTHEIGHTSCALAR);
            textColour = findColour(LumatoneEditorColourIDs::DescriptionText);
            margin = juce::roundToInt(height * comboBoxRoundedCornerScalar);
        }

        if (!item.isEnabled)
            textColour = textColour.overlaidWith(findColour(LumatoneEditorColourIDs::DisabledOverlay));

        font.setHeight(font.getHeight() * GLOBALFONTSCALAR);

        // If it's the last item, reduce size so highlight doesn't pass rounded corners
        if (target != nullptr && item.itemID == target->getItemId(target->getNumItems() - 1))
        {
            areaToUse = areaToUse.withTrimmedBottom(juce::roundToInt(margin * 0.5f) + 1);
        }

        if (isHighlighted && !item.isSeparator)
        {
            g.setColour(juce::Colours::white.withAlpha(0.15f));
            g.fillRect(areaToUse);
        }

        g.setColour(textColour);
        g.setFont(font);

        juce::Rectangle<int> textArea = areaToUse.withTrimmedLeft(margin).withTrimmedRight(margin);
        g.drawFittedText(item.text, textArea, juce::Justification::centredLeft, 1);

        if (item.subMenu)
        {
            g.drawFittedText(">", areaToUse.withTrimmedRight(margin), juce::Justification::centredRight, 1, 0.5f);
            item.subMenu->setLookAndFeel(this);
        }
        else if (renderColour && item.text.length() == 6)
        {
            juce::Colour itemColour = juce::Colour::fromString("ff" + item.text);
            if (itemColour.isOpaque())
            {
                g.setColour(itemColour);

                float textWidth = font.getStringWidth("DDDDDD ");
                float colourSize = getPopupMenuColourItemSize(font);
                float colourY = juce::roundToInt((areaToUse.getHeight() - colourSize) * 0.5f);
                juce::Rectangle<int> colourArea(textArea.getX() + textWidth, colourY, colourSize, colourSize);
                g.fillRect(colourArea);

                juce::Colour backgroundColour = (target->getProperties().contains(LumatoneEditorStyleIDs::popupMenuBackgroundColour))
                    ? juce::Colour::fromString(target->getProperties()[LumatoneEditorStyleIDs::popupMenuBackgroundColour].toString()).withMultipliedSaturation(1.5f) // Box colour will always be highlighted
                    : findColour(LumatoneEditorColourIDs::MenuBarBackground);

                g.setColour(juce::Colour::contrasting(backgroundColour, itemColour));
                g.drawRect(colourArea);
            }
        }

        if (item.isTicked)
        {
            g.setColour(textColour);
            g.setFont(font);
            g.drawFittedText("*", textArea, juce::Justification::centredRight, 1);
        }
    }

    void getIdealPopupMenuItemSizeWithOptions(const juce::String& text, bool isSeparator, int standardMenuItemHeight, int& idealWidth, int& idealHeight,
        const juce::PopupMenu::Options& options) override
    {
        auto target = dynamic_cast<juce::ComboBox*>(options.getTargetComponent());

		if (target != nullptr)
		{
			idealWidth = target->getWidth();

			// If the option is the last available option, add a margin
			if (target->getItemText(target->getNumItems() - 1) == text)
			{
				idealHeight = juce::roundToInt(target->getHeight() * (1 + comboBoxRoundedCornerScalar * 0.5f));
			}
			else
			{
				idealHeight = target->getHeight();
			}

            if (target->getProperties()[LumatoneEditorStyleIDs::comboBoxPopupItemWidthOverride])
            {
                idealWidth = juce::roundToInt((float)target->getProperties()[LumatoneEditorStyleIDs::comboBoxPopupItemWidthOverride]);
            }
            else if (target->getProperties()[LumatoneEditorStyleIDs::comboBoxRenderColourItems])
            {
                juce::Font itemFont = getPopupMenuFont().withHeight(idealHeight * CONTROLBOXFONTHEIGHTSCALAR);
                float colourSize = getPopupMenuColourItemSize(itemFont);
                idealWidth = itemFont.getStringWidth("DDDDDD_") + colourSize;
            }
		}
		else
		  getDefaultLookAndFeel().getIdealPopupMenuItemSizeWithOptions(text, isSeparator, standardMenuItemHeight, idealWidth, idealHeight,
			  options);
    }

    juce::Font getPopupMenuFont() override
    {
        return getAppFont(LumatoneEditorFont::UniviaProBold);
    }

    int getMenuWindowFlags() override
    {
        return juce::ComponentPeer::StyleFlags::windowIsSemiTransparent;
    }

    void preparePopupMenuWindow(juce::Component& window) override
    {
        window.setOpaque(false);
    }

    //==================================================================
    //
    // MENUBAR METHODS
    //
    //==================================================================

    void drawMenuBarBackground(juce::Graphics& g, int w, int h, bool isMouseOverBar, juce::MenuBarComponent& menuBar) override
    {
        g.setColour(findColour(LumatoneEditorColourIDs::MenuBarBackground));
        g.fillRect(0, 0, w, h);
    }

    void drawMenuBarItem(juce::Graphics& g, int w, int h, int index, const juce::String& itemText,
        bool isMouseOver, bool isMenuOpen, bool isMouseOverBar, juce::MenuBarComponent& menuBar) override
    {
        if (isMouseOver)
        {
            juce::Colour bkgdColour = findColour(LumatoneEditorColourIDs::MenuBarBackground).brighter(0.2f);
            g.setColour(bkgdColour);
            g.fillRect(0, 0, w, h);
        }

        g.setColour(findColour(LumatoneEditorColourIDs::ActiveText));
        g.setFont(appFonts.getFont(LumatoneEditorFont::GothamNarrowMedium));
        g.drawFittedText(itemText, 0, 0, w, h, juce::Justification::centred, 1);

    }

    //==================================================================
    //
    // TABBEDBUTTONBAR METHODS
    //
    //==================================================================

    int getTabButtonSpaceAroundImage() override { return 0; }
    int getTabButtonOverlap(int /*tabDepth*/) override { return 0; }
    int getTabButtonBestWidth(juce::TabBarButton& tbb, int /*tabDepth*/) override
    {
        juce::TabbedButtonBar& bar = tbb.getTabbedButtonBar();
        return bar.getWidth() / bar.getNumTabs();
    }
    juce::Rectangle<int> getTabButtonExtraComponentBounds(const juce::TabBarButton& /*tbb*/, juce::Rectangle<int>& /*area*/, juce::Component& /*extraComp*/) override
    {
        return juce::Rectangle<int>(400,400,400,400);
    }
    void drawTabAreaBehindFrontButton(juce::TabbedButtonBar& /*tbb*/, juce::Graphics& /*g*/, int /*w*/, int /*h*/) override {}


    //==================================================================
    //
    // CALLOUTBOX METHODS
    //
    //==================================================================

    void drawCallOutBoxBackground(juce::CallOutBox& box, juce::Graphics& g, const juce::Path& path, juce::Image& img) override
    {
        //juce::Rectangle<float> bounds = box.getLocalBounds().toFloat().reduced(getCallOutBoxBorderSize(box) / 2);
        g.setColour(juce::Colours::black);
        g.fillPath(path);
    }

    int getCallOutBoxBorderSize(const juce::CallOutBox& /*box*/) override
    {
        return 20;
    }

    float getCallOutBoxCornerSize(const juce::CallOutBox& box) override
    {
        return box.getHeight() * 0.0303f;
    }

    //==================================================================
    //
    // ALERTWINDOW METHODS
    //
    //==================================================================

    //virtual juce::AlertWindow* createAlertWindow(const juce::String& title, const juce::String& msg, const juce::String& btn1, const juce::String& btn2, const juce::String& btn3, juce::AlertWindow::AlertIconType type, int numButtons, juce::Component* associatedComponent) override
    //{
    //    auto window = new juce::AlertWindow(title, msg, type, associatedComponent);
    //    window->setSize(480, 240);

    //    if (numButtons > 0)
    //    {
    //        int w = window->getWidth();
    //        int h = window->getHeight();

    //        float btnW = w / 7.0f;
    //        float allMargin = w - btnW * numButtons;
    //        float eachmargin = allMargin / (numButtons + 1);
    //
    //        float btnH = h / 6.0f;
    //
    //        auto firstButton = new juce::TextButton(title + btn1);
    //        firstButton->setButtonText(btn1);
    //        firstButton->setBounds(juce::roundToInt(eachmargin), h - 32, window->getWidth() / window->getWidth() / 7.0, 24);
    //        window->addAndMakeVisible(*firstButton);
    //    }
    //
    //    window->setOpaque(true);

    //    return window;
    //}

    //virtual void drawAlertBox(juce::Graphics& g, juce::AlertWindow& window, const juce::Rectangle<int> &textArea, juce::TextLayout& layout) override
    //{
    //    g.setColour(findColour(juce::AlertWindow::ColourIds::backgroundColourId));
    //    //g.fillRoundedRectangle(window.getLocalBounds().toFloat(), 15);
    //    g.fillAll();

    //    g.setColour(findColour(juce::AlertWindow::ColourIds::outlineColourId));
    //    g.drawRect(window.getLocalBounds(), 2);

    //    g.setColour(findColour(juce::AlertWindow::ColourIds::textColourId));
    //    float widthRatio = 0.05f;
    //    float heightRatio = 0.15f;
    //
    //    layout.draw(g,
    //        textArea.constrainedWithin(
    //            window.getLocalBounds().reduced(window.getWidth() * widthRatio, window.getHeight() * heightRatio)
    //        ).toFloat());
    //}

    //virtual juce::Array<int> getWidthsForTextButtons(juce::AlertWindow& window, const juce::Array<juce::TextButton*>& btns) override
    //{
    //    return getDefaultLookAndFeel().getWidthsForTextButtons(window, btns);
    //}

    //virtual int getAlertWindowButtonHeight() override
    //{
    //    return 16;
    //}

    //virtual juce::Font getAlertWindowTitleFont() override
    //{
    //    return getAppFont(LumatoneEditorFont::UniviaProBold).withHeight(18);
    //}

    //virtual juce::Font getAlertWindowMessageFont()  override
    //{
    //    return getAppFont(LumatoneEditorFont::GothamNarrowMedium).withHeight(16);
    //}

    //virtual juce::Font getAlertWindowFont()  override
    //{
    //    return getAppFont(LumatoneEditorFont::GothamNarrowMedium).withHeight(14);
    //}

public:
    //==============================================================================================
    // Component Setup functions

    // Set colours of TextButton based on Live and Offline Editor buttons
    void setupRadioTextButton(juce::Button& btn, int radioGroup, bool initalToggledState)
    {
        btn.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colour(0xff1c1c1c));
        btn.setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colour(0xff383b3d));
        btn.setColour(juce::TextButton::ColourIds::textColourOffId, juce::Colour(0xffb5b5b5));
        btn.setColour(juce::TextButton::ColourIds::textColourOnId, juce::Colour(0xffffffff));
        btn.setClickingTogglesState(true);
        btn.setRadioGroupId(radioGroup, juce::dontSendNotification);
        btn.setToggleState(initalToggledState, juce::dontSendNotification);
    }

    // Set colours of Image button based on Live and Offline Editor Buttons
    // Use the same image for Normal, Highlighted, and Down, but apply alpha layer on Highlighted
    //void setupImageButton(juce::ImageButton& btn, juce::Image btnImage)
    //{
    //    btn.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colour(0xff1c1c1c));
    //    btn.setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colour(0xff383b3d));

    //    btn.setImages(false, true, true,
    //        btnImage, 1.0f, juce::Colour(),
    //        btnImage, 1.0f, juce::Colours::white.withAlpha(0.1f),
    //        btnImage, 1.0f, juce::Colour()
    //    );
    //}

    // Set Slider style to Rotary
    void setupRotarySlider(juce::Slider& sld)
    {
        sld.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    }

    // Set generic TextEditor colours
    void setupTextEditor(juce::TextEditor& editor)
    {
        editor.setIndents(4, 0);
        editor.setColour(juce::TextEditor::ColourIds::backgroundColourId, findColour(juce::TextEditor::ColourIds::backgroundColourId));
        editor.setColour(juce::TextEditor::ColourIds::textColourId, findColour(juce::TextEditor::ColourIds::textColourId));
        editor.setColour(juce::TextEditor::ColourIds::outlineColourId, juce::Colour());

    }

public:
    //==============================================================================================
    // Non-static drawing helpers

    // Button shape path helper with default rounded corner size
    juce::Path getButtonShape(const juce::Button& btn)
    {
        return getConnectedRoundedRectPath(btn.getLocalBounds().toFloat(), btn.getHeight() * buttonRoundedCornerScalar, btn.getConnectedEdgeFlags());
    }

private:
    //==============================================================================================

    /// <summary>
    /// MUST be called a single time via the LookAndFeel constructor in the MainComponent
    /// </summary>
    void cacheImages()
    {
        LumatoneAssets::LoadAssets(LumatoneAssets::ID::LumatoneGraphic);
        LumatoneAssets::LoadAssets(LumatoneAssets::ID::KeyShape);
        LumatoneAssets::LoadAssets(LumatoneAssets::ID::KeyShadow);

        juce::ImageCache::addImageToCache(juce::ImageCache::getFromMemory(BinaryData::TrashCanIcon2x_png, BinaryData::TrashCanIcon2x_pngSize), (juce::int64)LumatoneAssets::ID::TrashCanIcon);
    }

    void cacheIcons()
    {
        saveIconPath = getSaveIconPath();
        arrowUpIconPath = getArrowPath(juce::Point<float>(0.5f, 0.96f), juce::Point<float>(0.5f, 0.08f), 0.55, 0.333f);
        arrowDownIconPath = getArrowPath(juce::Point<float>(0.5f, 0.08f), juce::Point<float>(0.5f, 0.96f), 0.55, 0.667f);
        colourPickerPath = getPickerIconPath();
    }

    /// <summary>
    /// Sets the LookAndFeel colour palette to default colours
    /// </summary>
    void setupDefaultColours()
    {
        // Colour references
        setColour(LumatoneEditorColourIDs::TitlePink,                       juce::Colour(0xffdfceca));
        setColour(LumatoneEditorColourIDs::LabelPink,                       juce::Colour(0xffcecece));
        setColour(LumatoneEditorColourIDs::LabelBlue,                       juce::Colour(0xff60aac5));
        setColour(LumatoneEditorColourIDs::DisconnectedRed,                 juce::Colour(0xffd7002a));
        setColour(LumatoneEditorColourIDs::ConnectedGreen,                  juce::Colour(0xff84aea3));
        setColour(LumatoneEditorColourIDs::HeaderBackground,                juce::Colour(0xff1a1b1c));
        setColour(LumatoneEditorColourIDs::MediumBackground,                juce::Colour(0xff212529));
        setColour(LumatoneEditorColourIDs::LightBackground,                 juce::Colour(0xff272b2e));
        setColour(LumatoneEditorColourIDs::ControlAreaHeader,               juce::Colour(0xff272b2e));
        setColour(LumatoneEditorColourIDs::ControlAreaBackground,           juce::Colour(0xff2d3135));
        setColour(LumatoneEditorColourIDs::ColourPaletteBackground,         juce::Colour(0xff292b2d));
        setColour(LumatoneEditorColourIDs::ActiveText,                      juce::Colours::white);
        setColour(LumatoneEditorColourIDs::InactiveText,                    juce::Colour(0xffb1b1b1));
        setColour(LumatoneEditorColourIDs::DescriptionText,                 juce::Colour(0xffcbcbcb));
        setColour(LumatoneEditorColourIDs::NumKeySelectedText,              juce::Colour(0xffd8697e));
        setColour(LumatoneEditorColourIDs::ControlBoxBackground,            juce::Colour(0xff1e2222));
        setColour(LumatoneEditorColourIDs::ControlBoxHighlighted,           juce::Colour(0xff3d5a78));
        setColour(LumatoneEditorColourIDs::DefaultPresetButtonActive,       juce::Colour(0xffff84e6));
        setColour(LumatoneEditorColourIDs::DefaultPresetButtonInactive,     juce::Colour(0xff5c7cf2));
        setColour(LumatoneEditorColourIDs::OutlineColourId,                 juce::Colours::white);
        setColour(LumatoneEditorColourIDs::CurveGradientMin,                juce::Colour(0xffbf961e));
        setColour(LumatoneEditorColourIDs::CurveGradientMax,                juce::Colour(0xffcd6f2e));
        setColour(LumatoneEditorColourIDs::CurveGridColour,                 juce::Colour(0xff303030));
        setColour(LumatoneEditorColourIDs::DisabledOverlay,                 juce::Colour(0x601b1b1b));
        setColour(LumatoneEditorColourIDs::MenuBarBackground,               juce::Colour(0xff1a1b1c));

        // Gradients
        setColour(LumatoneEditorColourIDs::ExprRotaryGradientMin,           juce::Colour(0xff5497b6));
        setColour(LumatoneEditorColourIDs::ExprRotaryGradientMax,           juce::Colour(0xff77a8b3));
        setColour(LumatoneEditorColourIDs::BrightnessRotaryGradientMin,     juce::Colour(0xff293044));
        setColour(LumatoneEditorColourIDs::BrightnessRotaryGradientMax,     juce::Colour(0xffaac1dd));
        setColour(LumatoneEditorColourIDs::HueRotaryGradientMin,            juce::Colour(0xffd4838b));
        setColour(LumatoneEditorColourIDs::HueRotaryGradientMid,            juce::Colour(0xff8dd48f));
        setColour(LumatoneEditorColourIDs::HueRotaryGradientMax,            juce::Colour(0xff8495d4));
        setColour(LumatoneEditorColourIDs::TempRotaryGradientMin,           juce::Colour(0xffd4838b));
        setColour(LumatoneEditorColourIDs::TempRotaryGradientMid,           juce::Colour(0xffe8e8e8));
        setColour(LumatoneEditorColourIDs::TempRotaryGradientMax,           juce::Colour(0xff8495d4));


        // Component defaults
        setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colour(0xff383b3d));
        setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colour(0xff383b3d));
        setColour(juce::TextButton::ColourIds::textColourOnId, juce::Colours::white);
        setColour(juce::TextButton::ColourIds::textColourOffId, juce::Colours::white);

        setColour(juce::ToggleButton::ColourIds::textColourId, findColour(LumatoneEditorColourIDs::DescriptionText));

        setColour(juce::ComboBox::ColourIds::backgroundColourId, findColour(LumatoneEditorColourIDs::ControlBoxBackground));
        setColour(juce::ComboBox::ColourIds::textColourId, findColour(LumatoneEditorColourIDs::DescriptionText));

        setColour(juce::TextEditor::ColourIds::backgroundColourId, findColour(LumatoneEditorColourIDs::ControlBoxBackground));
        setColour(juce::TextEditor::ColourIds::textColourId, findColour(LumatoneEditorColourIDs::DescriptionText));
        setColour(juce::TextEditor::ColourIds::highlightColourId, findColour(LumatoneEditorColourIDs::ControlBoxHighlighted));
        setColour(juce::TextEditor::ColourIds::highlightedTextColourId, findColour(LumatoneEditorColourIDs::DescriptionText));
        setColour(juce::TextEditor::ColourIds::shadowColourId, juce::Colour());

        setColour(juce::AlertWindow::ColourIds::backgroundColourId, findColour(LumatoneEditorColourIDs::HeaderBackground));
        setColour(juce::AlertWindow::ColourIds::textColourId, findColour(LumatoneEditorColourIDs::DescriptionText));
        setColour(juce::AlertWindow::ColourIds::outlineColourId, findColour(LumatoneEditorColourIDs::MediumBackground));
    }

private:

    const LumatoneEditorFontLibrary& appFonts;

    juce::Path saveIconPath;
    juce::Path arrowUpIconPath;
    juce::Path arrowDownIconPath;
//    juce::Path ccPolarityDefaultIconPath;
//    juce::Path ccPolarityInvertedIconPath;
    juce::Path colourPickerPath;

    // Default graphics constants
    const float buttonRoundedCornerScalar = 0.2f;

    const float comboBoxRoundedCornerScalar = 0.304878f;

    const float rotaryAngleStart = juce::MathConstants<float>::pi * -0.64f; // pi * -2/3
    const float rotaryAngleEnd = -rotaryAngleStart;

public:
    LumatoneEditorCompactWindow compactWindowStyle;

};
