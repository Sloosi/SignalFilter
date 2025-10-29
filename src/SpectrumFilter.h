#pragma once

#include <vector>
#include "math/fft.h"

class SpectrumFilter
{
public:
    static std::vector<Complex>
    Apply(const std::vector<Complex>& spectrum,
          const std::vector<double>& amplitudes,
          double signalEnergyRatio);
};