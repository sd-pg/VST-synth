//Copyright 2022 Artem Ryaboshapko
//
//This program is free software : you can redistribute it and /or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program.If not, see < https://www.gnu.org/licenses/>.

#include "PluginProcessor.h"
#include "PluginEditor.h"

TapSynthAudioProcessorEditor::TapSynthAudioProcessorEditor(TapSynthAudioProcessor& p)
    : AudioProcessorEditor(&p)
    , audioProcessor(p)
    , osc(audioProcessor.apvts, "OSC1WAVETYPE", "OSC1FMFREQ", "OSC1FMDEPTH")
    , adsr("ADSR", audioProcessor.apvts, "ATTACK", "DECAY", "SUSTAIN", "RELEASE")
    , filterAdsr("ADSR Filter", audioProcessor.apvts, "FILTERATTACK", "FILTERDECAY", "FILTERSUSTAIN", "FILTERRELEASE")
    , filter(audioProcessor.apvts, "FILTERTYPE", "FILTERFREQ", "FILTERRES")

{
    setSize(620, 700);
    addAndMakeVisible(osc);
    addAndMakeVisible(adsr);
    addAndMakeVisible(filterAdsr);
    addAndMakeVisible(filter);

    createGUI();
    
    addAndMakeVisible(audioProcessor.visualise);
}

TapSynthAudioProcessorEditor::~TapSynthAudioProcessorEditor()
{
}

void TapSynthAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
}

void TapSynthAudioProcessorEditor::resized()
{
    const auto paddingX = 5;
    const auto paddingY = 35;
    const auto paddingY2 = 235;

    osc.setBounds(paddingX, paddingY, 300, 200);
    adsr.setBounds(osc.getRight(), paddingY, 300, 200);
    filterAdsr.setBounds(paddingX, paddingY2, 300, 200);
    filter.setBounds(filterAdsr.getRight(), paddingY2, 300, 200);
    audioProcessor.visualise.setBounds(10, 510, 600, 150);
    presetSelect.setBounds(10, 10, 90, 30);
}

void TapSynthAudioProcessorEditor::createGUI()
{
    presetSelect.addItem("default", 1);  
    presetSelect.addItem("preset 1", 2);
    presetSelect.addItem("preset 2", 3);
    addAndMakeVisible(presetSelect);
    presetSelectAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(audioProcessor.parameters, "MICOMBO_ID", presetSelect);
}