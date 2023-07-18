/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
RtconvolveAudioProcessorEditor::RtconvolveAudioProcessorEditor (RtconvolveAudioProcessor& p)
    : AudioProcessorEditor (&p)
    , processor (p)
    , mButtonChooseIR("Choose Impulse Response")
{

    mButtonChooseIR.changeWidthToFitText();
    setSize (400, 300);
    
    Rectangle<int> boundingBox = getBounds();
    mButtonChooseIR.setBounds(boundingBox);

    addAndMakeVisible(&mButtonChooseIR);
    
    mButtonChooseIR.addListener(this);

}

RtconvolveAudioProcessorEditor::~RtconvolveAudioProcessorEditor()
{
}

void RtconvolveAudioProcessorEditor::buttonClicked(juce::Button* b)
{
    FileChooser fchooser("Select Impulse Response");
    if (fchooser.browseForFileToOpen())
    {
        File ir = fchooser.getResult();
        FileInputStream irInputStream(ir);
        AudioFormatManager manager;
        manager.registerBasicFormats();
        auto formatReader = manager.createReaderFor(ir);
        AudioSampleBuffer sampleBuffer(static_cast<int>(formatReader->numChannels), static_cast<int>(formatReader->lengthInSamples));
        formatReader->read(&sampleBuffer, 0, static_cast<int>(formatReader->lengthInSamples), 0, 1, 1);
        
        processor.setImpulseResponse(sampleBuffer, ir.getFullPathName());
        delete formatReader;
    }
}

//==============================================================================
void RtconvolveAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll (Colours::white);

    g.setColour (Colours::black);
    g.setFont (15.0f);

}

void RtconvolveAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}
