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

#include "FilterData.h"

void FilterData::prepareToPlay(double sampleRate, int samplesPerBlock, int numChannels)
{
    filter.reset();

    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.sampleRate = sampleRate;
    spec.numChannels = numChannels;

    filter.prepare(spec);

    isPrepared = true;
}

void FilterData::process(juce::AudioBuffer<float>& buffer)
{
    jassert(isPrepared);

    juce::dsp::AudioBlock<float> block{ buffer };
    filter.process(juce::dsp::ProcessContextReplacing<float> { block });
}

void FilterData::updateParameters(const float modulator, const int filterType, const float frequency, const float resonance)
{
    switch (filterType)
    {
    case 0:
        filter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
        break;

    case 1:
        filter.setType(juce::dsp::StateVariableTPTFilterType::bandpass);
        break;

    case 2:
        filter.setType(juce::dsp::StateVariableTPTFilterType::highpass);
        break;
    }

    float modulatedFreq = frequency * modulator;
    modulatedFreq = std::fmax(std::fmin(modulatedFreq, 20000.0f), 20.0f); // границы для частоты

    filter.setCutoffFrequency(modulatedFreq);
    filter.setResonance(resonance);
}

void FilterData::reset()
{
    filter.reset();
}