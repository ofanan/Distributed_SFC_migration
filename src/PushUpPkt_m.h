//
// Generated file, do not edit! Created by nedtool 5.6 from PushUpPkt.msg.
//

#ifndef __PUSHUPPKT_M_H
#define __PUSHUPPKT_M_H

#if defined(__clang__)
#  pragma clang diagnostic ignored "-Wreserved-id-macro"
#endif
#include <omnetpp.h>

// nedtool version check
#define MSGC_VERSION 0x0506
#if (MSGC_VERSION!=OMNETPP_VERSION)
#    error Version mismatch! Probably this file was generated by an earlier version of nedtool: 'make clean' should help.
#endif



// cplusplus {{
#pragma once
#include "Chain.h"
// }}

/**
 * Class generated from <tt>PushUpPkt.msg:9</tt> by nedtool.
 * <pre>
 * packet PushUpPkt
 * {
 *     Chain pushUpVec[];
 * }
 * </pre>
 */
class PushUpPkt : public ::omnetpp::cPacket
{
  protected:
    Chain *pushUpVec; // array ptr
    unsigned int pushUpVec_arraysize;

  private:
    void copy(const PushUpPkt& other);

  protected:
    // protected and unimplemented operator==(), to prevent accidental usage
    bool operator==(const PushUpPkt&);

  public:
    PushUpPkt(const char *name=nullptr, short kind=0);
    PushUpPkt(const PushUpPkt& other);
    virtual ~PushUpPkt();
    PushUpPkt& operator=(const PushUpPkt& other);
    virtual PushUpPkt *dup() const override {return new PushUpPkt(*this);}
    virtual void parsimPack(omnetpp::cCommBuffer *b) const override;
    virtual void parsimUnpack(omnetpp::cCommBuffer *b) override;

    // field getter/setter methods
    virtual void setPushUpVecArraySize(unsigned int size);
    virtual unsigned int getPushUpVecArraySize() const;
    virtual Chain& getPushUpVec(unsigned int k);
    virtual const Chain& getPushUpVec(unsigned int k) const {return const_cast<PushUpPkt*>(this)->getPushUpVec(k);}
    virtual void setPushUpVec(unsigned int k, const Chain& pushUpVec);
};

inline void doParsimPacking(omnetpp::cCommBuffer *b, const PushUpPkt& obj) {obj.parsimPack(b);}
inline void doParsimUnpacking(omnetpp::cCommBuffer *b, PushUpPkt& obj) {obj.parsimUnpack(b);}


#endif // ifndef __PUSHUPPKT_M_H
