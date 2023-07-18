//
//  ConvolutionManager.h
//  RTConvolve
//
//  Created by Graham Barab on 2/7/17.
//
//

#ifndef ConvolutionManager_h
#define ConvolutionManager_h

#include "UniformPartitionConvolver.h"
#include "TimeDistributedFFTConvolver.h"
#include "../JuceLibraryCode/JuceHeader.h"
#include "util/util.h"
#include "util/SincFilter.hpp"

static const int DEFAULT_NUM_SAMPLES = 512;
static const int DEFAULT_BUFFER_SIZE = 512;

template <typename FLOAT_TYPE>
class ConvolutionManager
{
public:
    ConvolutionManager(FLOAT_TYPE *impulseResponse = nullptr, int numSamples = 0, int bufferSize = 0)
    : mBufferSize(bufferSize)
    , mTimeDistributedConvolver(nullptr)
    {
        if (impulseResponse == nullptr)
        {
            mBufferSize = DEFAULT_BUFFER_SIZE;
            mImpulseResponse = std::make_unique<juce::AudioBuffer<FLOAT_TYPE>>(1, DEFAULT_NUM_SAMPLES);
            
            FLOAT_TYPE *ir = mImpulseResponse->getWritePointer(0);
            genImpulse(ir, DEFAULT_NUM_SAMPLES);
            
            init(ir, DEFAULT_NUM_SAMPLES);
        }
        else
        {
            mImpulseResponse = std::make_unique<juce::AudioBuffer<FLOAT_TYPE>>(1, numSamples);
            mImpulseResponse->clear();
            FLOAT_TYPE *ir = mImpulseResponse->getWritePointer(0);
            
            memcpy(ir, impulseResponse, numSamples * sizeof(FLOAT_TYPE));
            init(ir, numSamples);
        }
    }
    
    /**
     Perform one base time period's worth of work for the convolution.
     @param input
     The input is expected to hold a number of samples equal to the 'bufferSize'
     specified in the constructor.
     */
    void processInput(FLOAT_TYPE *input)
    {
        mUniformConvolver->processInput(input);
        const FLOAT_TYPE *out1 = mUniformConvolver->getOutputBuffer();
        FLOAT_TYPE *output = mOutput->getWritePointer(0);
        
        /* Prepare output */
        
        if (mTimeDistributedConvolver != nullptr)
        {
            mTimeDistributedConvolver->processInput(input);
            const FLOAT_TYPE *out2 = mTimeDistributedConvolver->getOutputBuffer();
            
            for (int i = 0; i < mBufferSize; ++i)
            {
                output[i] = out1[i] + out2[i];
            }
        }
        else
        {
            for (int i = 0; i < mBufferSize; ++i)
            {
                output[i] = out1[i];
            }
        }
    }
    
    const FLOAT_TYPE *getOutputBuffer() const
    {
        return mOutput->getReadPointer(0);
    }
    
    void setBufferSize(int bufferSize)
    {
        FLOAT_TYPE *ir = mImpulseResponse->getWritePointer(0);
        int numSamples = mImpulseResponse->getNumSamples();
        mBufferSize = bufferSize;
        
        init(ir, numSamples);
    }
    
    void setImpulseResponse(const FLOAT_TYPE *impulseResponse, int numSamples)
    {
        mImpulseResponse = std::make_unique<juce::AudioBuffer<FLOAT_TYPE>>(1, numSamples);
        
        mImpulseResponse->clear();
        
        FLOAT_TYPE *ir = mImpulseResponse->getWritePointer(0);
        memcpy(ir, impulseResponse, numSamples * sizeof(FLOAT_TYPE));
        init(ir, numSamples);
    }
    
private:
    int mBufferSize;
    std::unique_ptr<UPConvolver<FLOAT_TYPE> > mUniformConvolver;
    std::unique_ptr<TimeDistributedFFTConvolver<FLOAT_TYPE> >  mTimeDistributedConvolver;
    std::unique_ptr<juce::AudioBuffer<FLOAT_TYPE> > mOutput;
    std::unique_ptr<juce::AudioBuffer<FLOAT_TYPE> > mImpulseResponse;
    
    void init(FLOAT_TYPE *impulseResponse, int numSamples)
    {
        mUniformConvolver = std::make_unique<UPConvolver<FLOAT_TYPE>>(impulseResponse, numSamples, mBufferSize, 8);
        
        FLOAT_TYPE *subIR = impulseResponse + (8 * mBufferSize);
        int subNumSamples = numSamples - (8 * mBufferSize);
        
        if (subNumSamples > 0)
        {
            mTimeDistributedConvolver = std::make_unique< TimeDistributedFFTConvolver<FLOAT_TYPE>>(subIR, subNumSamples, mBufferSize);
        }
        else
        {
            mTimeDistributedConvolver = nullptr;
        }
        
        mOutput = std::make_unique<juce::AudioBuffer<FLOAT_TYPE>>(1, mBufferSize);
    }
};

#endif /* ConvolutionManager_h */
