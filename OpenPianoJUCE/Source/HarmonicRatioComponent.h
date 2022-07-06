#include <JuceHeader.h>

#pragma once

//==============================================================================
class HarmonicRatioComponent : private juce::Timer
{
public:
    HarmonicRatioComponent(unsigned int sample_rate):
          harmonicRatioImage (juce::Image::RGB, 512, 512, true),
          Fs(sample_rate)
    {        
        audioData.resize(blockSize);
        fifo.resize(blockSize);

        startTimerHz(harmonicRatioRefreshRate);
    }

    //==============================================================================
    void pushNextSampleIntoFifo (float sample) noexcept
    {
        // if the fifo contains enough data, set a flag to say
        // that the next line should now be rendered..
        if (fifoIndex == blockSize)
        {
            if (! nextHarmonicRatioBlockReady)
            {
                std::fill (audioData.begin(), audioData.end(), 0.0f);
                std::copy (fifo.begin(), fifo.end(), audioData.begin());
                nextHarmonicRatioBlockReady = true;
            }
            fifoIndex = 0;
        }
        fifo[(size_t) fifoIndex++] = sample;
    }

    float computeHarmonicRatio(std::vector<float>& x)
    {
        uint32_t M = 0.04*Fs; // Maximum lag. The maximum lag is 40 ms,
                              // which corresponds to a minimum fundamental frequency of 25 Hz.


        std::vector<float> R(M);
        float sum1, sum2, sum3;
        uint32_t i, j;

        // Compute the normalized autocorrelation
        for (i = 0; i < M; i++)
        {
            sum1 = 0.0f;
            for (j = 1; j < M-i; j++)
            {
                sum1 += x[j] * x[j+i];
            }

            // Denominator
            sum2 = 0.0f;
            for (j = 1; j < M-i; j++)
            {
                sum2 += x[j] * x[j];
            }
            sum3 = 0.0f;
            for (j = 0; j < M-i; j++)
            {
                sum3 += x[j+i] * x[j+i];
            }

            R[i] = sum1/sqrt(sum2*sum3);
        }

        // The harmonic ratio is determined as the maximum
        // of the (normalized) autocorrelation, within a given range.
        // The lower edge of the search range is determined as the
        // first zero crossing of the normalized autocorrelation.
        unsigned int zero_cross_idx = 0;
        for (unsigned int k = 0; k < M-1; k++)
        {
            if (!std::signbit(x[k+1]) != !std::signbit(x[k]))
            {
                zero_cross_idx = k;
                break;
            }
        }

        float max_it = *max_element(R.begin()+zero_cross_idx, R.begin()+M);
        return max_it;
    }

    void drawNextLineOfHarmonicRatio()
    {
        auto rightHandEdge = harmonicRatioImage.getWidth() - 1;
        auto imageHeight   = harmonicRatioImage.getHeight();

        // first, shuffle our image leftwards by 1 pixel..
        harmonicRatioImage.moveImageSection (0, 0, 1, 0, rightHandEdge, imageHeight);

        // then compute the current harmonic ratio...
        currentHarmonicRatio = computeHarmonicRatio(audioData);

        //std::cout << "Harm. ratio: " << currentHarmonicRatio << std::endl;

        // Set the line to black
        for (auto y = 0; y < imageHeight; ++y)
        {
            harmonicRatioImage.setPixelAt (rightHandEdge, y, juce::Colour::fromRGB (0, 0, 0));
        }
        // Set the point corresponding to the harm. ratio to white
        if(currentHarmonicRatio <= 1 && currentHarmonicRatio >= -1)
        {
            auto y = (1-currentHarmonicRatio)*256;
            harmonicRatioImage.setPixelAt (rightHandEdge, y, juce::Colour::fromRGB (255, 255, 255));
        }
    }

    void timerCallback() override
    {
        if (nextHarmonicRatioBlockReady)
        {
            drawNextLineOfHarmonicRatio();
            nextHarmonicRatioBlockReady = false;
        }
    }

    const juce::Image getharmonicRatioImage()
    {
        return (const juce::Image)this->harmonicRatioImage;
    }

private:
    juce::Image harmonicRatioImage;

    unsigned int Fs;
    unsigned int blockSize = 2048;
    float currentHarmonicRatio;
    std::vector<float> audioData;
    std::vector<float> fifo;
    unsigned int fifoIndex = 0;
    bool nextHarmonicRatioBlockReady = false;
    int harmonicRatioRefreshRate = 60;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HarmonicRatioComponent)
};
