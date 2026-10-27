// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include "PluginProcessor.h"
// Message-thread-only browser shared by all three preset scopes.
class TidePresetBrowser final:public juce::Component,private juce::ListBoxModel {
 struct Entry{juce::String key,name;juce::File file;int index=0;bool factory=true;};
 juce::LookAndFeel_V4 theme;TideProcessor& processor;int scope;std::vector<Entry> all,shown;juce::StringArray favorites,recent;
 juce::TextEditor search,name;juce::ComboBox collection,tags;juce::ListBox list{"Presets",this};
 juce::TextButton load{"LOAD"},star{"FAVORITE"},save{"SAVE USER"},import{"IMPORT / EXPORT"},previous{"<"},next{">"};juce::Label heading,message;
 std::function<void()> importAction;
 juce::File folder()const{return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory).getChildFile("Wolf Audio/Wolf Tide/Presets");}
 juce::File preferences()const{return folder().getParentDirectory().getChildFile("Browser.xml");}
 void readPreferences(){if(auto xml=juce::XmlDocument::parse(preferences())){favorites=juce::StringArray::fromTokens(xml->getStringAttribute("favorites"),"\n","");recent=juce::StringArray::fromTokens(xml->getStringAttribute("recent"),"\n","");}}
 void persist(){preferences().getParentDirectory().createDirectory();juce::XmlElement xml("TideBrowser");xml.setAttribute("favorites",favorites.joinIntoString("\n"));xml.setAttribute("recent",recent.joinIntoString("\n"));preferences().replaceWithText(xml.toString());}
 void inventory(){all.clear();if(scope==0){for(int pass=0;pass<2;++pass)for(int i=pass==0?16:0;i<(pass==0?tide::numPresets:16);++i)all.push_back({"F0:"+juce::String(i),processor.getProgramName(i)+(i<16?" [Legacy]":""),{},i,true});}
  else{const int count=scope==1?9:11;for(int i=0;i<count;++i)for(int variant=0;variant<2;++variant){const char* model=scope==1?(i<6?tide::delayNames[i]:tide::newDelayNames[i-6]):(i<4?tide::reverbNames[i]:tide::newReverbNames[i-4]);all.push_back({"F"+juce::String(scope)+":"+juce::String(i*2+variant),juce::String(model)+(variant?" / Wide":" / Focused"),{},i*2+variant,true});}}
  if(folder().isDirectory())for(const auto& file:folder().findChildFiles(juce::File::findFiles,false,"*.wolftide"))if(file.getSize()<=1024*1024)if(auto xml=juce::XmlDocument::parse(file))if(xml->hasTagName("WolfTidePreset")&&xml->getIntAttribute("scope",-1)==scope)all.push_back({"U:"+file.getFullPathName(),xml->getStringAttribute("name",file.getFileNameWithoutExtension()),file,0,false});
 }
 void filter(){shown.clear();for(const auto& e:all){const int kind=collection.getSelectedId();if(kind==1&&!e.factory)continue;if(kind==2&&e.factory)continue;if(kind==3&&!favorites.contains(e.key))continue;if(kind==4&&!recent.contains(e.key))continue;
   if(!e.name.containsIgnoreCase(search.getText()))continue;const auto tag=tags.getText();if(tags.getSelectedId()>1&&!e.name.containsIgnoreCase(tag))continue;shown.push_back(e);}
  if(collection.getSelectedId()==4)std::stable_sort(shown.begin(),shown.end(),[&](const Entry& a,const Entry& b){return recent.indexOf(a.key)<recent.indexOf(b.key);});list.updateContent();list.selectRow(shown.empty()?-1:0);list.repaint();}
 void apply(){const int row=list.getSelectedRow();if(row<0||row>=int(shown.size()))return;const auto e=shown[std::size_t(row)];
  if(e.factory){if(scope==0)processor.setCurrentProgram(e.index);else if(scope==1)processor.applyDelayPreset(e.index/2,e.index%2!=0);else processor.applyReverbPreset(e.index/2,e.index%2!=0);message.setText("Loaded "+e.name,juce::dontSendNotification);}
  else{auto result=processor.loadPreset(e.file,scope);if(result.failed()){message.setText(result.getErrorMessage(),juce::dontSendNotification);return;}message.setText("Loaded "+e.name,juce::dontSendNotification);}
  recent.removeString(e.key);recent.insert(0,e.key);while(recent.size()>20)recent.remove(recent.size()-1);persist();}
 int getNumRows()override{return int(shown.size());}
 void paintListBoxItem(int row,juce::Graphics& g,int w,int h,bool selected)override{if(row<0||row>=int(shown.size()))return;const auto& e=shown[std::size_t(row)];g.fillAll(juce::Colour(selected?0xff285566:0xff172a32));g.setColour(juce::Colour(0xffeee6d7));g.setFont(juce::FontOptions(15));g.drawText((favorites.contains(e.key)?"* ":"")+e.name,12,0,w-24,h,juce::Justification::centredLeft,true);}
 void listBoxItemDoubleClicked(int,const juce::MouseEvent&)override{apply();}
 void returnKeyPressed(int)override{apply();}
public:
 TidePresetBrowser(TideProcessor& p,int section,std::function<void()> action):processor(p),scope(section),importAction(std::move(action)){
  for(juce::Component* component:std::initializer_list<juce::Component*>{&search,&name,&collection,&tags,&list,&load,&star,&save,&import,&previous,&next,&heading,&message})addAndMakeVisible(component);
  theme.setColour(juce::ComboBox::backgroundColourId,juce::Colour(0xff1d3742));theme.setColour(juce::ComboBox::textColourId,juce::Colour(0xffeee6d7));theme.setColour(juce::PopupMenu::backgroundColourId,juce::Colour(0xff1d3742));theme.setColour(juce::PopupMenu::textColourId,juce::Colour(0xffeee6d7));setLookAndFeel(&theme);
  list.setColour(juce::ListBox::backgroundColourId,juce::Colour(0xff172a32));message.setColour(juce::Label::textColourId,juce::Colour(0xffc2d4d8));
  for(auto* field:{&search,&name}){field->setColour(juce::TextEditor::backgroundColourId,juce::Colour(0xff0b1b22));field->setColour(juce::TextEditor::textColourId,juce::Colour(0xffeee6d7));}
  heading.setText(section==0?"TIDE / COMBINED PRESETS":section==1?"TIDE / DELAY PRESETS":"TIDE / REVERB PRESETS",juce::dontSendNotification);heading.setFont(juce::FontOptions(17));heading.setColour(juce::Label::textColourId,juce::Colour(0xffeee6d7));
  search.setTextToShowWhenEmpty("Search sounds...",juce::Colour(0xffaabac0));name.setTextToShowWhenEmpty("Name your user preset",juce::Colour(0xffaabac0));
  collection.addItemList({"Factory","User","Favorites","Recent"},1);collection.setSelectedId(1);tags.addItemList({"All instruments","Guitar","Vocal","Keys","Ambient","Utility"},1);tags.setSelectedId(1);tags.setEnabled(scope==0);
  search.onTextChange=[this]{filter();};collection.onChange=[this]{filter();};tags.onChange=[this]{filter();};load.onClick=[this]{apply();};
  previous.onClick=[this]{list.selectRow(std::max(0,list.getSelectedRow()-1));apply();};next.onClick=[this]{list.selectRow(std::min(int(shown.size())-1,list.getSelectedRow()+1));apply();};
  star.onClick=[this]{const int row=list.getSelectedRow();if(row<0||row>=int(shown.size()))return;auto key=shown[std::size_t(row)].key;if(favorites.contains(key))favorites.removeString(key);else favorites.add(key);persist();list.repaint();};
  save.onClick=[this]{auto title=juce::File::createLegalFileName(name.getText().trim());if(title.isEmpty()){message.setText("Enter a name before saving.",juce::dontSendNotification);return;}folder().createDirectory();auto file=folder().getNonexistentChildFile(title,".wolftide");auto result=processor.savePreset(file,scope);message.setText(result.wasOk()?"Saved "+file.getFileNameWithoutExtension():result.getErrorMessage(),juce::dontSendNotification);if(result.wasOk()){inventory();collection.setSelectedId(2);filter();}};
  import.onClick=[this]{importAction();};list.setRowHeight(30);readPreferences();inventory();filter();setSize(680,440);
 }
 ~TidePresetBrowser()override{setLookAndFeel(nullptr);}
 void paint(juce::Graphics& g)override{g.fillAll(juce::Colour(0xff10232c));g.setColour(juce::Colour(0xff86bbc5));g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(.5f),8,1);}
 void resized()override{heading.setBounds(18,12,640,26);search.setBounds(18,48,642,30);collection.setBounds(18,88,200,28);tags.setBounds(230,88,230,28);previous.setBounds(474,88,40,28);next.setBounds(522,88,40,28);star.setBounds(570,88,90,28);list.setBounds(18,126,642,212);name.setBounds(18,350,350,30);save.setBounds(378,350,130,30);load.setBounds(518,350,142,30);message.setBounds(18,387,420,35);import.setBounds(440,390,220,28);}
};
