//
// Copyright (C) 2015 OpenSim Ltd.
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

#include "Ieee80211MacTxCoordinationSta.h"
#include "inet/linklayer/ieee80211/thenewmac/signals/Ieee80211MacSignals_m.h"
#include "inet/common/ModuleAccess.h"
#include "inet/linklayer/ieee80211/thenewmac/Ieee80211NewFrame_m.h"

namespace inet {
namespace ieee80211 {

Define_Module(Ieee80211MacTxCoordinationSta);

void Ieee80211MacTxCoordinationSta::handleMessage(cMessage* msg)
{
    if (msg->isSelfMessage())
    {
        if (strcmp("ResetMAC", msg->getName()) == 0)
            handleResetMac();
        if (strcmp("Tifs timer", msg->getName()) == 0)
            handleTifs();
        else if (strcmp("Trsp timer", msg->getName()) == 0)
            handleTrsp();
        else if (strcmp("Tpdly timer", msg->getName()) == 0)
            handleTpdly();
        else
            throw cRuntimeError("Unknown self message", msg->getName());
    }
    else
    {
        if (dynamic_cast<Ieee80211MacSignalPduRequest *>(msg))
            handlePduRequest(dynamic_cast<Ieee80211MacSignalPduRequest *>(msg));
        else if (dynamic_cast<Ieee80211MacSignalCfPoll *>(msg))
            handleCfPoll(dynamic_cast<Ieee80211MacSignalCfPoll *>(msg));
        else if (dynamic_cast<Ieee80211MacSignalTbtt *>(msg))
            handleTbtt();
        else if (dynamic_cast<Ieee80211MacSignalBkDone *>(msg))
            handleBkDone(dynamic_cast<Ieee80211MacSignalBkDone *>(msg));
        else if (dynamic_cast<Ieee80211MacSignalCfEnd *>(msg))
            handleCfEnd();
        else if (dynamic_cast<Ieee80211MacSignalTxCfAck *>(msg))
            handleTxCfAck(dynamic_cast<Ieee80211MacSignalTxCfAck *>(msg));
        else if (dynamic_cast<Ieee80211MacSignalTxConfirm *>(msg))
            handleTxConfirm();
        else if (dynamic_cast<Ieee80211MacSignalAck *>(msg))
            handleAck(dynamic_cast<Ieee80211MacSignalAck *>(msg));
        else if (dynamic_cast<Ieee80211MacSignalCts *>(msg))
            handleCts(dynamic_cast<Ieee80211MacSignalCts *>(msg));
        else if (dynamic_cast<Ieee80211MacSignalMmCancel *>(msg))
            handleMmCancel();
        else if (dynamic_cast<Ieee80211MacSignalWake *>(msg))
            handleWake();
        else if (dynamic_cast<Ieee80211MacSignalDoze *>(msg))
            handleDoze();
        else if (dynamic_cast<Ieee80211MacSignalSwChnl *>(msg))
            handleSwChnl(dynamic_cast<Ieee80211MacSignalSwChnl *>(msg));
    }
}

void Ieee80211MacTxCoordinationSta::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
    {
        tifs = new cMessage("Tifs timer");
        trsp = new cMessage("Trsp timer");
        tpdly = new cMessage("Tpdly timer");
        macsorts = getModuleFromPar<Ieee80211MacMacsorts>(par("macsortsPackage"), this);
        macmib = getModuleFromPar<Ieee80211MacMacmibPackage>(par("macmibPackage"), this);
    }
    if (stage == INITSTAGE_LINK_LAYER)
    {
        backoffProcedureGate = this->gate("txOBackoff");
        dataPumpProcedureGate = this->gate("txODataPump");
        tmgtGate = this->gate("tmgt");
        tdatGate = this->gate("tdat");
        ccw = macmib->getPhyOperationTable()->getCWmin();
    }
}

void Ieee80211MacTxCoordinationSta::handleResetMac()
{
    dSifsDelay = macmib->getPhyOperationTable()->getSifsTime() - macmib->getPhyOperationTable()->getRxTxTurnaroundTime();
   // Mmrate must be selected from mBrates.
   // Other selection criteria are not specified.
   // TODO: 'mmrate:= rate to send mmpdus'
   ssrc = 0;
   slrc = 0;
   ccw = macmib->getPhyOperationTable()->getCWmin();
   atimcw = dcfcw = ccw;
   emitBackoff(ccw, -1);
   state = TX_COORDINATION_STATE_TXC_IDLE;
}

void Ieee80211MacTxCoordinationSta::txcReq()
{
    tpdu = fsdu->pdus[fsdu->fCur];
    // Test if fsdu seqnmber and tx lifetime set.
    if (fsdu->eol == SIMTIME_ZERO)
    {
        fsdu->sqf = seqnum;
        seqnum = seqnum == 4095 ? 0 : seqnum + 1;
        fsdu->eol = simTime() + macmib->getOperationTable()->getDot11MaxTransmitMsduLifetime();
    }
    tpdu->setSeq(fsdu->sqf);
    txcReq2();
}

void Ieee80211MacTxCoordinationSta::txcReq2()
{
    // See 9.6 for rules about selecting transmit data rate.
    // TODO: txrate = 'selected tx data rate' --
    // TxTime(sAckCtsLng/8, txrate, ackctstime);
    // TODO: TxTime(length(fsdu->pdus[fsdu->fCur+1]), txrate, frametime);
    // With FH PHY, if next fragment will be after a dwell boundary, Duration/ID may be set to
    // one ACK time plus SIFS time.
    tpdu->setDurId(macmib->getPhyOperationTable()->getSifsTime() + ackctstime + fsdu->fTot == fsdu->fCur + 1 ? 0 : 2 * macmib->getPhyOperationTable()->getSifsTime() + ackctstime + frametime);
    tpdu->setPwrMgt(macmib->getStationConfigTable()->getDot11PowerMangementMode());
    emitBackoff(0,0);
    chkRtsCts();
}


void Ieee80211MacTxCoordinationSta::chkRtsCts()
{
    if (useCtsToSelf())
    {
        // TxTime(length(tpdu,frametime2))
//         rtsdu = mkctl(cts, frametime2 + 2 * aSifstime + ackctstime);
        state = TX_COORDINATION_STATE_WAIT_CTS_BACKOFF;
    }
    else
    {
        if (useCtsRtsProtection())
        {
        }
        else
        {
            //            ((length
            //            (tpdu) +sCrcLng) > import(
            //                    dot11RtsThreshold)) and
            //                    (not fsdu!grpa) and ((fsdu!fCur=0)
            //                    or retry(tpdu) or (fsdu!resume))
            //sendMpdu();

             // TODO: 4. oldal
        }
    }
}

void Ieee80211MacTxCoordinationSta::handlePduRequest(Ieee80211MacSignalPduRequest *pduRequest)
{
    fsdu = new FragSdu(pduRequest->getFsdu());
    if (state == TX_COORDINATION_STATE_TXC_IDLE)
    {
        if (!macsorts->getIntraMacRemoteVariables()->isBkIp())
            txcReq();
    }
    else if (state == TX_COORDINATION_STATE_ATIM_WINDOW)
    {
        if (!macsorts->getIntraMacRemoteVariables()->isAtimW())
        {
            if (ftype(fsdu->pdus[0]) == TypeSubtype::TypeSubtype_beacon)
            {
                n = intuniform(0,2*macmib->getPhyOperationTable()->getCWmin());
                emitBackoff(n,-1);
                state = TX_COORDINATION_STATE_IBSS_WAIT_BEACON;
            }
            else
            {
                tpdu = new Ieee80211NewFrame();
                tpdu->setFtype(TypeSubtype_atim);
                tpdu->setAddr1(fsdu->dst);
                tpdu->setAddr2(macmib->getOperationTable()->getDot11MacAddress());
                emitBackoff(ccw, -1);
                state = TX_COORDINATION_STATE_IBSS_WAIT_ATIM;
                ibssAtimWBullshit();
            }
        }
    }
    else if (state == TX_COORDINATION_STATE_ASLEEP)
    {
        scheduleAt(simTime() + usecToSimtime(macsorts->getIntraMacRemoteVariables()->getPdly()), tpdly);
        state = TX_COORDINATION_STATE_WAKE_WAIT_PROBE_DELAY;
    }
    else if (state == TX_COORDINATION_STATE_CF_RESPONSE)
    {
        TypeSubtype pack = ftype(tpdu);
        tpdu = fsdu->pdus[fsdu->fCur];
        if (fsdu->eol == SIMTIME_ZERO)
        {
            fsdu->sqf = seqnum;
            seqnum = seqnum + 1;
            fsdu->eol = simTime() + macmib->getOperationTable()->getDot11MaxTransmitMsduLifetime();
        }
        tpdu->setSeq(fsdu->sqf);
        // Change data to data+cfAck if appropriate.
        tpdu->setFtype(TypeSubtype(tpdu->getFtype() | pack));
        // TODO: See 9.6 for rules about selecting transmit data rate.
        // 'txrate:= selected tx data rate'
        state = TX_COORDINATION_STATE_WAIT_CFP_SIFS;
    }
}

void Ieee80211MacTxCoordinationSta::handleTbtt()
{
    if (state == TX_COORDINATION_STATE_TXC_IDLE)
    {
        dcfcnt = -1;
        function1();
    }
    else if (state == TX_COORDINATION_STATE_WAIT_RTS_BACKOFF || state == TX_COORDINATION_STATE_WAIT_MPDU_BACKOFF)
    {
        emitPduConfirm(fsdu, TxResult::TxResult_partial);
        emitCancel();
        state = TX_COORDINATION_STATE_ATW_START;
    }
    else if (state == TX_COORDINATION_STATE_WAIT_CTS_BACKOFF)
    {
        emitPduConfirm(fsdu, TxResult::TxResult_partial);
        emitCancel();
        state = TX_COORDINATION_STATE_ATW_START;
    }
    else if (state == TX_COORDINATION_STATE_TXC_BACKOFF)
    {
        if (macsorts->getIntraMacRemoteVariables()->isIbss())
        {
            emitCancel();
            dcfcw = ccw;
            ccw = atimcw;
            emitAtimW();
            state = TX_COORDINATION_STATE_ATIM_WINDOW;
        }
    }
}

void Ieee80211MacTxCoordinationSta::handleBkDone(Ieee80211MacSignalBkDone *bkDone)
{
    bstat = bkDone->getSlotCount();
    if (state == TX_COORDINATION_STATE_ATW_START)
    {
        dcfcnt = bkDone->getSlotCount();
        function1();
    }
    else if (state == TX_COORDINATION_STATE_WAIT_RTS_BACKOFF)
    {
        if (bstat == -2)
        {
            psmChg = curPm == macmib->getStationConfigTable()->getDot11PowerMangementMode() ? false : true;
        }
    }
    else if (state == TX_COORDINATION_STATE_WAIT_MPDU_BACKOFF)
    {
        if (bstat == -2)
        {
            emitTxRequest(tpdu, txrate);
            if (fsdu->grpa) // group addr
                endFx();
            else
                state = TX_COORDINATION_STATE_WAIT_PDU_SENT;
        }
        else
        {
            emitBackoff(ccw,-1);
            state = TX_COORDINATION_STATE_TXC_BACKOFF;
        }
    }
    else if (state == TX_COORDINATION_STATE_WAIT_CTS_BACKOFF)
    {
        if (bstat == -2)
        {
            emitBackoff(ccw, -1);
            state = TX_COORDINATION_STATE_TXC_BACKOFF;
        }
        else
        {
            psmChg = curPm == macmib->getStationConfigTable()->getDot11PowerMangementMode() ? false : true; // curPsm -- curPm
            macsorts->getIntraMacRemoteVariables()->setFxIp(true);
            macmib->getCountersTable()->incDot11TransmittedFragmentCount();
//            emitTxRequest(rtsdu, txrate);
            state = TX_COORDINATION_STATE_WAIT_CTS_SENT;
        }
    }
    else if (state == TX_COORDINATION_STATE_IBSS_WAIT_ATIM)
    {
        emitTxRequest(tpdu, mmrate);
        if (fsdu->grpa)
            atimAck();
        else
        {
//            dRsp:=dUsec(aSifsTime +calcDur(txrate, ackctstime))
//            set(now+dRsp,Trsp)
            state = TX_COORDINATION_STATE_WAIT_ATIM_ACK;
        }
    }
    else if (state == TX_COORDINATION_STATE_IBSS_WAIT_BEACON)
    {
        emitTxRequest(fsdu->pdus[0], mmrate);
        state = TX_COORDINATION_STATE_WAIT_BEACON_TRANSMIT;
    }
    else if (state == TX_COORDINATION_STATE_WAIT_BEACON_CANCEL)
    {
        n = bkDone->getSlotCount();
        state = TX_COORDINATION_STATE_ATIM_WINDOW;
    }
    else if (state == TX_COORDINATION_STATE_TXC_BACKOFF)
    {
        if (cont)
        {
            cont = false;
            sendFrag();
        }
        else
            state = TX_COORDINATION_STATE_TXC_IDLE;
    }
    else if (state == TX_COORDINATION_STATE_SW_CHNL_BACKOFF)
        state = macsorts->getIntraMacRemoteVariables()->isAtimW() ? TX_COORDINATION_STATE_ATIM_WINDOW : TX_COORDINATION_STATE_TXC_IDLE;
}

void Ieee80211MacTxCoordinationSta::function1()
{
    if (macsorts->getIntraMacRemoteVariables()->isIbss())
    {
        dcfcw = ccw;
        ccw = atimcw;
        emitAtimW();
        state = TX_COORDINATION_STATE_ATIM_WINDOW;
        atimWBullshit();
    }
}

void Ieee80211MacTxCoordinationSta::handleCfPoll(Ieee80211MacSignalCfPoll *cfPoll)
{
    endRx = cfPoll->getTime();
    if (state == TX_COORDINATION_STATE_TXC_IDLE || state == TX_COORDINATION_STATE_TXC_CFP)
    {
        tpdu = new Ieee80211NewFrame();
        tpdu->setFtype(TypeSubtype_null_frame);
        tpdu->setAddr1(macsorts->getIntraMacRemoteVariables()->getBssId());
        tpdu->setAddr2(macsorts->getIntraMacRemoteVariables()->getBssId());
        scheduleAt(endRx + dSifsDelay, trsp);
        emitCfPolled();
        state = TX_COORDINATION_STATE_CF_RESPONSE;
    }
}

void Ieee80211MacTxCoordinationSta::handleTxCfAck(Ieee80211MacSignalTxCfAck *txCfAck)
{
    // TODO: saved signals
    if (state == TX_COORDINATION_STATE_CF_RESPONSE)
    {
        endRx = txCfAck->getTime();
        tpdu = new Ieee80211NewFrame();
        tpdu->setFtype(TypeSubtype_cfack);
        tpdu->setAddr1(macsorts->getIntraMacRemoteVariables()->getBssId());
        tpdu->setAddr2(macsorts->getIntraMacRemoteVariables()->getBssId());
    }
    else if (state == TX_COORDINATION_STATE_TXC_CFP)
    {
        endRx = txCfAck->getTime();
        TypeSubtype rtpye = TypeSubtype::TypeSubtype_cfack;
        tpdu = new Ieee80211NewFrame();
        tpdu->setFtype(rtpye);
        tpdu->setAddr1(macsorts->getIntraMacRemoteVariables()->getBssId());
        tpdu->setAddr2(macsorts->getIntraMacRemoteVariables()->getBssId());
        txSifs();
    }
    else if (state == TX_COORDINATION_STATE_WAIT_CFP_SIFS)
        tpdu->setFtype(TypeSubtype::TypeSubtype_data_ack);
}

void Ieee80211MacTxCoordinationSta::txSifs()
{
    cMessage *tifs = new cMessage("Tifs");
    state = TX_COORDINATION_STATE_WAIT_SIFS;
    scheduleAt(endRx + dSifsDelay, tifs);
    emitTxRequest(tpdu, txrate);
    tpdu->setPwrMgt(curPm);
}

void Ieee80211MacTxCoordinationSta::handleTifs()
{
    if (state == TX_COORDINATION_STATE_WAIT_CTS_SIFS)
        sendTxReq();
}

void Ieee80211MacTxCoordinationSta::txWait()
{
}

void Ieee80211MacTxCoordinationSta::sendFrag()
{
    tpdu = fsdu->pdus[fsdu->fCur];
    txcReq2();
}

void Ieee80211MacTxCoordinationSta::sendRts()
{
    state = TX_COORDINATION_STATE_WAIT_RTS_BACKOFF;
}

void Ieee80211MacTxCoordinationSta::emitAtimW()
{
}

void Ieee80211MacTxCoordinationSta::emitTxRequest(cPacket* tpdu, bps txrate)
{
    Ieee80211MacSignalTxRequest *txRequest = new Ieee80211MacSignalTxRequest("TxRequest");
    txRequest->encapsulate(tpdu);
    txRequest->setTxrate(txrate.get());
    send(txRequest, dataPumpProcedureGate);
}

void Ieee80211MacTxCoordinationSta::emitBackoff(int ccw, int par2)
{
    cMessage *backoffSignal = new cMessage("Backoff");
    backoffSignal->addPar("ccw") = ccw;
    backoffSignal->addPar("par2") = par2;
    send(backoffSignal, backoffProcedureGate);
}

void Ieee80211MacTxCoordinationSta::emitPduConfirm(FragSdu* fsdu, TxResult txResult)
{
    Ieee80211MacSignalPduConfirm *pduConfirm = new Ieee80211MacSignalPduConfirm("PduConfirm");
    pduConfirm->setFsdu(*fsdu);
    pduConfirm->setTxResult(txResult);
    send(pduConfirm, tdatGate);
}

void Ieee80211MacTxCoordinationSta::sendMpdu()
{
    state = TX_COORDINATION_STATE_WAIT_MPDU_BACKOFF;
}

void Ieee80211MacTxCoordinationSta::handleTxConfirm()
{
    if (state == TX_COORDINATION_STATE_WAIT_PDU_SENT)
    {
//        scheduleAt(simTime()+usecToSimtime((macmib->getPhyOperationTable()->getSifsTime() + calcDur(txrate,stuff(aMpduDurationFactor, sAckCtsLng))+ aPlcpHeaderLength+aPreambleLength+aSlotTime)), trsp);
        state = TX_COORDINATION_STATE_WAIT_ACK;
    }
    else if (state == TX_COORDINATION_STATE_WAIT_CTS_SENT)
    {
        scheduleAt(simTime() + usecToSimtime(macmib->getPhyOperationTable()->getSifsTime()), tifs); // Tsifs??
        state = TX_COORDINATION_STATE_WAIT_CTS_SIFS;
    }
    else if (state == TX_COORDINATION_STATE_WAIT_BEACON_TRANSMIT)
        state = TX_COORDINATION_STATE_ATIM_WINDOW;
    else if (state == TX_COORDINATION_STATE_WAIT_CFP_TX_DONE)
    {
        scheduleAt(simTime() + usecToSimtime(macmib->getPhyOperationTable()->getSifsTime()), trsp);
        state = TX_COORDINATION_STATE_WAIT_CF_ACK;
    }
}

void Ieee80211MacTxCoordinationSta::endFx()
{
    if (psmChg)
        emitPsmDone();
    macsorts->getIntraMacRemoteVariables()->setFxIp(false);
    psmChg = false;
    ccw = macmib->getPhyOperationTable()->getCWmin();
    confirmPdu();
}

void Ieee80211MacTxCoordinationSta::confirmPdu()
{
    slrc = 0;
    ssrc = 0;
    fsdu->lrc = 0;
    fsdu->src = 0;
    macmib->getCountersTable()->incDot11MulticastTransmittedFrameCount();
    if (fsdu->grpa || (tpdu->getToDs() && tpdu->getAddr3().isMulticast() && fsdu->fTot == fsdu->fCur + 1))
        macmib->getCountersTable()->incDot11MulticastTransmittedFrameCount();
    if (fsdu->fTot == fsdu->fCur + 1)
    {
        macmib->getCountersTable()->incDot11TransmittedFragmentCount();
        emitPduConfirm(fsdu, TxResult::TxResult_successful);
        emitBackoff(ccw, -1);
        state = TX_COORDINATION_STATE_TXC_BACKOFF;
    }
    else
    {
        if (fsdu->eol < simTime())
        {
            emitPduConfirm(fsdu, TxResult::TxResult_txLifetime);
            state = TX_COORDINATION_STATE_TXC_IDLE;
        }
        else
        {
            fsdu->fCur++;
            sendFrag();
        }
    }
}

void Ieee80211MacTxCoordinationSta::handleAck(Ieee80211MacSignalAck *ack)
{
    endRx = ack->getEndRx();
    txrate = bps(ack->getTxRate());
    if (state == TX_COORDINATION_STATE_WAIT_ACK)
        endFx();
    else if (state == TX_COORDINATION_STATE_WAIT_ATIM_ACK)
    {
        ccw = macmib->getPhyOperationTable()->getCWmin();
        atimAck();
    }
    else if (state == TX_COORDINATION_STATE_WAIT_CF_ACK)
    {
        cancelEvent(trsp);
        if (fsdu->fTot == fsdu->fCur + 1)
        {
            emitPduConfirm(fsdu, TxResult::TxResult_successful);
            macmib->getCountersTable()->incDot11TransmittedFragmentCount();
        }
        else
        {
            if (fsdu->eol < simTime())
                emitPduConfirm(fsdu, TxResult::TxResult_txLifetime);
            else
                fsdu->fCur++;
        }
        state = TX_COORDINATION_STATE_TXC_CFP;
    }
}

void Ieee80211MacTxCoordinationSta::handleTrsp()
{
    if (state == TX_COORDINATION_STATE_WAIT_ACK)
    {
        macmib->getCountersTable()->incDot11AckFailureCount();
        ackFail();
    }
    else if (state == TX_COORDINATION_STATE_WAIT_CTS)
    {
        macmib->getCountersTable()->incDot11RtsFailureCount();
        ctsFail();
    }
    else if (state == TX_COORDINATION_STATE_WAIT_ATIM_ACK)
    {
        ccw = ccw == macmib->getPhyOperationTable()->getCWmax() ? macmib->getPhyOperationTable()->getCWmax(): 2*ccw + 1;
        ssrc++;
        fsdu->src++;
        if (ssrc == macmib->getOperationTable()->getDot11ShortRetryLimit())
            ccw = macmib->getPhyOperationTable()->getCWmin();
        if (fsdu->src == macmib->getOperationTable()->getDot11ShortRetryLimit())
        {
            atimLimit();
        }
        else
            atimFail();
    }
    else if (state == TX_COORDINATION_STATE_WAIT_CFP_SIFS)
    {
        emitTxRequest(tpdu, txrate);
        macmib->getCountersTable()->incDot11TransmittedFragmentCount();
        //dot11MulticastTransmittedFrameCount,
//        if (fsdu!grpa or
//        (( toDs(tpdu) = 1)
//        and (isGrp(addr3(tpdu)))
//        and (fsdu!fTot=fsdu!fCur+1)))
//        then inc(cTmcfrm)
//        else cTmcfrm f
//        macmib->getCountersTable()->setDot11MulticastTransmittedFrameCount();
        state = TX_COORDINATION_STATE_WAIT_CFP_TX_DONE;
    }
    else if (state == TX_COORDINATION_STATE_WAIT_CF_ACK)
    {
        macmib->getCountersTable()->incDot11AckFailureCount();
        tpdu->setRetryBit(true);
        fsdu->pdus[fsdu->fCur]->setRetryBit(1);
        fsdu->src++;
        if (fsdu->src == macmib->getOperationTable()->getDot11LongRetryLimit())
        {
            emitPduConfirm(fsdu, TxResult::TxResult_retryLimit);
            macmib->getCountersTable()->incDot11FailedCount();
        }
        else
        {
            emitPduConfirm(fsdu, TxResult::TxResult_partial);
            macmib->getCountersTable()->incDot11RetryCount();
        }
        state = TX_COORDINATION_STATE_TXC_CFP;
    }
}

void Ieee80211MacTxCoordinationSta::emitCancel()
{
    Ieee80211MacSignalCancel *cancel = new Ieee80211MacSignalCancel("Cancel");
    send(cancel, backoffProcedureGate);
}

void Ieee80211MacTxCoordinationSta::ackFail()
{
    macsorts->getIntraMacRemoteVariables()->setFxIp(false);
    ccw = ccw == macmib->getPhyOperationTable()->getCWmax() ? macmib->getPhyOperationTable()->getCWmax() : 2*ccw + 1;
    emitBackoff(ccw, -1);
    tpdu->setRetryBit(true);
    fsdu->pdus[fsdu->fCur]->setRetryBit(true);
    if (/*length(tpdu) + sCrcLng > macmib->getOperationTable()->getDot11RtsThreshold()*/1)
    {
        ctsFail2();
    }
    else
    {
        ssrc++;
        fsdu->src++;
        if (ssrc == macmib->getOperationTable()->getDot11ShortRetryLimit())
        {
            ccw = macmib->getPhyOperationTable()->getCWmin();
        }
        if (fsdu->src == macmib->getOperationTable()->getDot11ShortRetryLimit())
        {
            emitPduConfirm(fsdu, TxResult::TxResult_retryLimit);
            macmib->getCountersTable()->getDot11FailedCount();
            cont = false;
        }
        else
            cont = true;
        state = TX_COORDINATION_STATE_TXC_BACKOFF;
    }
}


void Ieee80211MacTxCoordinationSta::ctsFail()
{
    macsorts->getIntraMacRemoteVariables()->setFxIp(false);
    ccw = ccw == macmib->getPhyOperationTable()->getCWmax() ? macmib->getPhyOperationTable()->getCWmax() : 2*ccw + 1;
    emitBackoff(ccw, -1);
}

void Ieee80211MacTxCoordinationSta::ctsFail2() // rename
{
    slrc++;
    fsdu->lrc++;
    if (slrc == macmib->getOperationTable()->getDot11LongRetryLimit())
        ccw = macmib->getPhyOperationTable()->getCWmin();
    if (fsdu->lrc == macmib->getOperationTable()->getDot11LongRetryLimit())
    {
        emitPduConfirm(fsdu, TxResult::TxResult_retryLimit);
        macmib->getCountersTable()->getDot11FailedCount();
        cont = false;
    }
    else
        cont = true;
    state = TX_COORDINATION_STATE_TXC_BACKOFF;
}

void Ieee80211MacTxCoordinationSta::handleCts(Ieee80211MacSignalCts *cts)
{
    endRx = cts->getEndRx();
    txrate = bps(cts->getTxRate());
    cancelEvent(trsp);
    ssrc = 0;
    fsdu->src = 0;
    macmib->getCountersTable()->incDot11RtsSuccessCount();
    scheduleAt(endRx + dSifsDelay, tifs);
//    tpdu:=
//    setDurId(tpdu,(aSifsTime + (calcDur
//            (txrate,stuff(aMpdu_
//            DurationFactor,sAck_
//            CtsLng))+aPlcpHeader_
//            Length+aPreambleLength)
//            + if (fsdu!fTot = (fsdu!
//            fCur+1)) then 0 else
//            ((2*aSifsTime)+(calcDur
//            (txrate,stuff(aMpdu_
//            DurationFactor,sAck_
//            CtsLng)) + aPlcpHeader_
//            Length + aPreambleLen_
//            gth)+stuff(aMpduDuration_
//            Factor,((length(fsdu!pdus
//            (fsdu!fCur+1))+sCrcLng)
//            *8)) + aPlcpHeaderLength
//            + aPreambleLength) )))
    state = TX_COORDINATION_STATE_WAIT_CTS_SIFS;
}

void Ieee80211MacTxCoordinationSta::sendTxReq()
{

}

TypeSubtype Ieee80211MacTxCoordinationSta::ftype(Ieee80211NewFrame *packet)
{
    return packet->getFtype();
}

void Ieee80211MacTxCoordinationSta::atimWBullshit()
{
    if (!macsorts->getIntraMacRemoteVariables()->isAtimW())
    {
        atimcw = ccw;
        ccw = dcfcw;
        if (dcfcnt >= 0)
        {
            emitBackoff(ccw, dcfcnt);
            cont = true;
            state = TX_COORDINATION_STATE_TXC_BACKOFF;
        }
        else
        {
            cont = false;
            state = TX_COORDINATION_STATE_TXC_IDLE;
        }
    }
}

void Ieee80211MacTxCoordinationSta::ibssAtimWBullshit()
{
    atimcw = ccw;
    ccw = dcfcw;
    state = TX_COORDINATION_STATE_TXC_IDLE;
}

void Ieee80211MacTxCoordinationSta::atimAck()
{
    emitPduConfirm(fsdu, TxResult::TxResult_atimAck);
    fsdu->fAnc = fsdu->fCur + 1;
    state = TX_COORDINATION_STATE_ATIM_WINDOW;
}

void Ieee80211MacTxCoordinationSta::handleMmCancel()
{
    if (state == TX_COORDINATION_STATE_IBSS_WAIT_BEACON)
    {
        emitCancel();
        state = TX_COORDINATION_STATE_WAIT_BEACON_CANCEL;
    }
}

void Ieee80211MacTxCoordinationSta::atimLimit()
{
    emitPduConfirm(fsdu, TxResult::TxResult_retryLimit);
    state = TX_COORDINATION_STATE_ATIM_WINDOW;
}

void Ieee80211MacTxCoordinationSta::atimFail()
{
    emitPduConfirm(fsdu, TxResult::TxResult_atimNak);
    state = TX_COORDINATION_STATE_ATIM_WINDOW;
}

void Ieee80211MacTxCoordinationSta::handleDoze()
{
    // In all states
    // TODO: 'turn off stuff to save power'
    // 'PlmeSet.request (doze stuff)'
    state = TX_COORDINATION_STATE_ASLEEP;
}

void Ieee80211MacTxCoordinationSta::handleWake()
{
    if (state == TX_COORDINATION_STATE_ASLEEP)
    {
        // TODO: 'turn on stuff that was off'
        // 'PlmeSet.request(wake stuff)'
        emitChangeNav(0, NavSrc::NavSrc_cswitch);
        state = TX_COORDINATION_STATE_TXC_IDLE;
    }
}

void Ieee80211MacTxCoordinationSta::emitPsmDone()
{
    cMessage *psmDone = new cMessage("PsmDone");
    send(psmDone, tmgtGate);
}

void Ieee80211MacTxCoordinationSta::handleTpdly()
{
    if (state == TX_COORDINATION_STATE_WAKE_WAIT_PROBE_DELAY)
    txcReq();
}

void Ieee80211MacTxCoordinationSta::handleSwChnl(Ieee80211MacSignalSwChnl *swChnl)
{
    chan = swChnl->getChan();
    doBkoff = swChnl->getDoBkoff();
    emitChangeNav(0, NavSrc::NavSrc_cswitch);
    // 'channel change is Phy-specific'
    // TODO: PlmeSet.request(chan stuff)'
    state = TX_COORDINATION_STATE_WAIT_CHANNEL;
}

void Ieee80211MacTxCoordinationSta::emitChangeNav(int par1, NavSrc navSrc)
{
}

void Ieee80211MacTxCoordinationSta::handleCfEnd()
{
}

void Ieee80211MacTxCoordinationSta::continousSignalTxCIdle()
{
    state = TX_COORDINATION_STATE_TXC_CFP;
}

bool Ieee80211MacTxCoordinationSta::useCtsRtsProtection()
{
    return false;
}

bool Ieee80211MacTxCoordinationSta::useCtsToSelf()
{
    return false;
}

void Ieee80211MacTxCoordinationSta::emitCfPolled()
{
}

void Ieee80211MacTxCoordinationSta::receiveSignal(cComponent* source, int signalID, cObject* obj)
{
    Enter_Method_Silent();
    printNotificationBanner(signalID, obj);
    if (signalID == Ieee80211MacMacsorts::intraMacRemoteVariablesChanged)
    {
        switch (state)
        {
            case TX_COORDINATION_STATE_TXC_IDLE:
                if (macsorts->getIntraMacRemoteVariables()->isCfp())
                    continousSignalTxCIdle();
                break;
        }
    }
}

void Ieee80211MacTxCoordinationSta::handlePlmeSetConfirm(Ieee80211MacSignalPlmeSetConfirm* plmeSetConfirm)
{
    if (state == TX_COORDINATION_STATE_WAIT_CHANNEL)
    {
        emitSwDone();
        if (doBkoff)
        {
            emitBackoff(ccw, -1);
            state = TX_COORDINATION_STATE_SW_CHNL_BACKOFF;
        }
        else {
            state = macsorts->getIntraMacRemoteVariables()->isAtimW() ? TX_COORDINATION_STATE_ATIM_WINDOW : TX_COORDINATION_STATE_TXC_IDLE;
        }
    }
}

void Ieee80211MacTxCoordinationSta::emitSwDone()
{
    Ieee80211MacSignalSwDone *swDone = new Ieee80211MacSignalSwDone();
    send(swDone, tmgtGate);
}

} /* namespace inet */
} /* namespace ieee80211 */