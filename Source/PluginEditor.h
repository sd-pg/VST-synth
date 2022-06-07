#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "UI/AdsrComponent.h"
#include "UI/OscComponent.h"
#include "UI/FilterComponent.h"

//==============================================================================
/**
*/
class TapSynthAudioProcessorEditor : public juce::AudioProcessorEditor  
                                     
{
public:
    TapSynthAudioProcessorEditor(TapSynthAudioProcessor&);
    ~TapSynthAudioProcessorEditor() override;

    //==============================================================================
    void paint(juce::Graphics&) override;
    void resized() override;
    
    void createGUI();
   

private:
    TapSynthAudioProcessor& audioProcessor;
    OscComponent osc;
    AdsrComponent adsr;
    AdsrComponent filterAdsr;
    FilterComponent filter;
    juce::ComboBox presetSelect;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> presetSelectAttachment;
    
 
     

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TapSynthAudioProcessorEditor)
};