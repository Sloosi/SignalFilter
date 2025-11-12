#include "SignalProcessor.h"
#include "SpectrumFilter.h"

SignalProcessor::SignalProcessor()
{
    m_Time.resize(m_Config.pointCount);
    m_InputSignal.resize(m_Config.pointCount);
    m_Noise.resize(m_Config.pointCount);
    m_CleanSignal.resize(m_Config.pointCount);
    m_IdealSignal.resize(m_Config.pointCount);

    GenerateWhiteNoise();
    GenerateSignal();
    ComputeSpectrum();
    FilterSpectrum();

    // IFFT
    auto ifftRes = ifft(m_FilteredSpectrum);
    for (size_t i = 0; i < ifftRes.size(); ++i)
    {
        m_CleanSignal[i] = ifftRes[i].real();
    }

    CalculateDelta();
}

void SignalProcessor::Update(const Config& cfg)
{
    if (cfg == m_Config) return;
    if (cfg.pointCount != m_Config.pointCount || cfg.sampleRate != m_Config.sampleRate)
    {
        m_Noise.resize(cfg.pointCount);
        GenerateWhiteNoise();
    }

    m_Config = cfg;

    m_Time.resize(m_Config.pointCount);
    m_InputSignal.resize(m_Config.pointCount);
    m_CleanSignal.resize(m_Config.pointCount);
    m_IdealSignal.resize(m_Config.pointCount);

    GenerateSignal();
    ComputeSpectrum();
    FilterSpectrum();

    // IFFT
    auto ifftRes = ifft(m_FilteredSpectrum);
    for (size_t i = 0; i < ifftRes.size(); ++i)
    {
        m_CleanSignal[i] = ifftRes[i].real();
    }

    CalculateDelta();
}

void SignalProcessor::CalculateDelta()
{
    m_Delta = 0;

    double counter = 0.0;

    for (size_t i = 0; i < m_Config.pointCount; i++)
    {
        m_Delta += (m_CleanSignal[i] - m_IdealSignal[i]) * (m_CleanSignal[i] - m_IdealSignal[i]);
        counter += m_IdealSignal[i] * m_IdealSignal[i];
    }
    m_Delta /= counter;
}

void SignalProcessor::GenerateWhiteNoise()
{
    for (size_t i = 0; i < m_Noise.size(); i++)
    {
        int noise = 0;
        for (size_t s = 0; s < 12; s++)
        {
            noise += rand();
        }
        m_Noise[i] = (noise - static_cast<double>(RAND_MAX * 6)) / (12 * RAND_MAX);
    }
}

void SignalProcessor::GenerateSignal()
{
    double dt = 1.0 / m_Config.sampleRate;
    double signalEnergy = 0;

    for (int i = 0; i < m_Config.pointCount; ++i)
    {
        double t = i * dt;
        double s1 = m_Config.sin1.amplitude * sin(2 * PI * m_Config.sin1.frequency * t + m_Config.sin1.phase);
        double s2 = m_Config.sin2.amplitude * sin(2 * PI * m_Config.sin2.frequency * t + m_Config.sin2.phase);
        double s3 = m_Config.sin3.amplitude * sin(2 * PI * m_Config.sin3.frequency * t + m_Config.sin3.phase);

        m_Time[i] = t;
        m_InputSignal[i] = s1 + s2 + s3;
        m_IdealSignal[i] = s1 + s2 + s3;
        signalEnergy += m_InputSignal[i] * m_InputSignal[i];
    }

    double noiseEnergy = 0;
    for (double n : m_Noise) noiseEnergy += n * n;

    double beta = std::sqrt(signalEnergy * m_Config.noiseAlpha / noiseEnergy);
    for (int i = 0; i < m_Config.pointCount; ++i)
    {
        m_InputSignal[i] += m_Noise[i] * beta;
    }
}

void SignalProcessor::ComputeSpectrum()
{
    auto N = m_InputSignal.size();

    auto fft_res = fft_real(m_InputSignal);
    m_Spectrum = fft_res;
    m_InputSpectrum = amplitude_spectrum(fft_res);

    m_Frequencies.resize(N);
    m_InputSpectrum.resize(N);
    for (int k = 0; k < N; k++)
    {
        m_Frequencies[k] = static_cast<float>(k * m_Config.sampleRate) / N;
    }
}

void SignalProcessor::FilterSpectrum()
{
    m_FilteredSpectrum = SpectrumFilter::Apply(m_Spectrum, m_InputSpectrum, m_Config.gamma);
    m_CleanSpectrum = amplitude_spectrum(m_FilteredSpectrum);
}

bool operator==(const SinParam &rhs, const SinParam &lhs)
{
    return 
        (rhs.amplitude == lhs.amplitude) &&
        (rhs.frequency == lhs.frequency) &&
        (rhs.phase == lhs.phase);
}

bool operator==(const SignalProcessor::Config &rhs, const SignalProcessor::Config &lhs)
{
    return 
        (rhs.showNoise == lhs.showNoise) &&
        (rhs.sampleRate == lhs.sampleRate) &&
        (rhs.pointCount == lhs.pointCount) &&
        (rhs.noiseAlpha == lhs.noiseAlpha) &&
        (rhs.gamma == lhs.gamma) &&
        (rhs.sin1 == lhs.sin1) &&
        (rhs.sin2 == lhs.sin2) &&
        (rhs.sin3 == lhs.sin3);
}
