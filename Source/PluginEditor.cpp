// SPDX-License-Identifier: AGPL-3.0-or-later
#include "PluginEditor.h"
#include "PresetBrowser.h"
#include "ModernPanel.h"
#include "BinaryData.h"
namespace {
using tide::ui::ink;using tide::ui::muted;
const auto silver=tide::ui::delay,blue=tide::ui::reverb,bg=tide::ui::chassis;
using SA=juce::AudioProcessorValueTreeState::SliderAttachment;
using CA=juce::AudioProcessorValueTreeState::ComboBoxAttachment;
using BA=juce::AudioProcessorValueTreeState::ButtonAttachment;
constexpr int mainIndices[]={0,1,2,3,11,12,13,14};
constexpr const char* tips[]={
 "Delay time in milliseconds. Turn SYNC off to edit freely.","Regeneration per pass. Kept below runaway feedback.","Repeat high-frequency cutoff; tape and bucket modes add their own coloration.","Audible delay return, independent of its reverb send.",
 "Delay modulation depth. Adds pitch movement.","Stereo width of repeats. 100% preserves their stereo field.","Soft saturation in Tape and Bucket modes.","Bass removed from repeating echoes.","Ducks echoes while the input is loud; they recover in the gaps.","Rate of delay modulation.","Second delay time as a ratio of the first, in Dual mode.",
 "Nominal decay. Room caps at 8 s; Hall/Bloom at 20 s. Damping shortens higher frequencies.","Time before the reverb begins.","Treble damping inside the reverb.","Audible reverb return.","Slow modulation in Hall and Bloom.","Stereo width of the reverb return. Check wide settings in mono.","Frequency used by the reverb bass-weight filter.","Ducks the reverb while you play or sing.","Pitch-shifted atmosphere and bounded feedback. Uses its own shimmer interval selector.","Amount of bass retained below the low-cut frequency.",
 "Stereo delay feed into reverb, independent of Delay Amount. Inactive in Parallel routing.","Original performance feed into reverb.","Global dry/wet blend. Send mode keeps the dry path silent.","Final gain trim. Amber meter means output exceeds 0 dBFS.","Performance macro: increases delay feedback and reverb decay within bounded ranges.","Tempo used when the host does not supply BPM.","Detector sensitivity for delay ducking. Raise for quieter recordings.","Time for echoes to recover after the input quiets.","Detector sensitivity for reverb ducking. Raise for quieter recordings.","Time for reverb to recover after the input quiets."
};
void label(juce::Graphics& g,const juce::String& text,float x,float y,float w,float h,float size,juce::Colour colour=ink){g.setColour(colour);g.setFont(tide::ui::font(size));g.drawText(text,juce::Rectangle<float>(x,y,w,h),juce::Justification::centredLeft);}
}
namespace tide::ui {
namespace {
struct SkinInfo { juce::Image image; int marginX,marginY; float corner; };
const auto& artwork(){
 static const std::array<SkinInfo,std::size_t(Skin::count)> data={{
  {juce::ImageFileFormat::loadFrom(BinaryData::chassis_png,BinaryData::chassis_pngSize),32,32,10.f},
  {juce::ImageFileFormat::loadFrom(BinaryData::expanded_png,BinaryData::expanded_pngSize),32,32,10.f},
  {juce::ImageFileFormat::loadFrom(BinaryData::delay_png,BinaryData::delay_pngSize),42,42,12.f},
  {juce::ImageFileFormat::loadFrom(BinaryData::reverb_png,BinaryData::reverb_pngSize),42,42,12.f},
  {juce::ImageFileFormat::loadFrom(BinaryData::master_png,BinaryData::master_pngSize),40,40,8.f},
  {juce::ImageFileFormat::loadFrom(BinaryData::knob_png,BinaryData::knob_pngSize),0,0,0.f},
  {juce::ImageFileFormat::loadFrom(BinaryData::glass_png,BinaryData::glass_pngSize),54,54,8.f},
  {juce::ImageFileFormat::loadFrom(BinaryData::button_normal_png,BinaryData::button_normal_pngSize),24,24,5.f},
  {juce::ImageFileFormat::loadFrom(BinaryData::button_hover_png,BinaryData::button_hover_pngSize),24,24,5.f},
  {juce::ImageFileFormat::loadFrom(BinaryData::button_pressed_png,BinaryData::button_pressed_pngSize),24,24,5.f},
  {juce::ImageFileFormat::loadFrom(BinaryData::button_delay_png,BinaryData::button_delay_pngSize),24,24,5.f},
  {juce::ImageFileFormat::loadFrom(BinaryData::button_reverb_png,BinaryData::button_reverb_pngSize),24,24,5.f},
  {juce::ImageFileFormat::loadFrom(BinaryData::button_disabled_png,BinaryData::button_disabled_pngSize),24,24,5.f},
  {juce::ImageFileFormat::loadFrom(BinaryData::field_normal_png,BinaryData::field_normal_pngSize),24,24,5.f},
  {juce::ImageFileFormat::loadFrom(BinaryData::field_hover_png,BinaryData::field_hover_pngSize),24,24,5.f},
  {juce::ImageFileFormat::loadFrom(BinaryData::field_active_png,BinaryData::field_active_pngSize),24,24,5.f},
  {juce::ImageFileFormat::loadFrom(BinaryData::field_disabled_png,BinaryData::field_disabled_pngSize),24,24,5.f},
  {juce::ImageFileFormat::loadFrom(BinaryData::track_png,BinaryData::track_pngSize),18,8,4.f},
  {juce::ImageFileFormat::loadFrom(BinaryData::thumb_png,BinaryData::thumb_pngSize),0,0,0.f},
  {juce::ImageFileFormat::loadFrom(BinaryData::body_png,BinaryData::body_pngSize),72,72,8.f},
  {juce::ImageFileFormat::loadFrom(BinaryData::selected_row_png,BinaryData::selected_row_pngSize),12,12,3.f},
  {juce::ImageFileFormat::loadFrom(BinaryData::chevron_png,BinaryData::chevron_pngSize),0,0,0.f},
 }};return data;
}
// GUI thread only. Slice geometry and highlights remain fixed; only native values move.
void nine(juce::Graphics& g,juce::Rectangle<float> r,const SkinInfo& art,float corner){
 const auto& im=art.image;
 if(art.marginX==0&&art.marginY==0){g.drawImage(im,r);return;}
 const float c=std::min({corner,r.getWidth()*.5f,r.getHeight()*.5f});
 const int mx=art.marginX,my=art.marginY,w=im.getWidth(),h=im.getHeight();
 const std::array<int,4> sx={0,mx,w-mx,w},sy={0,my,h-my,h};
 const std::array<float,4> dx={r.getX(),r.getX()+c,r.getRight()-c,r.getRight()},dy={r.getY(),r.getY()+c,r.getBottom()-c,r.getBottom()};
 for(int y=0;y<3;++y)for(int x=0;x<3;++x)
  g.drawImage(im,dx[std::size_t(x)],dy[std::size_t(y)],dx[std::size_t(x+1)]-dx[std::size_t(x)],dy[std::size_t(y+1)]-dy[std::size_t(y)],sx[std::size_t(x)],sy[std::size_t(y)],sx[std::size_t(x+1)]-sx[std::size_t(x)],sy[std::size_t(y+1)]-sy[std::size_t(y)]);
}
}
juce::Font font(float size,int style){
 // Keep the original italic wordmark; functional text uses licensed embedded Plex.
 if((style&juce::Font::italic)!=0)return juce::Font(juce::FontOptions(juce::Font::getDefaultSansSerifFontName(),size,style));
 static auto regular=juce::Typeface::createSystemTypefaceFor(BinaryData::IBMPlexSansRegular_ttf,BinaryData::IBMPlexSansRegular_ttfSize);
 static auto medium=juce::Typeface::createSystemTypefaceFor(BinaryData::IBMPlexSansMedium_ttf,BinaryData::IBMPlexSansMedium_ttfSize);
 return juce::Font(juce::FontOptions((style&juce::Font::bold)!=0?medium:regular).withHeight(size));
}
void paintSkin(juce::Graphics& g,juce::Rectangle<float> r,Skin skin,float opacity){
 const auto& info=artwork()[std::size_t(skin)];g.saveState();g.setOpacity(opacity);
 // Static large surfaces already live in the editor backdrop. Bound the shared small-skin cache.
 if(r.getWidth()*r.getHeight()<50000){
  const float scale=g.getInternalContext().getPhysicalPixelScaleFactor();
  const int w=std::max(1,juce::roundToInt(r.getWidth()*scale)),h=std::max(1,juce::roundToInt(r.getHeight()*scale));
  struct Entry {Skin id=Skin::count;int w=0,h=0;juce::Image image;};
  static std::array<Entry,96> cache;static std::size_t next=0;
  Entry* found=nullptr;for(auto& e:cache)if(e.id==skin&&e.w==w&&e.h==h){found=&e;break;}
  if(found==nullptr){found=&cache[next++%cache.size()];*found={skin,w,h,juce::Image(juce::Image::ARGB,w,h,true)};juce::Graphics cg(found->image);nine(cg,{0,0,float(w),float(h)},info,info.corner*scale);}
  g.drawImage(found->image,r);
 }else nine(g,r,info,info.corner);
 g.restoreState();
}
TideLook& sharedLook(){static TideLook look;return look;}
}
namespace {
const juce::Image& sizedKnobArtwork(int pixels){
 static std::array<std::pair<int,juce::Image>,8> sizes;static std::size_t next=0;
 for(const auto& item:sizes)if(item.first==pixels)return item.second;
 auto& item=sizes[next++%sizes.size()];item={pixels,tide::ui::artwork()[std::size_t(tide::ui::Skin::knob)].image.rescaled(pixels,pixels,juce::Graphics::highResamplingQuality)};return item.second;
}
void focusBorder(juce::Graphics& g,juce::Rectangle<float> r,juce::Colour colour){g.setColour(colour);g.drawRoundedRectangle(r.reduced(1.8f),3.5f,1.3f);}
}
TideLook::TideLook(){
 using namespace tide::ui;
 for(int colour:std::initializer_list<int>{juce::ComboBox::backgroundColourId,juce::PopupMenu::backgroundColourId,juce::TextEditor::backgroundColourId,juce::ListBox::backgroundColourId,juce::AlertWindow::backgroundColourId})setColour(colour,chassis);
 for(int colour:std::initializer_list<int>{juce::ComboBox::textColourId,juce::PopupMenu::textColourId,juce::Slider::textBoxTextColourId,juce::Label::textColourId,juce::TextButton::textColourOffId,juce::TextButton::textColourOnId,juce::ToggleButton::textColourId,juce::TextEditor::textColourId,juce::AlertWindow::textColourId})setColour(colour,ink);
 setColour(juce::PopupMenu::highlightedBackgroundColourId,reverb);setColour(juce::PopupMenu::highlightedTextColourId,ink);
 setColour(juce::Slider::textBoxOutlineColourId,juce::Colours::transparentBlack);setColour(juce::Slider::textBoxBackgroundColourId,juce::Colours::transparentBlack);
 setColour(juce::Slider::trackColourId,reverb);setColour(juce::Slider::thumbColourId,reverb);setColour(juce::ToggleButton::tickColourId,reverb);
 setColour(juce::ComboBox::outlineColourId,edge);setColour(juce::TextEditor::outlineColourId,edge);setColour(juce::TextEditor::focusedOutlineColourId,reverb);setColour(juce::TextEditor::highlightColourId,reverb.withAlpha(.25f));setColour(juce::ListBox::outlineColourId,edge);setColour(juce::ScrollBar::thumbColourId,reverb);setColour(juce::AlertWindow::outlineColourId,edge);
 (void)artwork();(void)font(13);
}
juce::Label* TideLook::createSliderTextBox(juce::Slider& slider){auto* box=juce::LookAndFeel_V4::createSliderTextBox(slider);box->setFont(tide::ui::font(13.5f));return box;}
void TideLook::drawButtonBackground(juce::Graphics& g,juce::Button& button,const juce::Colour&,bool hover,bool down){
 using namespace tide::ui;const bool left=dynamic_cast<TideEditor*>(button.getParentComponent())!=nullptr&&button.getX()<button.getParentWidth()/2&&button.getY()>100;
 const auto active=left?Skin::button_delay:Skin::button_reverb;
 auto skin=!button.isEnabled()?Skin::button_disabled:down?Skin::button_pressed:button.getToggleState()?active:hover?Skin::button_hover:Skin::button_normal;
 paintSkin(g,button.getLocalBounds().toFloat().reduced(.5f),skin,button.isEnabled()?1.f:.75f);
 if(button.getToggleState()||button.hasKeyboardFocus(true))focusBorder(g,button.getLocalBounds().toFloat(),left?delay:reverb);
}
void TideLook::drawButtonText(juce::Graphics& g,juce::TextButton& b,bool,bool){g.setColour(tide::ui::ink.withAlpha(b.isEnabled()?1.f:.7f));g.setFont(getTextButtonFont(b,b.getHeight()));g.drawFittedText(b.getButtonText(),b.getLocalBounds().reduced(5,2),juce::Justification::centred,1,.85f);}
void TideLook::drawLabel(juce::Graphics& g,juce::Label& label){
 const bool value=dynamic_cast<juce::Slider*>(label.getParentComponent())!=nullptr;
 if(value)tide::ui::paintSkin(g,label.getLocalBounds().toFloat(),tide::ui::Skin::field_normal,label.isEnabled()?1.f:.65f);
 if(!label.isBeingEdited()){g.setColour((value?tide::ui::ink:label.findColour(juce::Label::textColourId)).withAlpha(label.isEnabled()?1.f:.7f));g.setFont(label.getFont());g.drawFittedText(label.getText(),label.getBorderSize().subtractedFrom(label.getLocalBounds()),label.getJustificationType(),1,.85f);}
}
void TideLook::drawComboBox(juce::Graphics& g,int w,int h,bool down,int,int,int,int,juce::ComboBox& box){
 using namespace tide::ui;paintSkin(g,{0,0,float(w),float(h)},!box.isEnabled()?Skin::field_disabled:box.hasKeyboardFocus(true)?Skin::field_active:box.isMouseOverOrDragging()?Skin::field_hover:Skin::field_normal);
 if(down||box.hasKeyboardFocus(true))focusBorder(g,{0,0,float(w),float(h)},reverb);
 paintSkin(g,{float(w)-22,float(h)*.5f-3.5f,11,7},Skin::chevron,box.isEnabled()?1.f:.6f);
}
void TideLook::positionComboBoxText(juce::ComboBox& box,juce::Label& label){label.setBounds(10,1,box.getWidth()-33,box.getHeight()-2);label.setFont(getComboBoxFont(box));}
void TideLook::drawLinearSlider(juce::Graphics& g,int x,int y,int w,int h,float position,float,float,juce::Slider::SliderStyle,juce::Slider& slider){
 using namespace tide::ui;const float cy=float(y)+float(h)*.5f,left=float(x)+5,right=float(x+w)-5;const float alpha=slider.isEnabled()?1.f:.5f;
 paintSkin(g,{left,cy-3,right-left,6},Skin::track,alpha);
 g.setColour(slider.findColour(juce::Slider::trackColourId).withAlpha(alpha));g.fillRoundedRectangle(left+3,cy-1,std::max(0.f,std::min(position,right)-left-3),2,1);
 paintSkin(g,{position-3.5f,cy-10,7,20},Skin::thumb,alpha);
 if(slider.hasKeyboardFocus(true))focusBorder(g,{left,cy-11,right-left,22},reverb);
}
void TideLook::drawTickBox(juce::Graphics& g,juce::Component&,float x,float y,float w,float h,bool ticked,bool enabled,bool hover,bool down){
 using namespace tide::ui;paintSkin(g,{x,y,w,h},!enabled?Skin::button_disabled:down?Skin::button_pressed:ticked?Skin::button_reverb:hover?Skin::button_hover:Skin::button_normal,enabled?1.f:.7f);
 if(ticked){juce::Path tick;tick.startNewSubPath(x+w*.24f,y+h*.52f);tick.lineTo(x+w*.43f,y+h*.7f);tick.lineTo(x+w*.78f,y+h*.28f);g.setColour(ink.withAlpha(enabled?1.f:.7f));g.strokePath(tick,juce::PathStrokeType(1.7f));}
}
void TideLook::drawToggleButton(juce::Graphics& g,juce::ToggleButton& b,bool hover,bool down){const float y=float(b.getHeight())*.5f-9;drawTickBox(g,b,5,y,18,18,b.getToggleState(),b.isEnabled(),hover,down);g.setColour(tide::ui::ink.withAlpha(b.isEnabled()?1.f:.7f));g.setFont(tide::ui::font(13));g.drawText(b.getButtonText(),32,0,b.getWidth()-36,b.getHeight(),juce::Justification::centredLeft);if(b.hasKeyboardFocus(true))focusBorder(g,b.getLocalBounds().toFloat(),tide::ui::reverb);}
void TideLook::fillTextEditorBackground(juce::Graphics& g,int w,int h,juce::TextEditor& editor){tide::ui::paintSkin(g,{0,0,float(w),float(h)},editor.isEnabled()?tide::ui::Skin::field_normal:tide::ui::Skin::field_disabled);}
void TideLook::drawTextEditorOutline(juce::Graphics& g,int w,int h,juce::TextEditor& e){if(e.hasKeyboardFocus(true))focusBorder(g,{0,0,float(w),float(h)},tide::ui::reverb);}
void TideLook::drawPopupMenuBackground(juce::Graphics& g,int w,int h){tide::ui::paintSkin(g,{0,0,float(w),float(h)},tide::ui::Skin::body);}
void TideLook::drawPopupMenuItem(juce::Graphics& g,const juce::Rectangle<int>& area,bool separator,bool active,bool highlighted,bool ticked,bool submenu,const juce::String& text,const juce::String& shortcut,const juce::Drawable* icon,const juce::Colour* colour){
 using namespace tide::ui;auto r=area.reduced(4,0);
 if(separator){g.setColour(edge.withAlpha(.45f));g.fillRect(r.getX()+8,r.getCentreY(),r.getWidth()-16,1);return;}
 if(highlighted&&active)paintSkin(g,r.toFloat(),Skin::selected_row);
 if(ticked){juce::Path tick;const float x=float(r.getX()+8),y=float(r.getCentreY());tick.startNewSubPath(x,y);tick.lineTo(x+4,y+4);tick.lineTo(x+10,y-4);g.setColour(reverb);g.strokePath(tick,juce::PathStrokeType(1.7f));}
 if(icon)icon->drawWithin(g,{float(r.getX()+4),float(r.getY()+3),18,float(r.getHeight()-6)},juce::RectanglePlacement::centred,active?1.f:.6f);
 g.setColour((colour?*colour:ink).withAlpha(active?1.f:.6f));g.setFont(getPopupMenuFont());g.drawText(text,r.withTrimmedLeft(27).withTrimmedRight(35),juce::Justification::centredLeft,true);
 if(shortcut.isNotEmpty()){g.setFont(tide::ui::font(11));g.drawText(shortcut,r.withTrimmedRight(20),juce::Justification::centredRight,true);}
 if(submenu){g.saveState();const float x=float(r.getRight()-12),y=float(r.getCentreY());g.addTransform(juce::AffineTransform::rotation(-juce::MathConstants<float>::halfPi,x,y));paintSkin(g,{x-5,y-3,10,6},Skin::chevron,active?1.f:.6f);g.restoreState();}
}
namespace {
juce::TextLayout tooltipText(const juce::String& text){juce::AttributedString a;a.setFont(tide::ui::font(13));a.setColour(tide::ui::ink);a.append(text);juce::TextLayout layout;layout.createLayoutWithBalancedLineLengths(a,380);return layout;}
}
juce::Rectangle<int> TideLook::getTooltipBounds(const juce::String& text,juce::Point<int> position,juce::Rectangle<int> parent){const auto layout=tooltipText(text);return juce::Rectangle<int>(position.x+12,position.y+22,juce::roundToInt(layout.getWidth())+24,juce::roundToInt(layout.getHeight())+20).constrainedWithin(parent);}
void TideLook::drawTooltip(juce::Graphics& g,const juce::String& text,int w,int h){tide::ui::paintSkin(g,{0,0,float(w),float(h)},tide::ui::Skin::body);tooltipText(text).draw(g,{12,10,float(w-24),float(h-20)});}
void TideLook::drawCallOutBoxBackground(juce::CallOutBox& box,juce::Graphics& g,const juce::Path& path,juce::Image&){g.saveState();g.reduceClipRegion(path);tide::ui::paintSkin(g,box.getLocalBounds().toFloat(),tide::ui::Skin::body);g.restoreState();}
juce::AlertWindow* TideLook::createAlertWindow(const juce::String& title,const juce::String& text,const juce::String& b1,const juce::String& b2,const juce::String& b3,juce::MessageBoxIconType type,int buttons,juce::Component* component){auto* window=juce::LookAndFeel_V4::createAlertWindow(title,text,b1,b2,b3,type,buttons,component);window->setLookAndFeel(&tide::ui::sharedLook());return window;}
void TideLook::drawAlertBox(juce::Graphics& g,juce::AlertWindow& alert,const juce::Rectangle<int>&,juce::TextLayout& layout){tide::ui::paintSkin(g,alert.getLocalBounds().toFloat(),tide::ui::Skin::body);const bool icon=alert.getAlertType()!=juce::MessageBoxIconType::NoIcon;if(icon){g.setColour(alert.getAlertType()==juce::MessageBoxIconType::WarningIcon?juce::Colour(0xffffbc7a):tide::ui::reverb);g.setFont(tide::ui::font(48,juce::Font::bold));g.drawText(alert.getAlertType()==juce::MessageBoxIconType::WarningIcon?"!":alert.getAlertType()==juce::MessageBoxIconType::InfoIcon?"i":"?",12,25,56,80,juce::Justification::centred);}layout.draw(g,{float(icon?81:1),30,float(alert.getWidth()-2),float(alert.getHeight()-getAlertWindowButtonHeight()-22)});}
void TideLook::drawScrollbar(juce::Graphics& g,juce::ScrollBar&,int x,int y,int w,int h,bool vertical,int start,int size,bool hover,bool down){if(size>0)tide::ui::paintSkin(g,vertical?juce::Rectangle<float>(float(x+2),float(start),float(w-4),float(size)):juce::Rectangle<float>(float(start),float(y+2),float(size),float(h-4)),down?tide::ui::Skin::button_pressed:hover?tide::ui::Skin::button_hover:tide::ui::Skin::button_normal);}
void TideLook::drawRotarySlider(juce::Graphics& g,int x,int y,int w,int h,float pos,float start,float end,juce::Slider& slider){
 const auto r=juce::Rectangle<float>(float(x),float(y),float(w),float(h)).reduced(6);const float diameter=std::min(r.getWidth(),r.getHeight()),radius=diameter*.5f,cx=r.getCentreX(),cy=r.getCentreY();
 const auto accent=slider.findColour(juce::Slider::rotarySliderFillColourId);const float alpha=slider.isEnabled()?1.f:.36f;
 const float scale=g.getInternalContext().getPhysicalPixelScaleFactor();const int pixels=std::max(8,juce::roundToInt(diameter*.92f*scale));const float faceSize=float(pixels)/scale;
 g.setOpacity(alpha);g.drawImage(sizedKnobArtwork(pixels),juce::Rectangle<float>(float(juce::roundToInt((cx-faceSize*.5f)*scale))/scale,float(juce::roundToInt((cy-faceSize*.5f)*scale))/scale,faceSize,faceSize));g.setOpacity(1);
 for(int i=0;i<9;++i){const float a=start+(end-start)*float(i)/8;g.setColour(tide::ui::ink.withAlpha(.16f*alpha));g.drawLine(cx+std::sin(a)*(radius+1),cy-std::cos(a)*(radius+1),cx+std::sin(a)*(radius-1),cy-std::cos(a)*(radius-1),.65f);}
 juce::Path track,arc;track.addCentredArc(cx,cy,radius-3,radius-3,0,start,end,true);arc.addCentredArc(cx,cy,radius-3,radius-3,0,start,start+pos*(end-start),true);
 g.setColour(juce::Colour(0xff0c171c));g.strokePath(track,juce::PathStrokeType(2.5f));g.setColour(accent.withAlpha(alpha));g.strokePath(arc,juce::PathStrokeType(2.2f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
 const float a=start+pos*(end-start);g.setColour(tide::ui::ink.withAlpha(alpha));g.drawLine(cx+std::sin(a)*(radius*.47f),cy-std::cos(a)*(radius*.47f),cx+std::sin(a)*(radius*.72f),cy-std::cos(a)*(radius*.72f),1.8f);
 if(slider.isMouseOverOrDragging()||slider.hasKeyboardFocus(true)){g.setColour(accent.withAlpha(.32f));g.drawEllipse(cx-radius+1,cy-radius+1,diameter-2,diameter-2,1);}
}
TideEditor::TideEditor(TideProcessor& p):AudioProcessorEditor(&p),processor(p){
 for(int i=0;i<=100;++i){const auto n=std::size_t(i);const float t=float(i)*.06f;waveEnvelope[n]=std::sin(float(i)/100*tide::pi);waveSinA[n]=std::sin(t*1.8f);waveCosA[n]=std::cos(t*1.8f);waveSinB[n]=std::sin(t*3);waveCosB[n]=std::cos(t*3);}for(auto& curve:oceanCurves)curve.preallocateSpace(320);
 setLookAndFeel(&look);setResizable(true,true);setResizeLimits(900,600,1620,1695);
 auto show=[&](juce::Component& c){addAndMakeVisible(c);};
 for(int i=0;i<tide::numFloats;++i){auto& k=knobs[std::size_t(i)];auto& l=labels[std::size_t(i)];const auto& spec=tide::specs[i];
  show(k);show(l);k.setName(spec.name);k.setTooltip(i<31?::tips[i]:spec.name);k.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);k.setTextBoxStyle(juce::Slider::TextBoxBelow,false,104,23);
  k.setColour(juce::Slider::rotarySliderFillColourId,i<11||(i>=31&&i<=34)||i==39?silver:blue);k.setDoubleClickReturnValue(true,spec.initial);k.setPopupMenuEnabled(true);
  l.setText(juce::String(spec.name).toUpperCase(),juce::dontSendNotification);l.setJustificationType(juce::Justification::centred);l.setFont(tide::ui::font(11,juce::Font::bold));
  sliders[std::size_t(i)]=std::make_unique<SA>(p.state,spec.id,k);
  const juce::String unit(spec.unit);k.textFromValueFunction=[this,unit,i](double x){if(i==0&&processor.value("sync")>.5f){const float time=std::min(8000.f,60000.f/std::max(20.f,processor.tempo.load())*tide::beats[std::clamp(int(processor.value("rhythm")),0,8)]);return juce::String(time,1)+" ms";}return juce::String(unit=="%"?x*100:x,unit=="Hz"&&x>10?0:unit=="s"?2:1)+" "+unit;};
  k.valueFromTextFunction=[unit](const juce::String& text){return text.getDoubleValue()/(unit=="%"?100.:1.);};k.updateText();
 }
 auto setupCombo=[&](juce::ComboBox& c,const char* id,int index,juce::StringArray items){show(c);c.addItemList(items,1);combos[std::size_t(index)]=std::make_unique<CA>(p.state,id,c);};
 show(delayType);delayType.addItemList({"Digital","Tape","Analog / Bucket","Pattern / Multitap","Reverse","Pitch","Diffusion","Granular Pitch","Lo-Fi","Oil Can","Spectral","Magnetic Drum"},1);
 delayType.onChange=[this]{int n=delayType.getSelectedId()-1;if(n<0)return;processor.setValue("dRevision",1);processor.setValue("d13Model",float(n));processor.setValue("dVersion",1);processor.setValue("dModel",n>=6&&n<=8?float(n-5):0.f);if(n<6)processor.setValue("dType",float(n));refreshSelectors();};
 show(reverbType);reverbType.addItemList({"Room / Classic","Plate / Classic","Hall / Classic","Bloom / Classic","Chamber","Cloud Ensemble","Halo Shimmer","Glint","Studio Plate","Studio Hall","Studio Bloom","Nonlinear","Resonator"},1);
 reverbType.onChange=[this]{int n=reverbType.getSelectedId()-1;if(n<0)return;processor.setValue("rRevision",1);processor.setValue("r13Model",float(n));processor.setValue("rVersion",1);processor.setValue("rModel",n>=4&&n<=10?float(n-3):0.f);if(n<4)processor.setValue("rType",float(n));refreshSelectors();};
 setupCombo(delayRevision,"dRevision",0,{"Legacy processing","Tide 1.3 processing"});setupCombo(reverbRevision,"rRevision",1,{"Legacy processing","Tide 1.3 processing"});
 setupCombo(rhythm,"rhythm",2,{"1/16","1/8 triplet","1/8","1/8 dotted","1/4","1/4 dotted","1/2","1/2 dotted","1 bar"});
 setupCombo(stereo,"stereo",3,{"Stereo","Dual","Ping-pong"});
 setupCombo(routing,"routing",4,{"Delay > Reverb","Parallel"});
 setupCombo(interval,"interval",5,{"-12 st","-7 st","+7 st","+12 st","+19 st","+24 st"});
 setupCombo(reverbInterval,"rInterval",6,{"-12 st","-7 st","+7 st","+12 st","+19 st","+24 st"});
 setupCombo(pitchMode,"pitchMode",7,{"Climbing repeats","Fixed harmony"});pitchMode.setTooltip("Climbing: each feedback pass shifts again. Fixed harmony: feedback stays at its original pitch, then the return is shifted. The wet pitch window adds latency; dry audio stays immediate.");
 setupCombo(pattern,"dPattern",8,{"2 taps","4 taps","8 taps"});setupCombo(dualMode,"dDual",9,{"Linked ratio","Independent A / B"});setupCombo(bRhythm,"dBRhythm",10,{"B: 1/16","B: 1/8 triplet","B: 1/8","B: dotted eighth","B: quarter","B: dotted quarter","B: half","B: dotted half","B: bar"});setupCombo(manualTempo,"manualTempo",11,{"Host tempo","Manual tempo"});
 delayType.setTooltip("Choose the delay processing model; this keeps your other controls.");reverbType.setTooltip("Choose reverb topology. Existing tank tails continue.");
 rhythm.setTooltip("Tempo division, used when SYNC is enabled.");stereo.setTooltip("Stereo preserves channels. Dual uses B/A timing ratio. Ping-pong crosses feedback.");routing.setTooltip("Serial feeds the stereo delay into reverb. Parallel feeds each independently.");interval.setTooltip("Pitch delay interval.");reverbInterval.setTooltip("Shimmer interval, independent of Pitch delay.");
 juce::TextButton* toggles[]={&delayOn,&reverbOn,&sync,&send,&hold};const char* ids[]={"dOn","rOn","sync","send","freeze"};
 for(int i=0;i<5;++i){show(*toggles[i]);toggles[i]->setClickingTogglesState(true);buttons[std::size_t(i)]=std::make_unique<BA>(p.state,ids[i],*toggles[i]);}
 delayOn.setTooltip("Stop new delay input; let existing repeats decay.");reverbOn.setTooltip("Stop new reverb input, including delay repeats; let the existing tail decay.");send.setTooltip("Use on a DAW return/aux: removes the dry signal. This setting survives factory preset changes.");hold.setTooltip("Hold the reverb tank and stop new excitation. Modulation can slowly soften the held sound.");
 for(auto* c:{&presetDelay,&presetReverb,&presetCombined})show(*c);
 delayType.setName("delayModel");reverbType.setName("reverbModel");presetDelay.setName("delayPreset");presetReverb.setName("reverbPreset");presetCombined.setName("combinedPreset");
 presetDelay.onChange=[this]{const int id=presetDelay.getSelectedId();if(id>0&&id<=24){processor.applyDelayPreset((id-1)/2,(id-1)%2!=0);refreshSelectors();}};
 presetReverb.onChange=[this]{const int id=presetReverb.getSelectedId();if(id>0&&id<=26){processor.applyReverbPreset((id-1)/2,(id-1)%2!=0);refreshSelectors();}};
 presetCombined.onChange=[this]{const int id=presetCombined.getSelectedId();if(id>0&&id<=tide::numPresets){processor.setCurrentProgram(id-1);refreshSelectors();}};
 for(auto* b:{&editDelay,&editReverb,&editRoute,&clear,&saveDelay,&saveReverb,&saveCombined,&lockDelay,&lockReverb,&lockMix,&motion,&undo,&undoLoad,&tap,&lockRoute,&help})show(*b);
 editDelay.onClick=[this]{if(processor.value("dRevision")>.5f)showModern(0);else showEdit(editing==1?0:1);};editReverb.onClick=[this]{if(processor.value("rRevision")>.5f)showModern(2);else showEdit(editing==2?0:2);};editRoute.onClick=[this]{if(processor.value("dRevision")>.5f||processor.value("rRevision")>.5f)showModern(4);else showEdit(editing==3?0:3);};
 clear.onClick=[this]{processor.clearRequested.store(true);};clear.setTooltip("Fade out, empty stored echoes and tails, then fade in. Does not change settings.");
 saveDelay.onClick=[this]{showBrowser(1);};saveReverb.onClick=[this]{showBrowser(2);};saveCombined.onClick=[this]{showBrowser(0);};saveCombined.setButtonText("BROWSE");saveDelay.setTooltip("Search delay presets, favorites and user sounds.");saveReverb.setTooltip("Search reverb presets, favorites and user sounds.");
 undoLoad.onClick=[this]{processor.undoLastLoad();refreshSelectors();};lockRoute.setClickingTogglesState(true);lockRoute.onClick=[this]{processor.lockRouting.store(lockRoute.getToggleState());};tap.onClick=[this]{const double now=juce::Time::getMillisecondCounterHiRes();if(lastTap>0&&now-lastTap>=250&&now-lastTap<=2000){processor.setValue("bpm",float(60000/(now-lastTap)));processor.setValue("manualTempo",1);processor.setValue("sync",1);}lastTap=now;};
 for(auto* b:{&lockDelay,&lockReverb,&lockMix,&motion})b->setClickingTogglesState(true);
 lockDelay.onClick=[this]{processor.lockDelay.store(lockDelay.getToggleState());};lockReverb.onClick=[this]{processor.lockReverb.store(lockReverb.getToggleState());};lockMix.onClick=[this]{processor.lockMix.store(lockMix.getToggleState());};
 lockDelay.setTooltip("Keep delay parameters when loading combined presets.");lockReverb.setTooltip("Keep reverb parameters when loading combined presets.");lockMix.setTooltip("Keep overall Mix and both Amount controls while browsing presets.");
 motion.onClick=[this]{const int mode=(processor.visualMode.load()+1)%3;processor.visualMode.store(mode);processor.reducedMotion.store(mode==0);timerCallback();};motion.setTooltip("Cycle STATIC / ACTIVITY / MOTION. Activity animates only while sound is present. Visuals never change the audio.");
 undo.setTooltip("First click stores A. Later clicks swap the current sound with the stored version.");
 undo.onClick=[this]{juce::MemoryBlock current;processor.getStateInformation(current);if(alternateReady)processor.setStateInformation(alternate.getData(),int(alternate.getSize()));alternate=current;alternateReady=true;undo.setButtonText("SWAP A / B");refreshSelectors();};
 help.onClick=[this]{juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::InfoIcon,"Wolf Tide / Quick guide","Grey = stereo delay. Blue = stereo reverb. Each side has its own presets and on/off button.\n\nBottom presets recall a complete sound. LOCK protects a side; LOCK MIX protects all three blend controls. SEND removes dry audio for return tracks.\n\nEDIT opens machine controls, including independent slots A/B. ROUTING opens Series / Parallel / Frequency split. Legacy processing preserves old sessions. Switching a side to Tide 1.3 explicitly upgrades its sound. Double-click resets a knob; click its value to type precisely.\n\nHOLD sustains the reverb; CLEAR TAILS deliberately empties both effects. DAW Stop can preserve tails only while the host keeps processing.\n\nFactory sounds are designed starting points, not measured copies of other products. Version "+juce::String(JucePlugin_VersionString)+".","OK",this);};
 show(buildLabel);buildLabel.setName("releaseBuild");buildLabel.setTooltip("Compiled release and source build ID. Legacy / Modern refers to sound generation, not this release version.");buildLabel.setText(p.releaseLabel(),juce::dontSendNotification);buildLabel.setFont(tide::ui::font(11,juce::Font::bold));
 show(status);status.setName("presetStatus");show(tempoLabel);status.setFont(juce::FontOptions(11));tempoLabel.setFont(tide::ui::font(11,juce::Font::bold));tempoLabel.setJustificationType(juce::Justification::centredRight);
 setSize(1080,720);resized();timerCallback();if(isShowing())startTimerHz(12);
}
TideEditor::~TideEditor(){processor.editorVisible.store(false);stopTimer();setLookAndFeel(nullptr);}
void TideEditor::visibilityChanged(){const bool showing=isShowing();processor.editorVisible.store(showing);if(showing){timerCallback();startTimerHz(12);}else stopTimer();}
void TideEditor::showEdit(int which){editing=which;const int h=which?1130:720;setSize(getWidth(),juce::roundToInt(float(getWidth())*float(h)/1080));resized();repaint();}
void TideEditor::fileMenu(int scope){juce::PopupMenu menu;menu.setLookAndFeel(&look);menu.addItem(1,"Save "+juce::String(scope==0?"combined":scope==1?"delay":"reverb")+" preset...");menu.addItem(2,"Load preset...");
 menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(scope==0?&saveCombined:scope==1?&saveDelay:&saveReverb),[safe=juce::Component::SafePointer<TideEditor>(this),scope](int id){if(safe&&id>0)safe->chooseFile(scope,id==1);});}
void TideEditor::chooseFile(int scope,bool save){
 const auto folder=juce::File::getSpecialLocation(juce::File::userDocumentsDirectory).getChildFile("Wolf Audio Presets");folder.createDirectory();
 chooser=std::make_unique<juce::FileChooser>(save?"Save Wolf Tide preset":"Load Wolf Tide preset",folder.getChildFile(scope==0?"My Tide.wolftide":scope==1?"My Delay.wolftide":"My Reverb.wolftide"),"*.wolftide");
 chooser->launchAsync((save?juce::FileBrowserComponent::saveMode|juce::FileBrowserComponent::warnAboutOverwriting:juce::FileBrowserComponent::openMode)|juce::FileBrowserComponent::canSelectFiles,
  [safe=juce::Component::SafePointer<TideEditor>(this),scope,save](const juce::FileChooser& fc){if(!safe)return;auto file=fc.getResult();if(file==juce::File{})return;
   auto result=save?safe->processor.savePreset(file.withFileExtension("wolftide"),scope):safe->processor.loadPreset(file,scope);
   if(result.wasOk())safe->refreshSelectors();
   if(result.failed())juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon,"Preset",result.getErrorMessage(),"OK",safe.getComponent());});
}
void TideEditor::refreshSelectors(){
 knobs[0].updateText(); // Sync/rhythm changes can leave the stored millisecond parameter unchanged.
 delayType.setSelectedId((processor.value("dRevision")>.5f?int(processor.value("d13Model")):tide::delayModel(int(processor.value("dType")),int(processor.value("dModel"))))+1,juce::dontSendNotification);
 reverbType.setSelectedId((processor.value("rRevision")>.5f?int(processor.value("r13Model")):tide::reverbModel(int(processor.value("rType")),int(processor.value("rModel"))))+1,juce::dontSendNotification);
 juce::ComboBox* boxes[]={&presetCombined,&presetDelay,&presetReverb};
 for(int scope=0;scope<3;++scope){auto& box=*boxes[scope];const auto display=processor.presetDisplay(scope);
  const auto title=(display.modified?"Modified: ":display.factory?"Factory: ":display.user?"User: ":"")+display.name;
  const int id=display.factory&&!display.modified&&display.index>=0?display.index+1:10000;
  if(box.getText()==title&&box.getSelectedId()==id)continue;
  box.clear(juce::dontSendNotification);
  if(scope==0){for(int pass=0;pass<2;++pass)for(int i=pass==0?16:0;i<(pass==0?tide::numPresets:16);++i)box.addItem("Factory: "+processor.getProgramName(i),i+1);}
  else for(int i=0;i<(scope==1?24:26);++i)box.addItem("Factory: "+TideProcessor::sidePresetName(scope,i),i+1);
  if(id==10000){box.addItem(title,10000);box.setItemEnabled(10000,false);}
  box.setSelectedId(id,juce::dontSendNotification);
 }
 const auto currentPreset=processor.presetDisplay(0);
 const auto presetStatus=currentPreset.modified?"MODIFIED / PARTIAL SOUND":currentPreset.factory?"FACTORY PRESET":currentPreset.user?"USER PRESET":"CUSTOM SOUND";
 status.setText(oLevel>=1?"OUTPUT ABOVE 0 dBFS / LOWER OUTPUT IN ROUTING":"STEREO IN / STEREO OUT     |     "+juce::String(presetStatus),juce::dontSendNotification);
 status.setColour(juce::Label::textColourId,oLevel>=1?juce::Colour(0xffffbc7a):muted);
}
void TideEditor::timerCallback(){
 const float oldD=dLevel,oldR=rLevel,oldO=oLevel;
 dLevel=std::max(processor.delayMeter.exchange(0),dLevel*.80f);rLevel=std::max(processor.reverbMeter.exchange(0),rLevel*.86f);oLevel=std::max(processor.outputMeter.exchange(0),oLevel*.84f);
 const int visualMode=processor.visualMode.load();const bool animate=visualMode==2||(visualMode==1&&std::max(dLevel,rLevel)>.0001f);
 if(animate)phase+=.025f;
 if(isShowing()&&getTimerInterval()!=(animate?1000/24:1000/12))startTimerHz(animate?24:12);
 delayOn.setButtonText(processor.value("dOn")>.5f?(processor.delayRunning.load()?"ON":"READY"):dLevel>.0001f?"TAIL":"OFF");reverbOn.setButtonText(processor.value("freeze")>.5f?"HOLD":processor.value("rOn")>.5f?(processor.reverbRunning.load()?"ON":"READY"):rLevel>.0001f?"TAIL":"OFF");
 lockDelay.setToggleState(processor.lockDelay.load(),juce::dontSendNotification);lockReverb.setToggleState(processor.lockReverb.load(),juce::dontSendNotification);lockMix.setToggleState(processor.lockMix.load(),juce::dontSendNotification);
 motion.setButtonText(visualMode==0?"STATIC":visualMode==1?"ACTIVITY":"MOTION");motion.setToggleState(visualMode!=0,juce::dontSendNotification);
 const bool synced=processor.value("sync")>.5f;knobs[0].setEnabled(!synced);if(synced&&tempoLabel.getText()!=juce::String(processor.tempo.load(),1)+" BPM / "+(processor.tempoFromHost.load()?"HOST":"LOCAL"))knobs[0].updateText();rhythm.setEnabled(synced);knobs[21].setEnabled(processor.value("routing")<.5f);
 refreshSelectors();
 lockRoute.setToggleState(processor.lockRouting.load(),juce::dontSendNotification);undoLoad.setEnabled(processor.undoReady);
 const int mode=tide::delayModel(int(processor.value("dType")),int(processor.value("dModel")));knobs[6].setEnabled(mode==1||mode==2);
 pitchMode.setEnabled(mode==5);interval.setEnabled(mode==5||mode==7);
 const int verb=tide::reverbModel(int(processor.value("rType")),int(processor.value("rModel")));
 knobs[15].setEnabled(verb>=2);knobs[35].setEnabled(verb>=4);knobs[36].setEnabled(verb>=4);knobs[37].setEnabled(verb==7||verb==10);knobs[38].setEnabled(verb==6||verb==7);
 knobs[4].setEnabled(mode!=4);knobs[9].setEnabled(mode!=4);pattern.setEnabled(mode==3&&processor.value("dVersion")>.5f);
 knobs[4].setTooltip(mode==7&&processor.value("dRevision")<.5f?"Grain overlap and source-position spread.":"Delay modulation depth.");knobs[9].setTooltip(mode==7&&processor.value("dRevision")<.5f?"Grain duration: higher rates shorten the grains.":"Rate of delay modulation.");
 const bool independent=processor.value("dDual")>.5f;
 knobs[10].setEnabled(!independent&&processor.value("stereo")>.5f&&processor.value("stereo")<1.5f);
 knobs[31].setEnabled(independent&&!synced);bRhythm.setEnabled(independent&&synced);knobs[32].setEnabled(independent);knobs[33].setEnabled(independent);knobs[39].setEnabled(mode==6);knobs[23].setEnabled(processor.value("send")<.5f);
 tempoLabel.setText(juce::String(processor.tempo.load(),1)+" BPM / "+(processor.tempoFromHost.load()?"HOST":"LOCAL"),juce::dontSendNotification);

 constexpr const char* displayParameters[]={"dFeedback","dMotion","rMotion","shimmer","feed","dOn","rOn","dTime","sync","rhythm","stereo","ratio","dDual","dBTime","dBRhythm","d13Model","dRevision","dPattern","dTap1Time","dTap1Level","dTap2Time","dTap2Level","dTap3Time","dTap3Level","dTap4Time","dTap4Level"};
 std::array<float,32> next{};for(std::size_t i=0;i<std::size(displayParameters);++i)next[i]=processor.value(displayParameters[i]);next[30]=processor.tempo.load();next[31]=float(visualMode);
 const bool changed=next!=visualValues;visualValues=next;
 const float sx=float(getWidth())/1080,sy=float(getHeight())/(editing?1130.f:720.f);auto region=[&](int x,int y,int w,int h){repaint(juce::Rectangle<int>(int(float(x)*sx),int(float(y)*sy),int(float(w)*sx)+2,int(float(h)*sy)+2));};
 if(changed||(visualMode!=0&&((visualMode==2||dLevel>.0001f)||std::abs(oldD-dLevel)>.0001f)))region(42,235,474,145);
 if(changed||(visualMode!=0&&((visualMode==2||rLevel>.0001f)||std::abs(oldR-rLevel)>.0001f)))region(565,235,474,145);
 if(changed)region(530,290,20,23);
 if(std::abs(oldO-oLevel)>.001f)region(960,698,80,8);
}
void TideEditor::drawEchoes(juce::Graphics& g){
 const float feedback=processor.value("dFeedback"),activity=processor.visualMode.load()==0?0.f:std::min(1.f,dLevel*5);
 const float bpm=std::max(20.f,processor.tempo.load());const bool syncTime=processor.value("sync")>.5f;
 const float time=syncTime?std::min(8000.f,60000.f/bpm*tide::beats[std::clamp(int(processor.value("rhythm")),0,8)]):processor.value("dTime");
 const float window=std::max(syncTime?240000.f/bpm:2000.f,time*2),motionDepth=processor.value("dMotion");
 const int model=processor.value("dRevision")>.5f?int(processor.value("d13Model")):tide::delayModel(int(processor.value("dType")),int(processor.value("dModel")));
 const bool modern=processor.value("dRevision")>.5f;
 g.saveState();g.reduceClipRegion(juce::Rectangle<int>(43,235,472,143));
 for(int line=0;line<5;++line){const float x=79+float(line)*102;g.setColour(silver.withAlpha(.045f));g.drawVerticalLine(int(x),267,365);}
 for(int row=0;row<2;++row){const float cy=284.f+float(row)*57;g.setColour(silver.withAlpha(.13f));g.drawHorizontalLine(int(cy),67,502);
  label(g,row?"R":"L",55,cy-9,15,18,10,muted);
  const bool dual=int(processor.value("stereo"))==1;float rowTime=time;
  if(row&&dual)rowTime=processor.value("dDual")>.5f?(syncTime?60000.f/bpm*tide::beats[std::clamp(int(processor.value("dBRhythm")),0,8)]:processor.value("dBTime")):time*processor.value("ratio");
  const int count=model==3?(modern?4:2<<std::clamp(int(processor.value("dPattern")),0,2)):8;float strength=1;
  for(int i=0;i<count;++i){float tapTime=rowTime*float(i+1),gain=strength;
   if(model==3&&modern){const auto prefix="dTap"+juce::String(i+1);tapTime=rowTime*processor.value(prefix+"Time");gain=processor.value(prefix+"Level");}
   const float x=78+tapTime/window*414;if(x>502)continue;
   if(int(processor.value("stereo"))==2&&((i+row)&1))gain*=.28f;
   const float y=cy+std::sin(phase+float(i)*.75f+float(row)*.4f)*motionDepth*(2+activity*2),radius=2.4f+gain*(1.6f+activity*2.5f);
   g.setColour(silver.withAlpha(.025f+activity*.07f));g.fillEllipse(x-radius*2.5f,y-radius*2.5f,radius*5,radius*5);
   g.setColour(silver.withAlpha(.08f+gain*.27f));g.drawLine(x-10-activity*8,y,x-4,y,1);
   g.setColour(silver.withAlpha(.2f+gain*.7f));g.fillEllipse(x-radius,y-radius,radius*2,radius*2);strength*=std::max(.18f,feedback*.9f);
  }
 }
 label(g,juce::String(window*.001f,1)+" s",444,242,58,18,10,muted);g.restoreState();
}
void TideEditor::drawOcean(juce::Graphics& g){
 g.saveState();g.reduceClipRegion(juce::Rectangle<int>(565,235,472,143));
 const float activity=processor.visualMode.load()==0?0.f:std::min(1.f,rLevel*6),movement=processor.value("rMotion"),shimmer=processor.value("shimmer");
 if(phase!=curvePhase||activity!=curveActivity||movement!=curveMovement){
  curvePhase=phase;curveActivity=activity;curveMovement=movement;
  const float s2=std::sin(-phase*.4f),c2=std::cos(-phase*.4f);
  for(int j=0;j<9;++j){auto& wave=oceanCurves[std::size_t(j)];wave.clear();const float offset=phase*(.4f+movement)+float(j)*.36f,s1=std::sin(offset),c1=std::cos(offset);
   for(int i=0;i<=100;++i){const auto n=std::size_t(i);const float x=580+float(i)*4.42f;
    const float y=276+float(j)*10+waveEnvelope[n]*((waveSinA[n]*c1+waveCosA[n]*s1)*10+(waveSinB[n]*c2+waveCosB[n]*s2)*5)*(.45f+activity*.7f);
    if(i==0)wave.startNewSubPath(x,y);else wave.lineTo(x,y);}
  }
 }
 for(int j=0;j<9;++j){const auto& wave=oceanCurves[std::size_t(j)];const float depth=float(j)/8;
  if(j==3||j==6){g.setColour(blue.withAlpha(.025f+activity*.025f));g.strokePath(wave,juce::PathStrokeType(5));}
  g.setColour(blue.withAlpha(.13f+depth*.3f+activity*.1f));g.strokePath(wave,juce::PathStrokeType(.7f+depth*.65f));}
 if(shimmer>.01f){g.setColour(ink.withAlpha(shimmer*(.35f+activity*.35f)));for(int i=0;i<9;++i){const float x=605+float(i)*45,y=267+std::sin(float(i)*2.3f+phase*.3f)*8;g.fillEllipse(x,y,1.7f,1.7f);}}
 g.restoreState();
}
void TideEditor::buildBackdrop(){
 const int h=editing?1130:720;const float scale=std::clamp(2.f*float(getWidth())/1080,1.f,3.f);const int imageW=juce::roundToInt(1080*scale),imageH=juce::roundToInt(float(h)*scale);if(backdrop.getWidth()==imageW&&backdrop.getHeight()==imageH&&backdropPage==editing)return;backdropPage=editing;backdrop=juce::Image(juce::Image::RGB,imageW,imageH,true);juce::Graphics g(backdrop);g.addTransform(juce::AffineTransform::scale(scale));g.fillAll(bg);
 // The approved Higgsfield surfaces are composed only when size/page changes.
 using namespace tide::ui;
 paintSkin(g,{0,0,1080,float(h)},editing?Skin::expanded:Skin::chassis);
 paintSkin(g,{24,105,510,465},Skin::delay);paintSkin(g,{546,105,510,465},Skin::reverb);
 paintSkin(g,{42,233,474,145},Skin::glass);paintSkin(g,{564,233,474,145},Skin::glass);
 label(g,"STEREO ECHO FIELD",57,244,330,16,10,muted);label(g,"ATMOSPHERIC FIELD",580,244,400,16,10,blue);

 label(g,"W O L F   A U D I O",28,17,220,20,11,muted);g.setColour(ink);g.setFont(tide::ui::font(49,juce::Font::bold|juce::Font::italic));g.drawText("Tide",25,34,160,54,juce::Justification::centredLeft);
 label(g,"ECHO / ATMOSPHERE",176,50,350,20,12,muted);
 g.setColour(juce::Colour(0xffa9aaa0));g.drawEllipse(570,38,35,35,1.1f);g.drawLine(588,32,588,79,.7f);g.drawLine(566,56,609,56,.7f);
 label(g,"01   DELAY",43,121,220,24,18,silver);label(g,"02   REVERB",565,121,230,24,18,blue);
 label(g,"CHARACTER",45,189,90,18,10,muted);label(g,"CHARACTER",567,189,90,18,10,muted);
 paintSkin(g,{24,585,1032,105},Skin::master);
 label(g,"COMBINED PRESET",43,596,180,20,10,muted);label(g,"MIX",846,595,70,20,10,muted);label(g,"BLOOM",953,595,80,20,10,blue);
 if(editing){paintSkin(g,{24,704,1032,397},Skin::body);label(g,editing==1?"DELAY / DEEP EDIT":editing==2?"REVERB / DEEP EDIT":"ROUTING / MIX & OUTPUT",43,713,500,24,12,editing==1?silver:blue);}
}
void TideEditor::paint(juce::Graphics& g){
 const float baseH=editing?1130.f:720.f;g.addTransform(juce::AffineTransform::scale(float(getWidth())/1080,float(getHeight())/baseH));
 if(backdrop.isValid())g.drawImage(backdrop,0,0,1080,int(baseH),0,0,backdrop.getWidth(),backdrop.getHeight());
 if(g.clipRegionIntersects(juce::Rectangle<int>(42,235,474,145)))drawEchoes(g);
 if(g.clipRegionIntersects(juce::Rectangle<int>(565,235,474,145)))drawOcean(g);
 g.setColour(juce::Colour(0xffa9aaa0));g.drawLine(531,301,548,301,2);g.setColour(blue.withAlpha(.3f+processor.value("feed")*.7f));g.fillEllipse(536,297,8,8);
 g.setColour(juce::Colour(0xff354758));g.fillRoundedRectangle(961,699,77,4,2);g.setColour(oLevel>=1?juce::Colour(0xffffbc7a):blue);g.fillRoundedRectangle(961,699,77*std::min(1.f,oLevel),4,2);
}
void TideEditor::resized(){
 buildBackdrop();
 const float sx=float(getWidth())/1080,sy=float(getHeight())/(editing?1130.f:720.f);
 auto rect=[&](int x,int y,int w,int h){return juce::Rectangle<int>(juce::roundToInt(float(x)*sx),juce::roundToInt(float(y)*sy),juce::roundToInt(float(w)*sx),juce::roundToInt(float(h)*sy));};
 for(int i=0;i<tide::numFloats;++i){knobs[std::size_t(i)].setVisible(false);labels[std::size_t(i)].setVisible(false);}
 auto knob=[&](int index,int x,int y,int w,int h,bool linear=false){auto& k=knobs[std::size_t(index)];auto& l=labels[std::size_t(index)];k.setVisible(true);l.setVisible(true);k.setSliderStyle(linear?juce::Slider::LinearHorizontal:juce::Slider::RotaryHorizontalVerticalDrag);l.setBounds(rect(x,y,w,19));k.setBounds(rect(x,y+20,w,h-20));};
 for(int i=0;i<8;++i){const bool right=i>=4;const int x=(right?565:43)+(i%4)*119;knob(mainIndices[i],x,390,112,140);}
 delayOn.setBounds(rect(438,118,76,28));reverbOn.setBounds(rect(960,118,76,28));
 presetDelay.setBounds(rect(43,157,365,28));saveDelay.setBounds(rect(414,157,42,28));lockDelay.setBounds(rect(462,157,52,28));
 presetReverb.setBounds(rect(565,157,365,28));saveReverb.setBounds(rect(936,157,42,28));lockReverb.setBounds(rect(984,157,52,28));
 delayType.setBounds(rect(137,188,180,29));reverbType.setBounds(rect(659,188,180,29));
 sync.setBounds(rect(324,188,64,29));rhythm.setBounds(rect(394,188,120,29));hold.setBounds(rect(960,188,76,29));
 editDelay.setBounds(rect(43,531,155,26));editReverb.setBounds(rect(565,531,155,26));delayRevision.setBounds(rect(209,531,305,26));reverbRevision.setBounds(rect(731,531,305,26));
 presetCombined.setBounds(rect(43,620,376,34));saveCombined.setBounds(rect(428,620,108,34));
 editRoute.setBounds(rect(546,620,94,34));send.setBounds(rect(649,620,66,34));lockMix.setBounds(rect(725,620,92,34));
 knob(23,828,613,105,73,true);knob(25,938,613,101,73,true);labels[23].setVisible(false);labels[25].setVisible(false);
 clear.setBounds(rect(678,42,115,29));undo.setBounds(rect(804,42,105,29));undoLoad.setBounds(rect(678,79,115,24));motion.setBounds(rect(918,42,87,29));help.setBounds(rect(1014,42,40,29));
 buildLabel.setBounds(rect(176,73,390,18));
 status.setBounds(rect(31,696,780,20));tempoLabel.setBounds(rect(771,79,274,19));
 stereo.setVisible(editing==1);interval.setVisible(editing==1);pitchMode.setVisible(editing==1);reverbInterval.setVisible(editing==2);routing.setVisible(editing==3);pattern.setVisible(editing==1);dualMode.setVisible(editing==1);bRhythm.setVisible(editing==1);manualTempo.setVisible(editing==3);tap.setVisible(editing==3);lockRoute.setVisible(editing==3);
 if(editing==1){pattern.setBounds(rect(43,907,150,27));dualMode.setBounds(rect(209,907,190,27));bRhythm.setBounds(rect(416,907,190,27));constexpr int extended[]={31,32,33,34,39};for(int i=0;i<5;++i)knob(extended[i],43+i*196,951,170,132);pitchMode.setBounds(rect(500,713,240,27));stereo.setBounds(rect(755,713,135,27));interval.setBounds(rect(900,713,135,27));constexpr int ids[]={4,5,6,7,8,9,10,27,28};for(int i=0;i<9;++i)knob(ids[i],43+i*110,752,101,132);}
 if(editing==2){constexpr int extended[]={35,36,37,38};for(int i=0;i<4;++i)knob(extended[i],43+i*245,951,180,132);reverbInterval.setBounds(rect(900,713,135,27));constexpr int ids[]={15,16,17,18,19,20,29,30};for(int i=0;i<8;++i)knob(ids[i],43+i*124,752,115,132);}
 if(editing==3){manualTempo.setBounds(rect(43,914,190,30));tap.setBounds(rect(245,914,80,30));lockRoute.setBounds(rect(340,914,120,30));routing.setBounds(rect(774,713,261,27));constexpr int ids[]={21,22,24,26};for(int i=0;i<4;++i)knob(ids[i],80+i*245,752,180,132);}
}

void TideEditor::showModern(int tab){auto panel=std::make_unique<TideModernPanel>(processor,tab);auto& popup=juce::CallOutBox::launchAsynchronously(std::move(panel),(tab<2?editDelay:tab<4?editReverb:editRoute).getScreenBounds(),nullptr);popup.setLookAndFeel(&tide::ui::sharedLook());}

void TideEditor::showBrowser(int scope){auto safe=juce::Component::SafePointer<TideEditor>(this);auto browser=std::make_unique<TidePresetBrowser>(processor,scope,[safe,scope]{if(safe)safe->fileMenu(scope);},[safe]{if(safe)safe->refreshSelectors();});auto& popup=juce::CallOutBox::launchAsynchronously(std::move(browser),(scope==0?saveCombined:scope==1?saveDelay:saveReverb).getScreenBounds(),nullptr);popup.setLookAndFeel(&tide::ui::sharedLook());}
