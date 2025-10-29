#pragma once

#include <iostream>
#include <vector>
#include <complex>
#include <cmath>
#include <algorithm>

using Complex = std::complex<double>;

const double PI = acos(-1.0);

std::vector<Complex> pad_to_power_of_two(const std::vector<Complex>& input);

std::vector<Complex> fft(const std::vector<Complex>& input);

std::vector<Complex> ifft(const std::vector<Complex>& input);

std::vector<Complex> fft_real(const std::vector<double>& real_input);

std::vector<double> amplitude_spectrum(const std::vector<Complex>& fft_result);

std::vector<double> phase_spectrum(const std::vector<Complex>& fft_result);