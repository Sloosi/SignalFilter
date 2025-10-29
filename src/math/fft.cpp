#include "fft.h"

std::vector<Complex> pad_to_power_of_two(const std::vector<Complex>& input)
{
    size_t n = input.size();

    size_t new_size = 1;
    while (new_size < n)
    {
        new_size *= 2;
    }

    std::vector<Complex> padded(new_size);
    for (size_t i = 0; i < n; ++i)
    {
        padded[i] = input[i];
    }
    for (size_t i = n; i < new_size; ++i)
    {
        padded[i] = Complex(0.0, 0.0);
    }

    return padded;
}

std::vector<Complex> fft(const std::vector<Complex>& input)
{
    
    auto pad_input = pad_to_power_of_two(input);

    int n = pad_input.size();

    if (n == 1) {
        return { pad_input[0] };
    }

    std::vector<Complex> even(n / 2), odd(n / 2);
    for (int i = 0; i < n / 2; ++i) {
        even[i] = pad_input[2 * i];
        odd[i] = pad_input[2 * i + 1];
    }

    
    auto even_fft = fft(even);
    auto odd_fft = fft(odd);

    
    std::vector<Complex> result(n);
    for (int k = 0; k < n / 2; ++k) {
        Complex t = std::polar(1.0, -2.0 * PI * k / n) * odd_fft[k];

        result[k] = even_fft[k] + t;
        result[k + n / 2] = even_fft[k] - t;
    }

    return result;
}

std::vector<Complex> ifft(const std::vector<Complex>& input)
{
    int n = input.size();

    std::vector<Complex> conjugated(n);
    for (int i = 0; i < n; ++i) {
        conjugated[i] = std::conj(input[i]);
    }

    auto result = fft(conjugated);

    for (int i = 0; i < n; ++i) {
        result[i] = std::conj(result[i]) / static_cast<double>(n);
    }

    return result;
}

std::vector<Complex> fft_real(const std::vector<double>& real_input)
{
    std::vector<Complex> complex_input(real_input.size());
    for (size_t i = 0; i < real_input.size(); ++i) {
        complex_input[i] = Complex(real_input[i], 0.0);
    }
    return fft(complex_input);
}

std::vector<double> amplitude_spectrum(const std::vector<Complex>& fft_result)
{
    auto N = fft_result.size();
    
    std::vector<double> spectrum(N);
    for (size_t i = 0; i < fft_result.size(); ++i) {
        spectrum[i] = 2 * std::abs(fft_result[i]) / N;
    }
    spectrum[0] = 0;
    return spectrum;
}

std::vector<double> phase_spectrum(const std::vector<Complex>& fft_result)
{
    std::vector<double> spectrum(fft_result.size());
    for (size_t i = 0; i < fft_result.size(); ++i)
    {
        spectrum[i] = std::arg(fft_result[i]);
    }
    return spectrum;
}