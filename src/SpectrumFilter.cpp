#include "SpectrumFilter.h"

std::vector<Complex>
SpectrumFilter::Apply(const std::vector<Complex>& spectrum,
                      const std::vector<double>& amplitudes,
                      double ratio)
{
    auto filtered = spectrum;
    double totalEnergy = 0;
    for (double a : amplitudes) totalEnergy += a * a;

    double targetEnergy = totalEnergy * ratio;
    double accumulated = 0;
    size_t k = 0;

    for (; k < amplitudes.size(); k++)
    {
        accumulated += amplitudes[k] * amplitudes[k];
        accumulated += amplitudes[amplitudes.size() - k - 1] * amplitudes[amplitudes.size() - k - 1];
        
        if (accumulated > targetEnergy) break;
    }

    if (k == 1024) return filtered;
    for (size_t i = k; i < amplitudes.size() - k; ++i)
    {
        filtered[i] = 0;
    }

    return filtered;
}