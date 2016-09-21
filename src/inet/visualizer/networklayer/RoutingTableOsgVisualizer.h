//
// Copyright (C) 2016 OpenSim Ltd.
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

#ifndef __INET_ROUTINGTABLEOSGVISUALIZER_H
#define __INET_ROUTINGTABLEOSGVISUALIZER_H

#include "inet/visualizer/base/RoutingTableVisualizerBase.h"

namespace inet {

namespace visualizer {

class INET_API RoutingTableOsgVisualizer : public RoutingTableVisualizerBase
{
#ifdef WITH_OSG

  protected:
    class INET_API OsgRoute : public Route {
      public:
        osg::Node *node = nullptr;

      public:
        OsgRoute(osg::Node *node, int nodeModuleId, int nextHopModuleId);
        virtual ~OsgRoute();
    };

  protected:
    virtual void addRoute(std::pair<int, int> nodeAndNextHop, const Route *route) override;
    virtual void removeRoute(const Route *route) override;

    virtual void setPosition(cModule *node, const Coord& position) const override;
    virtual const Route *createRoute(cModule *node, cModule *nextHop) const override;

#else // ifdef WITH_OSG

  protected:
    virtual void setPosition(cModule *node, const Coord& position) const override { }
    virtual const Route *createRoute(cModule *node, cModule *nextHop) const override { return nullptr; }

#endif // ifdef WITH_OSG
};

} // namespace visualizer

} // namespace inet

#endif // ifndef __INET_ROUTINGTABLEOSGVISUALIZER_H

