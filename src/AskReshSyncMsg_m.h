//
// Generated file, do not edit! Created by nedtool 5.6 from AskReshSyncMsg.msg.
//

#ifndef __ASKRESHSYNCMSG_M_H
#define __ASKRESHSYNCMSG_M_H

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
// }}

/**
 * Class generated from <tt>AskReshSyncMsg.msg:10</tt> by nedtool.
 * <pre>
 * message AskReshSyncMsg
 * {
 * }
 * </pre>
 */
class AskReshSyncMsg : public ::omnetpp::cMessage
{
  protected:

  private:
    void copy(const AskReshSyncMsg& other);

  protected:
    // protected and unimplemented operator==(), to prevent accidental usage
    bool operator==(const AskReshSyncMsg&);

  public:
    AskReshSyncMsg(const char *name=nullptr, short kind=0);
    AskReshSyncMsg(const AskReshSyncMsg& other);
    virtual ~AskReshSyncMsg();
    AskReshSyncMsg& operator=(const AskReshSyncMsg& other);
    virtual AskReshSyncMsg *dup() const override {return new AskReshSyncMsg(*this);}
    virtual void parsimPack(omnetpp::cCommBuffer *b) const override;
    virtual void parsimUnpack(omnetpp::cCommBuffer *b) override;

    // field getter/setter methods
};

inline void doParsimPacking(omnetpp::cCommBuffer *b, const AskReshSyncMsg& obj) {obj.parsimPack(b);}
inline void doParsimUnpacking(omnetpp::cCommBuffer *b, AskReshSyncMsg& obj) {obj.parsimUnpack(b);}


#endif // ifndef __ASKRESHSYNCMSG_M_H
