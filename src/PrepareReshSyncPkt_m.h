//
// Generated file, do not edit! Created by nedtool 5.6 from PrepareReshSyncPkt.msg.
//

#ifndef __PREPARERESHSYNCPKT_M_H
#define __PREPARERESHSYNCPKT_M_H

#if defined(__clang__)
#  pragma clang diagnostic ignored "-Wreserved-id-macro"
#endif
#include <omnetpp.h>

// nedtool version check
#define MSGC_VERSION 0x0506
#if (MSGC_VERSION!=OMNETPP_VERSION)
#    error Version mismatch! Probably this file was generated by an earlier version of nedtool: 'make clean' should help.
#endif



/**
 * Class generated from <tt>PrepareReshSyncPkt.msg:3</tt> by nedtool.
 * <pre>
 * packet PrepareReshSyncPkt
 * {
 * }
 * </pre>
 */
class PrepareReshSyncPkt : public ::omnetpp::cPacket
{
  protected:

  private:
    void copy(const PrepareReshSyncPkt& other);

  protected:
    // protected and unimplemented operator==(), to prevent accidental usage
    bool operator==(const PrepareReshSyncPkt&);

  public:
    PrepareReshSyncPkt(const char *name=nullptr, short kind=0);
    PrepareReshSyncPkt(const PrepareReshSyncPkt& other);
    virtual ~PrepareReshSyncPkt();
    PrepareReshSyncPkt& operator=(const PrepareReshSyncPkt& other);
    virtual PrepareReshSyncPkt *dup() const override {return new PrepareReshSyncPkt(*this);}
    virtual void parsimPack(omnetpp::cCommBuffer *b) const override;
    virtual void parsimUnpack(omnetpp::cCommBuffer *b) override;

    // field getter/setter methods
};

inline void doParsimPacking(omnetpp::cCommBuffer *b, const PrepareReshSyncPkt& obj) {obj.parsimPack(b);}
inline void doParsimUnpacking(omnetpp::cCommBuffer *b, PrepareReshSyncPkt& obj) {obj.parsimUnpack(b);}


#endif // ifndef __PREPARERESHSYNCPKT_M_H
