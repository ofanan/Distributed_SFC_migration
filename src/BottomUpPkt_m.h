//
// Generated file, do not edit! Created by nedtool 5.6 from BottomUpPkt.msg.
//

#ifndef __BOTTOMUPPKT_M_H
#define __BOTTOMUPPKT_M_H

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
 * Class generated from <tt>BottomUpPkt.msg:10</tt> by nedtool.
 * <pre>
 * packet BottomUpPkt
 * {
 *     Chain notAssigned[];
 *     Chain pushUpVec[];
 * }
 * </pre>
 */
class BottomUpPkt : public ::omnetpp::cPacket
{
  protected:
    Chain *notAssigned; // array ptr
    unsigned int notAssigned_arraysize;
    Chain *pushUpVec; // array ptr
    unsigned int pushUpVec_arraysize;

  private:
    void copy(const BottomUpPkt& other);

  protected:
    // protected and unimplemented operator==(), to prevent accidental usage
    bool operator==(const BottomUpPkt&);

  public:
    BottomUpPkt(const char *name=nullptr, short kind=0);
    BottomUpPkt(const BottomUpPkt& other);
    virtual ~BottomUpPkt();
    BottomUpPkt& operator=(const BottomUpPkt& other);
    virtual BottomUpPkt *dup() const override {return new BottomUpPkt(*this);}
    virtual void parsimPack(omnetpp::cCommBuffer *b) const override;
    virtual void parsimUnpack(omnetpp::cCommBuffer *b) override;

    // field getter/setter methods
    virtual void setNotAssignedArraySize(unsigned int size);
    virtual unsigned int getNotAssignedArraySize() const;
    virtual Chain& getNotAssigned(unsigned int k);
    virtual const Chain& getNotAssigned(unsigned int k) const {return const_cast<BottomUpPkt*>(this)->getNotAssigned(k);}
    virtual void setNotAssigned(unsigned int k, const Chain& notAssigned);
    virtual void setPushUpVecArraySize(unsigned int size);
    virtual unsigned int getPushUpVecArraySize() const;
    virtual Chain& getPushUpVec(unsigned int k);
    virtual const Chain& getPushUpVec(unsigned int k) const {return const_cast<BottomUpPkt*>(this)->getPushUpVec(k);}
    virtual void setPushUpVec(unsigned int k, const Chain& pushUpVec);
};

inline void doParsimPacking(omnetpp::cCommBuffer *b, const BottomUpPkt& obj) {obj.parsimPack(b);}
inline void doParsimUnpacking(omnetpp::cCommBuffer *b, BottomUpPkt& obj) {obj.parsimUnpack(b);}


#endif // ifndef __BOTTOMUPPKT_M_H

