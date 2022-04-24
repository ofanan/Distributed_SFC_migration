//
// Generated file, do not edit! Created by opp_msgtool 6.0 from src/bottomUpPkt.msg.
//

#ifndef __BOTTOMUPPKT_M_H
#define __BOTTOMUPPKT_M_H

#if defined(__clang__)
#  pragma clang diagnostic ignored "-Wreserved-id-macro"
#endif
#include <omnetpp.h>

// opp_msgtool version check
#define MSGC_VERSION 0x0600
#if (MSGC_VERSION!=OMNETPP_VERSION)
#    error Version mismatch! Probably this file was generated by an earlier version of opp_msgtool: 'make clean' should help.
#endif

class bottomUpPkt;
// cplusplus {{
#pragma once
//#include "Chain.h"
// }}

/**
 * Class generated from <tt>src/bottomUpPkt.msg:10</tt> by opp_msgtool.
 * <pre>
 * packet bottomUpPkt
 * {
 * }
 * </pre>
 */
class bottomUpPkt : public ::omnetpp::cPacket
{
  protected:

  private:
    void copy(const bottomUpPkt& other);

  protected:
    bool operator==(const bottomUpPkt&) = delete;

  public:
    bottomUpPkt(const char *name=nullptr, short kind=0);
    bottomUpPkt(const bottomUpPkt& other);
    virtual ~bottomUpPkt();
    bottomUpPkt& operator=(const bottomUpPkt& other);
    virtual bottomUpPkt *dup() const override {return new bottomUpPkt(*this);}
    virtual void parsimPack(omnetpp::cCommBuffer *b) const override;
    virtual void parsimUnpack(omnetpp::cCommBuffer *b) override;
};

inline void doParsimPacking(omnetpp::cCommBuffer *b, const bottomUpPkt& obj) {obj.parsimPack(b);}
inline void doParsimUnpacking(omnetpp::cCommBuffer *b, bottomUpPkt& obj) {obj.parsimUnpack(b);}


namespace omnetpp {

template<> inline bottomUpPkt *fromAnyPtr(any_ptr ptr) { return check_and_cast<bottomUpPkt*>(ptr.get<cObject>()); }

}  // namespace omnetpp

#endif // ifndef __BOTTOMUPPKT_M_H

