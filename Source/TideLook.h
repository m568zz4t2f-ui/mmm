// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
namespace tide::ui {
inline const juce::Colour chassis(0xff19272e),surface(0xff1a282e),ink(0xfffff4e4),muted(0xffc3d0cf),delay(0xffd6cbb8),reverb(0xff77d7d5),edge(0xff6d8082);
juce::Font font(float size,int style=juce::Font::plain);
enum class Skin {chassis,delay,reverb,master,knob,glass,button_normal,button_hover,button_pressed,button_delay,button_reverb,button_disabled,track,thumb,body,selected_row,chevron,count,expanded=chassis,field_normal=button_pressed,field_hover=button_hover,field_active=button_reverb,field_disabled=button_disabled};
void paintSkin(juce::Graphics&,juce::Rectangle<float>,Skin,float opacity=1.f);
std::size_t cachedSkinBytes();

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
 void drawButtonText(juce::Graphics&,juce::TextButton&,bool,bool)override;
 void drawLabel(juce::Graphics&,juce::Label&)override;
 void drawTickBox(juce::Graphics&,juce::Component&,float,float,float,float,bool,bool,bool,bool)override;
 void drawToggleButton(juce::Graphics&,juce::ToggleButton&,bool,bool)override;
 void fillTextEditorBackground(juce::Graphics&,int,int,juce::TextEditor&)override;
 void drawTextEditorOutline(juce::Graphics&,int,int,juce::TextEditor&)override;
 void drawPopupMenuBackground(juce::Graphics&,int,int)override;
 void drawPopupMenuItem(juce::Graphics&,const juce::Rectangle<int>&,bool,bool,bool,bool,bool,const juce::String&,const juce::String&,const juce::Drawable*,const juce::Colour*)override;
 void drawTooltip(juce::Graphics&,const juce::String&,int,int)override;
 juce::Rectangle<int> getTooltipBounds(const juce::String&,juce::Point<int>,juce::Rectangle<int>)override;
 void drawCallOutBoxBackground(juce::CallOutBox&,juce::Graphics&,const juce::Path&,juce::Image&)override;
 void drawAlertBox(juce::Graphics&,juce::AlertWindow&,const juce::Rectangle<int>&,juce::TextLayout&)override;
 juce::AlertWindow* createAlertWindow(const juce::String&,const juce::String&,const juce::String&,const juce::String&,const juce::String&,juce::MessageBoxIconType,int,juce::Component*)override;
 void drawScrollbar(juce::Graphics&,juce::ScrollBar&,int,int,int,int,bool,int,int,bool,bool)override;
 void drawLinearSlider(juce::Graphics&,int,int,int,int,float,float,float,juce::Slider::SliderStyle,juce::Slider&)override;
};

namespace tide::ui { TideLook& sharedLook(); }
