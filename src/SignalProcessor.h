#pragma once
#include <vector>
#include "math/fft.h"

struct SinParam
{
    float amplitude = 0, frequency = 0, phase = 0;
};

class SignalProcessor
{
public:
    struct Config
    {
        int sampleRate = 1024;
        int pointCount = 1024;
        float noiseAlpha = 0.2;
        float gamma = 1.0;
        SinParam sin1{10, 10, 0};
        SinParam sin2{0, 0, 0};
        SinParam sin3{0, 0, 0};
    };

    void Update(const Config& cfg);

    const std::vector<double>& GetTime() const { return m_Time; }
    const std::vector<double>& GetInputSignal() const { return m_InputSignal; }
    const std::vector<double>& GetCleanSignal() const { return m_CleanSignal; }
    const std::vector<double>& GetFrequencies() const { return m_Frequencies; }
    const std::vector<double>& GetInputSpectrum() const { return m_InputSpectrum; }
    const std::vector<double>& GetCleanSpectrum() const { return m_CleanSpectrum; }

private:
    void GenerateWhiteNoise();
    void GenerateSignal();
    void ComputeSpectrum();
    void FilterSpectrum();

    Config m_Config;
    std::vector<double> m_Time, m_InputSignal, m_CleanSignal;
    std::vector<double> m_Noise;
    std::vector<double> m_Frequencies, m_InputSpectrum, m_CleanSpectrum;
    std::vector<Complex> m_Spectrum, m_FilteredSpectrum;
};

bool operator==(const SinParam& rhs, const SinParam& lhs);
bool operator==(const SignalProcessor::Config& rhs, const SignalProcessor::Config& lhs);