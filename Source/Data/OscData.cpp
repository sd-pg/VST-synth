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

#include "OscData.h"

void OscData::prepareToPlay(juce::dsp::ProcessSpec& spec)
{
    prepare(spec);
    fmOsc.prepare(spec);
}

void OscData::setWaveType(const int choice)
{
    switch (choice)
    {
    case 0:
        // Sine
        initialise([](float x) { return std::sin(x); });
        break;

    case 1:
        // Saw wave
        initialise([](float x) { return x / juce::MathConstants<float>::pi; });
        break;

    case 2:
        // Square wave
        initialise([](float x) { return x < 0.0f ? -1.0f : 1.0f; });
        break;

    default:
        jassertfalse;   // You're not supposed to be here!
        break;
    }
}

void OscData::setWaveFrequency(const int midiNoteNumber)
{
    setFrequency(juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber) + fmMod);
    //меняем частоту на осциляторе в зависимости от нажатой клавиши на миди клавиатуре
    // значение fmMod [1;-1], это волна будет модулировать частоту нашей основной волны
    lastMidiNote = midiNoteNumber; // мы не можем передать локальное значение midiNoteNumber в другую функцию, потому создали эту переменную
}

void OscData::getNextAudioBlock(juce::dsp::AudioBlock<float>& block)
{
    processFmOsc(block);
    process(juce::dsp::ProcessContextReplacing<float>(block));
}

void OscData::processFmOsc(juce::dsp::AudioBlock<float>& block)
{
    for (int ch = 0; ch < block.getNumChannels(); ++ch)
    {
        for (int s = 0; s < block.getNumSamples(); ++s)
        {
            fmMod = fmOsc.processSample(block.getSample(ch, s)) * fmDepth;
        }
    }
}

void OscData::updateFm(const float freq, const float depth) // обновляем значения переданные с midi чтобы наш осцилятор мог реагировать и изменять их
{
    fmOsc.setFrequency(freq);
    fmDepth = depth;
    auto currentFreq = juce::MidiMessage::getMidiNoteInHertz(lastMidiNote) + fmMod;
    setFrequency(currentFreq >= 0 ? currentFreq : currentFreq * -1.0f);
}