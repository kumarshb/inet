// Copyright (C) 2012 OpenSim Ltd
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

#include "inet/physicallayer/ieee80211/mode/Ieee80211ModeSet.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211FHSSMode.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211IRMode.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211DSSSMode.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211HRDSSSMode.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211OFDMMode.h"

namespace inet {

namespace physicallayer {

const std::vector<Ieee80211ModeSet> Ieee80211ModeSet::modeSets = {
    Ieee80211ModeSet('a', {
        {true, &Ieee80211OFDMCompliantModes::ofdmMode6MbpsCS20MHz},
        {false, &Ieee80211OFDMCompliantModes::ofdmMode9MbpsCS20MHz},
        {true, &Ieee80211OFDMCompliantModes::ofdmMode12MbpsCS20MHz},
        {false, &Ieee80211OFDMCompliantModes::ofdmMode18MbpsCS20MHz},
        {true, &Ieee80211OFDMCompliantModes::ofdmMode24MbpsCS20MHz},
        {false, &Ieee80211OFDMCompliantModes::ofdmMode36Mbps},
        {false, &Ieee80211OFDMCompliantModes::ofdmMode48Mbps},
        {false, &Ieee80211OFDMCompliantModes::ofdmMode54Mbps},
    }),
    Ieee80211ModeSet('b', {
        {true, &Ieee80211DsssCompliantModes::dsssMode1Mbps},
        {true, &Ieee80211DsssCompliantModes::dsssMode2Mbps},
        {true, &Ieee80211HrDsssCompliantModes::hrDsssMode5_5MbpsCckLongPreamble},
        {true, &Ieee80211HrDsssCompliantModes::hrDsssMode11MbpsCckLongPreamble},
    }),
    Ieee80211ModeSet('g', {
        {true, &Ieee80211DsssCompliantModes::dsssMode1Mbps},
        {true, &Ieee80211DsssCompliantModes::dsssMode2Mbps},
        {true, &Ieee80211HrDsssCompliantModes::hrDsssMode5_5MbpsCckLongPreamble},
        {true, &Ieee80211OFDMCompliantModes::ofdmMode6MbpsCS20MHz}, // TODO: should be ErpOfdm
        {false, &Ieee80211OFDMCompliantModes::ofdmMode9MbpsCS20MHz}, // TODO: should be ErpOfdm
        {true, &Ieee80211HrDsssCompliantModes::hrDsssMode11MbpsCckLongPreamble},
        {true, &Ieee80211OFDMCompliantModes::ofdmMode12MbpsCS20MHz}, // TODO: should be ErpOfdm
        {false, &Ieee80211OFDMCompliantModes::ofdmMode18MbpsCS20MHz}, // TODO: should be ErpOfdm
        {true, &Ieee80211OFDMCompliantModes::ofdmMode24MbpsCS20MHz}, // TODO: should be ErpOfdm
        {false, &Ieee80211OFDMCompliantModes::ofdmMode36Mbps}, // TODO: should be ErpOfdm
        {false, &Ieee80211OFDMCompliantModes::ofdmMode48Mbps}, // TODO: should be ErpOfdm
        {false, &Ieee80211OFDMCompliantModes::ofdmMode54Mbps}, // TODO: should be ErpOfdm
    }),
    Ieee80211ModeSet('p', {
        {true, &Ieee80211OFDMCompliantModes::ofdmMode3MbpsCS10MHz},
        {false, &Ieee80211OFDMCompliantModes::ofdmMode4_5MbpsCS10MHz},
        {true, &Ieee80211OFDMCompliantModes::ofdmMode6MbpsCS10MHz},
        {false, &Ieee80211OFDMCompliantModes::ofdmMode9MbpsCS10MHz},
        {true, &Ieee80211OFDMCompliantModes::ofdmMode12MbpsCS10MHz},
        {false, &Ieee80211OFDMCompliantModes::ofdmMode18MbpsCS10MHz},
        {false, &Ieee80211OFDMCompliantModes::ofdmMode24MbpsCS10MHz},
        {false, &Ieee80211OFDMCompliantModes::ofdmMode27Mbps},
    }),
};

Ieee80211ModeSet::Ieee80211ModeSet(char name, const std::vector<Entry> entries) :
    name(name),
    entries(entries)
{
}

int Ieee80211ModeSet::getModeIndex(const IIeee80211Mode *mode) const
{
    for (int index = 0; index < (int)entries.size(); index++) {
        if (entries[index].mode == mode)
            return index;
    }
    return -1;
}

const IIeee80211Mode *Ieee80211ModeSet::getMode(bps bitrate) const
{
    for (int index = 0; index < (int)entries.size(); index++) {
        const IIeee80211Mode *mode = entries[index].mode;
        if (mode->getDataMode()->getNetBitrate() == bitrate)
            return entries[index].mode;
    }
    return nullptr;
}

const IIeee80211Mode *Ieee80211ModeSet::getSlowestMode() const
{
    return entries.front().mode;
}

const IIeee80211Mode *Ieee80211ModeSet::getFastestMode() const
{
    return entries.back().mode;
}

const IIeee80211Mode *Ieee80211ModeSet::getSlowerMode(const IIeee80211Mode *mode) const
{
    int index = getModeIndex(mode);
    if (index > 0)
        return entries[index - 1].mode;
    else
        return nullptr;
}

const IIeee80211Mode *Ieee80211ModeSet::getFasterMode(const IIeee80211Mode *mode) const
{
    int index = getModeIndex(mode);
    if (index < (int)entries.size() - 1)
        return entries[index + 1].mode;
    else
        return nullptr;
}

const Ieee80211ModeSet *Ieee80211ModeSet::getModeSet(char mode) {
    for (int index = 0; index < (int)Ieee80211ModeSet::modeSets.size(); index++) {
        const Ieee80211ModeSet *modeSet = &Ieee80211ModeSet::modeSets[index];
        if (modeSet->getName() == mode)
            return modeSet;
    }
    return nullptr;
}

} // namespace physicallayer

} // namespace inet
