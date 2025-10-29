#pragma once

#include "../SignalProcessor.h"

bool SinController(const char* title, SinParam& paramRef);
void PlotRenderer(const char* title, const char* x_lable, const char* y_lable,
				  const double* x_data, const double* y_data, int count);