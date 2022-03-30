//
// Generated file, do not edit! Created by nedtool 5.7 from src/pushUpPkt.msg.
//

#ifndef __PUSHUPPKT_M_H
#define __PUSHUPPKT_M_H

#if defined(__clang__)
#  pragma clang diagnostic ignored "-Wreserved-id-macro"
#endif
#include <omnetpp.h>

// nedtool version check
#define MSGC_VERSION 0x0507
#if (MSGC_VERSION!=OMNETPP_VERSION)
#    error Version mismatch! Probably this file was generated by an earlier version of nedtool: 'make clean' should help.
#endif



// cplusplus {{
#pragma once
#include "Chain.h"
// }}

/**
 * Class generated from <tt>src/pushUpPkt.msg:10</tt> by nedtool.
 * <pre>
 * packet pushUpPkt
 * {
 *     ChainList pushUpList;
 * }
 * </pre>
 */
class pushUpPkt : public ::omnetpp::cPacket
{
  protected:
    ChainList pushUpList;

  private:
    void copy(const pushUpPkt& other);

  protected:
    // protected and unimplemented operator==(), to prevent accidental usage
    bool operator==(const pushUpPkt&);

  public:
    pushUpPkt(const char *name=nullptr, short kind=0);
    pushUpPkt(const pushUpPkt& other);
    virtual ~pushUpPkt();
    pushUpPkt& operator=(const pushUpPkt& other);
    virtual pushUpPkt *dup() const override {return new pushUpPkt(*this);}
    virtual void parsimPack(omnetpp::cCommBuffer *b) const override;
    virtual void parsimUnpack(omnetpp::cCommBuffer *b) override;

    // field getter/setter methods
    virtual ChainList& getPushUpList();
    virtual const ChainList& getPushUpList() const {return const_cast<pushUpPkt*>(this)->getPushUpList();}
    virtual void setPushUpList(const ChainList& pushUpList);
};

inline void doParsimPacking(omnetpp::cCommBuffer *b, const pushUpPkt& obj) {obj.parsimPack(b);}
inline void doParsimUnpacking(omnetpp::cCommBuffer *b, pushUpPkt& obj) {obj.parsimUnpack(b);}


#endif // ifndef __PUSHUPPKT_M_H
