#include <layers/AlphaBetaFilter.h>
#include <Random.h>
#include <optimization/OptimizationTricks.h>

namespace OpenANN {

AlphaBetaFilter::AlphaBetaFilter(OutputInfo info, fpt deltaT, bool bias, fpt stdDev)
  : debugLogger(Logger::CONSOLE),
    I(info.outputs()), J(2*I), deltaT(deltaT), bias(bias), stdDev(stdDev), gamma(I),
    gammad(I), alpha(I), beta(I), first(true), x(0), y(J+bias)
{
}

OutputInfo AlphaBetaFilter::initialize(std::vector<fpt*>& parameterPointers,
                                       std::vector<fpt*>& parameterDerivativePointers)
{
  parameterPointers.reserve(parameterPointers.size() + I);
  parameterDerivativePointers.reserve(parameterDerivativePointers.size() + I);
  for(int i = 0; i < I; i++)
  {
    parameterPointers.push_back(&gamma(i));
    parameterDerivativePointers.push_back(&gammad(i));
  }

  // Bias component will not change after initialization
  if(bias)
    y(J) = fpt(1.0);

  initializeParameters();

  OutputInfo info;
  info.bias = bias;
  info.dimensions.push_back(J);
  return info;
}

void AlphaBetaFilter::initializeParameters()
{
  RandomNumberGenerator rng;
  for(int i = 0; i < I; i++)
  {
    gamma(i) = rng.sampleNormalDistribution<fpt>() * stdDev;
    gammad(i) = (fpt) 0;
  }
}

void AlphaBetaFilter::updatedParameters()
{
  for(int i = 0; i < I; i++)
  {
    gamma(i) = OpenANN::periodicMapping(gamma(i), 0.5);
    const fpt r = (4.0 + gamma(i) - sqrt(8.0 * gamma(i) + gamma(i) * gamma(i))) / 4.0;
    alpha(i) = 1.0 - r*r;
    const fpt rr = 1.0 - r;
    beta(i) = 2.0 * rr * rr;
  }
  reset();
}

void AlphaBetaFilter::reset()
{
  first = true;
  y.fill(0.0);
}

void AlphaBetaFilter::forwardPropagate(Vt* x, Vt*& y, bool dropout)
{
  this->x = x;

  if(first)
  {
    for(int i = 0, j = 0; i < I; i++, j+=2)
      this->y(j) = (*x)(i);
    first = false;
  }

  for(int i = 0, j = 0; i < I; i++, j+=2)
  {
    const fpt posEstimated = this->y(j) + deltaT * this->y(j+1);
    const fpt velEstimated = this->y(j+1);
    const fpt r = (*x)(i) - posEstimated;
    this->y(j) = posEstimated + alpha(i) * r;
    this->y(j+1) = velEstimated + beta(i) / deltaT * r;
  }

  y = &(this->y);
}

void AlphaBetaFilter::backpropagate(Vt* ein, Vt*& eout)
{
  // Do nothing.
}

Vt& AlphaBetaFilter::getOutput()
{
  return y;
}

}
