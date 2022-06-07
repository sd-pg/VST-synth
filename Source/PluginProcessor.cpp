#include "PluginProcessor.h"
#include "PluginEditor.h"

TapSynthAudioProcessor::TapSynthAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    ), apvts(*this, nullptr, "Parameters", createParams()) 
    , parameters(*this, nullptr, "PARAMETERS", intializeGUI())

    
#endif
{
    synth.addSound(new SynthSound()); 
    synth.addVoice(new SynthVoice());
}

TapSynthAudioProcessor::~TapSynthAudioProcessor()
{

}

const juce::String TapSynthAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool TapSynthAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool TapSynthAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool TapSynthAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double TapSynthAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int TapSynthAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int TapSynthAudioProcessor::getCurrentProgram()
{
    return 0;
}

void TapSynthAudioProcessor::setCurrentProgram(int index)
{
}

const juce::String TapSynthAudioProcessor::getProgramName(int index)
{
    return {};
}

void TapSynthAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
}

void TapSynthAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)  //Вызывается перед началом воспроизведения, чтобы позволить процессору подготовиться.
{
    synth.setCurrentPlaybackSampleRate(sampleRate); //контролирует скорость воспроизведения
    //visualise.clear();
    visualise.setColours(juce::Colours::blue, juce::Colours::green);
    visualise.setBufferSize(256);
    visualise.setSamplesPerBlock(16);
    visualise.setRepaintRate(10);

    for (int i = 0; i < synth.getNumVoices(); i++)
    {
        if (auto voice = dynamic_cast<SynthVoice*>(synth.getVoice(i))) //преобразовываем, чтобы была возможность вызвать свой метод
        {
            voice->prepareToPlay(sampleRate, samplesPerBlock, getTotalNumOutputChannels());
        }
    }
}

void TapSynthAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool TapSynthAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
#if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}
#endif

void TapSynthAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());
    {  

        for (int i = 0; i < synth.getNumVoices(); ++i)//в цикле происходит обновление изменяемых пользователем значений
        {
            if (auto voice = dynamic_cast<SynthVoice*>(synth.getVoice(i))) //проверка на то что voice правильно преобразовался
            {
                // Osc
                auto& oscWaveChoice = *apvts.getRawParameterValue("OSC1WAVETYPE");

                // FM
                auto& fmFreq = *apvts.getRawParameterValue("OSC1FMFREQ");
                auto& fmDepth = *apvts.getRawParameterValue("OSC1FMDEPTH");

                // Amp Adsr
                auto& attack = *apvts.getRawParameterValue("ATTACK");
                auto& decay = *apvts.getRawParameterValue("DECAY");
                auto& sustain = *apvts.getRawParameterValue("SUSTAIN");
                auto& release = *apvts.getRawParameterValue("RELEASE");

                // Filter Adsr
                auto& fAttack = *apvts.getRawParameterValue("FILTERATTACK");
                auto& fDecay = *apvts.getRawParameterValue("FILTERDECAY");
                auto& fSustain = *apvts.getRawParameterValue("FILTERSUSTAIN");
                auto& fRelease = *apvts.getRawParameterValue("FILTERRELEASE");

                // Filter
                auto& filterType = *apvts.getRawParameterValue("FILTERTYPE");
                auto& cutoff = *apvts.getRawParameterValue("FILTERFREQ");
                auto& resonance = *apvts.getRawParameterValue("FILTERRES");

                // Update voice
                voice->getOscillator().setWaveType(oscWaveChoice);
                voice->getOscillator().updateFm(fmFreq, fmDepth);
                voice->getAdsr().update(attack.load(), decay.load(), sustain.load(), release.load());
                voice->getFilterAdsr().update(fAttack.load(), fDecay.load(), fSustain.load(), fRelease.load());
                voice->updateFilter(filterType, cutoff, resonance);
            }
        }
    }
    
    synth.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());
    visualise.pushBuffer(buffer); 
    setPreset();
}

bool TapSynthAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* TapSynthAudioProcessor::createEditor()
{
    return new TapSynthAudioProcessorEditor(*this);
}

void TapSynthAudioProcessor::getStateInformation(juce::MemoryBlock& destData) //сохраняяет и загружает инофрмацию в плагин 
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    auto state = apvts.state;
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void TapSynthAudioProcessor::setStateInformation(const void* data, int sizeInBytes) //сохраняет состояние плагина перд закрытием 
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
        
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
   
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TapSynthAudioProcessor();
}

juce::AudioProcessorValueTreeState::ParameterLayout TapSynthAudioProcessor::createParams()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    //OSC select
    params.push_back(std::make_unique<juce::AudioParameterChoice>("OSC1WAVETYPE", "Osc 1 Wave Type", juce::StringArray{ "Sine", "Saw", "Square" }, 0));

    // FM
    params.push_back(std::make_unique<juce::AudioParameterFloat>("OSC1FMFREQ", "Osc 1 FM Frequency", juce::NormalisableRange<float> { 0.0f, 1000.0f, 0.01f, 0.3f }, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("OSC1FMDEPTH", "Osc 1 FM Depth", juce::NormalisableRange<float> { 0.0f, 1000.0f, 0.01f, 0.3f }, 0.0f));

    // ADSR
    params.push_back(std::make_unique<juce::AudioParameterFloat>("ATTACK", "Attack", juce::NormalisableRange<float> { 0.1f, 1.0f, 0.1f }, 0.1f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("DECAY", "Decay", juce::NormalisableRange<float> { 0.1f, 1.0f, 0.1f }, 0.1f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("SUSTAIN", "Sustain", juce::NormalisableRange<float> { 0.1f, 1.0f, 0.1f }, 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("RELEASE", "Release", juce::NormalisableRange<float> { 0.1f, 3.0f, 0.1f }, 0.4f));

    // Filter ADSR
    params.push_back(std::make_unique<juce::AudioParameterFloat>("FILTERATTACK", "Filter Attack", juce::NormalisableRange<float> { 0.1f, 1.0f, 0.1f }, 0.1f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("FILTERDECAY", "Filter Decay", juce::NormalisableRange<float> { 0.1f, 1.0f, 0.1f }, 0.1f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("FILTERSUSTAIN", "Filter Sustain", juce::NormalisableRange<float> { 0.1f, 1.0f, 0.1f }, 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("FILTERRELEASE", "Filter Release", juce::NormalisableRange<float> { 0.1f, 3.0f, 0.1f }, 0.4f));

    // Filter
    params.push_back(std::make_unique<juce::AudioParameterChoice>("FILTERTYPE", "Filter Type", juce::StringArray{ "Low-Pass", "Band-Pass", "High-Pass" }, 0));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("FILTERFREQ", "Filter Freq", juce::NormalisableRange<float> { 20.0f, 20000.0f, 0.1f, 0.6f }, 200.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("FILTERRES", "Filter Resonance", juce::NormalisableRange<float> { 1.0f, 10.0f, 0.1f }, 1.0f));
    
    return { params.begin(), params.end() };
}

void TapSynthAudioProcessor::setPreset()
{

    if (b == (int)*parameters.getRawParameterValue("MICOMBO_ID"))
        return;

    else
    {
        switch ((int)*parameters.getRawParameterValue("MICOMBO_ID"))
        {
        case 0:
            preXml = juce::XmlDocument::parse(juce::File("C:/presets/2.xml"));
            apvts.state = (juce::ValueTree::fromXml(*preXml));
            b = (int)*parameters.getRawParameterValue("MICOMBO_ID");
            break;
    
        case 1:
            preXml = juce::XmlDocument::parse(juce::File("C:/presets/1.xml"));
            apvts.state = (juce::ValueTree::fromXml(*preXml));
            b = (int)*parameters.getRawParameterValue("MICOMBO_ID");
            break;

        case 2:
            preXml = juce::XmlDocument::parse(juce::File("C:/presets/3.xml"));
            apvts.state = (juce::ValueTree::fromXml(*preXml));
            b = (int)*parameters.getRawParameterValue("MICOMBO_ID");
            break;
        }
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout TapSynthAudioProcessor::intializeGUI()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> par_preset;
    par_preset.push_back(std::make_unique<juce::AudioParameterChoice>("MICOMBO_ID", "MICOMBO_NAME", juce::StringArray("default", "preset 1", "preset 2"), 0));

    return { par_preset.begin(), par_preset.end() };
}
