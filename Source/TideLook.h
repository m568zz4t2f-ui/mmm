// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
namespace tide::ui {
inline const juce::Colour chassis(0xff19272e),surface(0xff1a282e),ink(0xfff4eadc),muted(0xffb6beb9),delay(0xffd6cbb8),reverb(0xff77d7d5),edge(0xff4b6063);
inline juce::Font font(float size,int style=juce::Font::plain){return juce::Font(juce::FontOptions(juce::Font::getDefaultSansSerifFontName(),size,style));}
}
class TideLook final:public juce::LookAndFeel_V4 {
public:
 TideLook();
 juce::Font getComboBoxFont(juce::ComboBox&)override{return tide::ui::font(13);}
 juce::Font getTextButtonFont(juce::TextButton&,int)override{return tide::ui::font(11.5f,juce::Font::bold);}
 juce::Font getPopupMenuFont()override{return tide::ui::font(13);}
 juce::Font getLabelFont(juce::Label& label)override{return label.getFont();}
 juce::Font getAlertWindowTitleFont()override{return tide::ui::font(19,juce::Font::bold);}
 juce::Font getAlertWindowMessageFont()override{return tide::ui::font(14);}
 juce::Font getAlertWindowFont()override{return tide::ui::font(14);}
 juce::Label* createSliderTextBox(juce::Slider&)override;
 void drawRotarySlider(juce::Graphics&,int,int,int,int,float,float,float,juce::Slider&)override;
 void drawButtonBackground(juce::Graphics&,juce::Button&,const juce::Colour&,bool,bool)override;
 void drawComboBox(juce::Graphics&,int,int,bool,int,int,int,int,juce::ComboBox&)override;
 void positionComboBoxText(juce::ComboBox&,juce::Label&)override;
 void drawLinearSlider(juce::Graphics&,int,int,int,int,float,float,float,juce::Slider::SliderStyle,juce::Slider&)override;
};
