//
// Generated file, do not edit! Created by nedtool 5.6 from src/endXmtPkt.msg.
//

#ifndef __ENDXMTPKT_M_H
#define __ENDXMTPKT_M_H

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
 * Class generated from <tt>src/endXmtPkt.msg:3</tt> by nedtool.
 * <pre>
 * // indicates the end of xmting a pkt to a given port number
 * message endXmtPkt
 * {
 *     int16_t portNum;
 * }
 * </pre>
 */
class endXmtPkt : public ::omnetpp::cMessage
{
  protected:
    int16_t portNum;

  private:
    void copy(const endXmtPkt& other);

  protected:
    // protected and unimplemented operator==(), to prevent accidental usage
    bool operator==(const endXmtPkt&);

  public:
    endXmtPkt(const char *name=nullptr, short kind=0);
    endXmtPkt(const endXmtPkt& other);
    virtual ~endXmtPkt();
    endXmtPkt& operator=(const endXmtPkt& other);
    virtual endXmtPkt *dup() const override {return new endXmtPkt(*this);}
    virtual void parsimPack(omnetpp::cCommBuffer *b) const override;
    virtual void parsimUnpack(omnetpp::cCommBuffer *b) override;

    // field getter/setter methods
    virtual int16_t getPortNum() const;
    virtual void setPortNum(int16_t portNum);
};

inline void doParsimPacking(omnetpp::cCommBuffer *b, const endXmtPkt& obj) {obj.parsimPack(b);}
inline void doParsimUnpacking(omnetpp::cCommBuffer *b, endXmtPkt& obj) {obj.parsimUnpack(b);}


#endif // ifndef __ENDXMTPKT_M_H

