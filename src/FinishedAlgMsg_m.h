//
// Generated file, do not edit! Created by nedtool 5.6 from FinishedAlgMsg.msg.
//

#ifndef __FINISHEDALGMSG_M_H
#define __FINISHEDALGMSG_M_H

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
 * Class generated from <tt>FinishedAlgMsg.msg:10</tt> by nedtool.
 * <pre>
 * message FinishedAlgMsg
 * {
 * }
 * </pre>
 */
class FinishedAlgMsg : public ::omnetpp::cMessage
{
  protected:

  private:
    void copy(const FinishedAlgMsg& other);

  protected:
    // protected and unimplemented operator==(), to prevent accidental usage
    bool operator==(const FinishedAlgMsg&);

  public:
    FinishedAlgMsg(const char *name=nullptr, short kind=0);
    FinishedAlgMsg(const FinishedAlgMsg& other);
    virtual ~FinishedAlgMsg();
    FinishedAlgMsg& operator=(const FinishedAlgMsg& other);
    virtual FinishedAlgMsg *dup() const override {return new FinishedAlgMsg(*this);}
    virtual void parsimPack(omnetpp::cCommBuffer *b) const override;
    virtual void parsimUnpack(omnetpp::cCommBuffer *b) override;

    // field getter/setter methods
};

inline void doParsimPacking(omnetpp::cCommBuffer *b, const FinishedAlgMsg& obj) {obj.parsimPack(b);}
inline void doParsimUnpacking(omnetpp::cCommBuffer *b, FinishedAlgMsg& obj) {obj.parsimUnpack(b);}


#endif // ifndef __FINISHEDALGMSG_M_H
