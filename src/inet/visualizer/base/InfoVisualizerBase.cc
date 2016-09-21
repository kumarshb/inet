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

#include "inet/visualizer/base/InfoVisualizerBase.h"

namespace inet {

namespace visualizer {

void InfoVisualizerBase::initialize(int stage)
{
    VisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        fontColor = cFigure::Color(par("fontColor"));
        backgroundColor = cFigure::Color(par("backgroundColor"));
        moduleMatcher.setPattern(par("modules"), true, true, true);
        auto simulation = getSimulation();
        for (int id = 0; id < simulation->getLastComponentId(); id++) {
            auto component = simulation->getComponent(id);
            if (component != nullptr && component->isModule() && moduleMatcher.matches(component->getFullPath().c_str()))
                moduleIds.push_back(static_cast<cModule *>(component)->getId());
        }
    }
}

void InfoVisualizerBase::refreshDisplay() const
{
    auto simulation = getSimulation();
    for (int i = 0; i < moduleIds.size(); i++) {
        auto module = simulation->getModule(moduleIds[i]);
        if (module != nullptr) {
            const char *content = par("content");
            const char *info;
            if (!strcmp(content, "info"))
                info = module->info().c_str();
            else if (!strcmp(content, "displayStringText"))
                info = module->getDisplayString().getTagArg("t", 0);
            else
                throw cRuntimeError("Unknown content parameter value: %s", content);
            setInfo(i, info);
        }
    }
}

} // namespace visualizer

} // namespace inet

