//
// Generated file, do not edit! Created by nedtool 5.6 from BottomUpPkt.msg.
//

// Disable warnings about unused variables, empty switch stmts, etc:
#ifdef _MSC_VER
#  pragma warning(disable:4101)
#  pragma warning(disable:4065)
#endif

#if defined(__clang__)
#  pragma clang diagnostic ignored "-Wshadow"
#  pragma clang diagnostic ignored "-Wconversion"
#  pragma clang diagnostic ignored "-Wunused-parameter"
#  pragma clang diagnostic ignored "-Wc++98-compat"
#  pragma clang diagnostic ignored "-Wunreachable-code-break"
#  pragma clang diagnostic ignored "-Wold-style-cast"
#elif defined(__GNUC__)
#  pragma GCC diagnostic ignored "-Wshadow"
#  pragma GCC diagnostic ignored "-Wconversion"
#  pragma GCC diagnostic ignored "-Wunused-parameter"
#  pragma GCC diagnostic ignored "-Wold-style-cast"
#  pragma GCC diagnostic ignored "-Wsuggest-attribute=noreturn"
#  pragma GCC diagnostic ignored "-Wfloat-conversion"
#endif

#include <iostream>
#include <sstream>
#include "BottomUpPkt_m.h"

namespace omnetpp {

// Template pack/unpack rules. They are declared *after* a1l type-specific pack functions for multiple reasons.
// They are in the omnetpp namespace, to allow them to be found by argument-dependent lookup via the cCommBuffer argument

// Packing/unpacking an std::vector
template<typename T, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::vector<T,A>& v)
{
    int n = v.size();
    doParsimPacking(buffer, n);
    for (int i = 0; i < n; i++)
        doParsimPacking(buffer, v[i]);
}

template<typename T, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::vector<T,A>& v)
{
    int n;
    doParsimUnpacking(buffer, n);
    v.resize(n);
    for (int i = 0; i < n; i++)
        doParsimUnpacking(buffer, v[i]);
}

// Packing/unpacking an std::list
template<typename T, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::list<T,A>& l)
{
    doParsimPacking(buffer, (int)l.size());
    for (typename std::list<T,A>::const_iterator it = l.begin(); it != l.end(); ++it)
        doParsimPacking(buffer, (T&)*it);
}

template<typename T, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::list<T,A>& l)
{
    int n;
    doParsimUnpacking(buffer, n);
    for (int i=0; i<n; i++) {
        l.push_back(T());
        doParsimUnpacking(buffer, l.back());
    }
}

// Packing/unpacking an std::set
template<typename T, typename Tr, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::set<T,Tr,A>& s)
{
    doParsimPacking(buffer, (int)s.size());
    for (typename std::set<T,Tr,A>::const_iterator it = s.begin(); it != s.end(); ++it)
        doParsimPacking(buffer, *it);
}

template<typename T, typename Tr, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::set<T,Tr,A>& s)
{
    int n;
    doParsimUnpacking(buffer, n);
    for (int i=0; i<n; i++) {
        T x;
        doParsimUnpacking(buffer, x);
        s.insert(x);
    }
}

// Packing/unpacking an std::map
template<typename K, typename V, typename Tr, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::map<K,V,Tr,A>& m)
{
    doParsimPacking(buffer, (int)m.size());
    for (typename std::map<K,V,Tr,A>::const_iterator it = m.begin(); it != m.end(); ++it) {
        doParsimPacking(buffer, it->first);
        doParsimPacking(buffer, it->second);
    }
}

template<typename K, typename V, typename Tr, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::map<K,V,Tr,A>& m)
{
    int n;
    doParsimUnpacking(buffer, n);
    for (int i=0; i<n; i++) {
        K k; V v;
        doParsimUnpacking(buffer, k);
        doParsimUnpacking(buffer, v);
        m[k] = v;
    }
}

// Default pack/unpack function for arrays
template<typename T>
void doParsimArrayPacking(omnetpp::cCommBuffer *b, const T *t, int n)
{
    for (int i = 0; i < n; i++)
        doParsimPacking(b, t[i]);
}

template<typename T>
void doParsimArrayUnpacking(omnetpp::cCommBuffer *b, T *t, int n)
{
    for (int i = 0; i < n; i++)
        doParsimUnpacking(b, t[i]);
}

// Default rule to prevent compiler from choosing base class' doParsimPacking() function
template<typename T>
void doParsimPacking(omnetpp::cCommBuffer *, const T& t)
{
    throw omnetpp::cRuntimeError("Parsim error: No doParsimPacking() function for type %s", omnetpp::opp_typename(typeid(t)));
}

template<typename T>
void doParsimUnpacking(omnetpp::cCommBuffer *, T& t)
{
    throw omnetpp::cRuntimeError("Parsim error: No doParsimUnpacking() function for type %s", omnetpp::opp_typename(typeid(t)));
}

}  // namespace omnetpp


// forward
template<typename T, typename A>
std::ostream& operator<<(std::ostream& out, const std::vector<T,A>& vec);

// Template rule which fires if a struct or class doesn't have operator<<
template<typename T>
inline std::ostream& operator<<(std::ostream& out,const T&) {return out;}

// operator<< for std::vector<T>
template<typename T, typename A>
inline std::ostream& operator<<(std::ostream& out, const std::vector<T,A>& vec)
{
    out.put('{');
    for(typename std::vector<T,A>::const_iterator it = vec.begin(); it != vec.end(); ++it)
    {
        if (it != vec.begin()) {
            out.put(','); out.put(' ');
        }
        out << *it;
    }
    out.put('}');
    
    char buf[32];
    sprintf(buf, " (size=%u)", (unsigned int)vec.size());
    out.write(buf, strlen(buf));
    return out;
}

Register_Class(BottomUpPkt)

BottomUpPkt::BottomUpPkt(const char *name, short kind) : ::omnetpp::cPacket(name,kind)
{
    notAssigned_arraysize = 0;
    this->notAssigned = 0;
    pushUpVec_arraysize = 0;
    this->pushUpVec = 0;
}

BottomUpPkt::BottomUpPkt(const BottomUpPkt& other) : ::omnetpp::cPacket(other)
{
    notAssigned_arraysize = 0;
    this->notAssigned = 0;
    pushUpVec_arraysize = 0;
    this->pushUpVec = 0;
    copy(other);
}

BottomUpPkt::~BottomUpPkt()
{
    delete [] this->notAssigned;
    delete [] this->pushUpVec;
}

BottomUpPkt& BottomUpPkt::operator=(const BottomUpPkt& other)
{
    if (this==&other) return *this;
    ::omnetpp::cPacket::operator=(other);
    copy(other);
    return *this;
}

void BottomUpPkt::copy(const BottomUpPkt& other)
{
    delete [] this->notAssigned;
    this->notAssigned = (other.notAssigned_arraysize==0) ? nullptr : new Chain[other.notAssigned_arraysize];
    notAssigned_arraysize = other.notAssigned_arraysize;
    for (unsigned int i=0; i<notAssigned_arraysize; i++)
        this->notAssigned[i] = other.notAssigned[i];
    delete [] this->pushUpVec;
    this->pushUpVec = (other.pushUpVec_arraysize==0) ? nullptr : new Chain[other.pushUpVec_arraysize];
    pushUpVec_arraysize = other.pushUpVec_arraysize;
    for (unsigned int i=0; i<pushUpVec_arraysize; i++)
        this->pushUpVec[i] = other.pushUpVec[i];
}

void BottomUpPkt::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::omnetpp::cPacket::parsimPack(b);
    b->pack(notAssigned_arraysize);
    doParsimArrayPacking(b,this->notAssigned,notAssigned_arraysize);
    b->pack(pushUpVec_arraysize);
    doParsimArrayPacking(b,this->pushUpVec,pushUpVec_arraysize);
}

void BottomUpPkt::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::omnetpp::cPacket::parsimUnpack(b);
    delete [] this->notAssigned;
    b->unpack(notAssigned_arraysize);
    if (notAssigned_arraysize==0) {
        this->notAssigned = 0;
    } else {
        this->notAssigned = new Chain[notAssigned_arraysize];
        doParsimArrayUnpacking(b,this->notAssigned,notAssigned_arraysize);
    }
    delete [] this->pushUpVec;
    b->unpack(pushUpVec_arraysize);
    if (pushUpVec_arraysize==0) {
        this->pushUpVec = 0;
    } else {
        this->pushUpVec = new Chain[pushUpVec_arraysize];
        doParsimArrayUnpacking(b,this->pushUpVec,pushUpVec_arraysize);
    }
}

void BottomUpPkt::setNotAssignedArraySize(unsigned int size)
{
    Chain *notAssigned2 = (size==0) ? nullptr : new Chain[size];
    unsigned int sz = notAssigned_arraysize < size ? notAssigned_arraysize : size;
    for (unsigned int i=0; i<sz; i++)
        notAssigned2[i] = this->notAssigned[i];
    notAssigned_arraysize = size;
    delete [] this->notAssigned;
    this->notAssigned = notAssigned2;
}

unsigned int BottomUpPkt::getNotAssignedArraySize() const
{
    return notAssigned_arraysize;
}

Chain& BottomUpPkt::getNotAssigned(unsigned int k)
{
    if (k>=notAssigned_arraysize) throw omnetpp::cRuntimeError("Array of size %d indexed by %d", notAssigned_arraysize, k);
    return this->notAssigned[k];
}

void BottomUpPkt::setNotAssigned(unsigned int k, const Chain& notAssigned)
{
    if (k>=notAssigned_arraysize) throw omnetpp::cRuntimeError("Array of size %d indexed by %d", notAssigned_arraysize, k);
    this->notAssigned[k] = notAssigned;
}

void BottomUpPkt::setPushUpVecArraySize(unsigned int size)
{
    Chain *pushUpVec2 = (size==0) ? nullptr : new Chain[size];
    unsigned int sz = pushUpVec_arraysize < size ? pushUpVec_arraysize : size;
    for (unsigned int i=0; i<sz; i++)
        pushUpVec2[i] = this->pushUpVec[i];
    pushUpVec_arraysize = size;
    delete [] this->pushUpVec;
    this->pushUpVec = pushUpVec2;
}

unsigned int BottomUpPkt::getPushUpVecArraySize() const
{
    return pushUpVec_arraysize;
}

Chain& BottomUpPkt::getPushUpVec(unsigned int k)
{
    if (k>=pushUpVec_arraysize) throw omnetpp::cRuntimeError("Array of size %d indexed by %d", pushUpVec_arraysize, k);
    return this->pushUpVec[k];
}

void BottomUpPkt::setPushUpVec(unsigned int k, const Chain& pushUpVec)
{
    if (k>=pushUpVec_arraysize) throw omnetpp::cRuntimeError("Array of size %d indexed by %d", pushUpVec_arraysize, k);
    this->pushUpVec[k] = pushUpVec;
}

class BottomUpPktDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertynames;
  public:
    BottomUpPktDescriptor();
    virtual ~BottomUpPktDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyname) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyname) const override;
    virtual int getFieldArraySize(void *object, int field) const override;

    virtual const char *getFieldDynamicTypeString(void *object, int field, int i) const override;
    virtual std::string getFieldValueAsString(void *object, int field, int i) const override;
    virtual bool setFieldValueAsString(void *object, int field, int i, const char *value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual void *getFieldStructValuePointer(void *object, int field, int i) const override;
};

Register_ClassDescriptor(BottomUpPktDescriptor)

BottomUpPktDescriptor::BottomUpPktDescriptor() : omnetpp::cClassDescriptor("BottomUpPkt", "omnetpp::cPacket")
{
    propertynames = nullptr;
}

BottomUpPktDescriptor::~BottomUpPktDescriptor()
{
    delete[] propertynames;
}

bool BottomUpPktDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<BottomUpPkt *>(obj)!=nullptr;
}

const char **BottomUpPktDescriptor::getPropertyNames() const
{
    if (!propertynames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
        const char **basenames = basedesc ? basedesc->getPropertyNames() : nullptr;
        propertynames = mergeLists(basenames, names);
    }
    return propertynames;
}

const char *BottomUpPktDescriptor::getProperty(const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : nullptr;
}

int BottomUpPktDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 2+basedesc->getFieldCount() : 2;
}

unsigned int BottomUpPktDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeFlags(field);
        field -= basedesc->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISARRAY | FD_ISCOMPOUND,
        FD_ISARRAY | FD_ISCOMPOUND,
    };
    return (field>=0 && field<2) ? fieldTypeFlags[field] : 0;
}

const char *BottomUpPktDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldName(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldNames[] = {
        "notAssigned",
        "pushUpVec",
    };
    return (field>=0 && field<2) ? fieldNames[field] : nullptr;
}

int BottomUpPktDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount() : 0;
    if (fieldName[0]=='n' && strcmp(fieldName, "notAssigned")==0) return base+0;
    if (fieldName[0]=='p' && strcmp(fieldName, "pushUpVec")==0) return base+1;
    return basedesc ? basedesc->findField(fieldName) : -1;
}

const char *BottomUpPktDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeString(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "Chain",
        "Chain",
    };
    return (field>=0 && field<2) ? fieldTypeStrings[field] : nullptr;
}

const char **BottomUpPktDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldPropertyNames(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

const char *BottomUpPktDescriptor::getFieldProperty(int field, const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldProperty(field, propertyname);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

int BottomUpPktDescriptor::getFieldArraySize(void *object, int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount();
    }
    BottomUpPkt *pp = (BottomUpPkt *)object; (void)pp;
    switch (field) {
        case 0: return pp->getNotAssignedArraySize();
        case 1: return pp->getPushUpVecArraySize();
        default: return 0;
    }
}

const char *BottomUpPktDescriptor::getFieldDynamicTypeString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldDynamicTypeString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    BottomUpPkt *pp = (BottomUpPkt *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string BottomUpPktDescriptor::getFieldValueAsString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldValueAsString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    BottomUpPkt *pp = (BottomUpPkt *)object; (void)pp;
    switch (field) {
        case 0: {std::stringstream out; out << pp->getNotAssigned(i); return out.str();}
        case 1: {std::stringstream out; out << pp->getPushUpVec(i); return out.str();}
        default: return "";
    }
}

bool BottomUpPktDescriptor::setFieldValueAsString(void *object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->setFieldValueAsString(object,field,i,value);
        field -= basedesc->getFieldCount();
    }
    BottomUpPkt *pp = (BottomUpPkt *)object; (void)pp;
    switch (field) {
        default: return false;
    }
}

const char *BottomUpPktDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructName(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        case 0: return omnetpp::opp_typename(typeid(Chain));
        case 1: return omnetpp::opp_typename(typeid(Chain));
        default: return nullptr;
    };
}

void *BottomUpPktDescriptor::getFieldStructValuePointer(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructValuePointer(object, field, i);
        field -= basedesc->getFieldCount();
    }
    BottomUpPkt *pp = (BottomUpPkt *)object; (void)pp;
    switch (field) {
        case 0: return (void *)(&pp->getNotAssigned(i)); break;
        case 1: return (void *)(&pp->getPushUpVec(i)); break;
        default: return nullptr;
    }
}


