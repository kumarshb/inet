//
// Copyright (C) OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_STATISTICVISUALIZERBASE_H
#define __INET_STATISTICVISUALIZERBASE_H

#include "inet/common/PatternMatcher.h"
#include "inet/visualizer/base/VisualizerBase.h"

namespace inet {

namespace visualizer {

class INET_API StatisticVisualizerBase : public VisualizerBase, public cListener
{
  protected:
    class INET_API LastValueRecorder : public cNumericResultRecorder
    {
      protected:
        double lastValue = NaN;

      protected:
        virtual void collect(simtime_t_cref t, double value DETAILS_ARG) override { lastValue = value; }

      public:
        void setLastValue(double lastValue) { this->lastValue = lastValue; }
        double getLastValue() const { return lastValue; }
    };

    class StatisticVisualization {
      public:
        LastValueRecorder *recorder = nullptr;
        int moduleId = -1;
        simsignal_t signal = -1;
        const char *unit = nullptr;

      public:
        StatisticVisualization(int moduleId, simsignal_t signal, const char *unit);
    };

  protected:
    /** @name Parameters */
    //@{
    cModule *subscriptionModule = nullptr;
    PatternMatcher sourcePathMatcher;
    const char *signalName = nullptr;
    const char *statisticName = nullptr;
    const char *unit = nullptr;
    const char *prefix = nullptr;
    cFigure::Color fontColor;
    cFigure::Color backgroundColor;
    double opacity = NaN;
    double minValue = NaN;
    double maxValue = NaN;
    //@}

    std::map<std::pair<int, simsignal_t>, StatisticVisualization *> statisticVisualizations;

  protected:
    virtual void initialize(int stage) override;

    virtual cResultFilter *findResultFilter(cComponent *source, simsignal_t signal);
    virtual cResultFilter *findResultFilter(cResultFilter *parentResultFilter, cResultListener *resultListener);
    virtual std::string getText(StatisticVisualization *statisticVisualization);
    virtual const char *getUnit(cComponent *source);

    virtual StatisticVisualization *createStatisticVisualization(cComponent *source, simsignal_t signal) = 0;
    virtual StatisticVisualization *getStatisticVisualization(cComponent *source, simsignal_t signal);
    virtual void addStatisticVisualization(StatisticVisualization *statisticVisualization);
    virtual void removeStatisticVisualization(StatisticVisualization *statisticVisualization);

    virtual void refreshStatisticVisualization(StatisticVisualization *statisticVisualization) = 0;
    virtual void processSignal(cComponent *source, simsignal_t signal, double value);

  public:
    virtual void receiveSignal(cComponent *source, simsignal_t signal, bool b DETAILS_ARG) override { processSignal(source, signal, NaN); }
    virtual void receiveSignal(cComponent *source, simsignal_t signal, long l DETAILS_ARG) override { processSignal(source, signal, l); }
    virtual void receiveSignal(cComponent *source, simsignal_t signal, unsigned long l DETAILS_ARG) override { processSignal(source, signal, l); }
    virtual void receiveSignal(cComponent *source, simsignal_t signal, double d DETAILS_ARG) override { processSignal(source, signal, d); }
    virtual void receiveSignal(cComponent *source, simsignal_t signal, const SimTime& t DETAILS_ARG) override { processSignal(source, signal, t.dbl()); }
    virtual void receiveSignal(cComponent *source, simsignal_t signal, const char *s DETAILS_ARG) override { processSignal(source, signal, NaN); }
    virtual void receiveSignal(cComponent *source, simsignal_t signal, cObject *obj DETAILS_ARG) override { processSignal(source, signal, NaN); }
};

} // namespace visualizer

} // namespace inet

#endif // ifndef __INET_STATISTICVISUALIZERBASE_H

