//
// Generated file, do not edit! Created by nedtool 5.6 from src/PlacementInfoMsg.msg.
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
#include "PlacementInfoMsg_m.h"

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

Register_Class(PlacementInfoMsg)

PlacementInfoMsg::PlacementInfoMsg(const char *name, short kind) : ::omnetpp::cMessage(name,kind)
{
    newlyPlacedChainsIds_arraysize = 0;
    this->newlyPlacedChainsIds = 0;
}

PlacementInfoMsg::PlacementInfoMsg(const PlacementInfoMsg& other) : ::omnetpp::cMessage(other)
{
    newlyPlacedChainsIds_arraysize = 0;
    this->newlyPlacedChainsIds = 0;
    copy(other);
}

PlacementInfoMsg::~PlacementInfoMsg()
{
    delete [] this->newlyPlacedChainsIds;
}

PlacementInfoMsg& PlacementInfoMsg::operator=(const PlacementInfoMsg& other)
{
    if (this==&other) return *this;
    ::omnetpp::cMessage::operator=(other);
    copy(other);
    return *this;
}

void PlacementInfoMsg::copy(const PlacementInfoMsg& other)
{
    delete [] this->newlyPlacedChainsIds;
    this->newlyPlacedChainsIds = (other.newlyPlacedChainsIds_arraysize==0) ? nullptr : new uint32_t[other.newlyPlacedChainsIds_arraysize];
    newlyPlacedChainsIds_arraysize = other.newlyPlacedChainsIds_arraysize;
    for (unsigned int i=0; i<newlyPlacedChainsIds_arraysize; i++)
        this->newlyPlacedChainsIds[i] = other.newlyPlacedChainsIds[i];
}

void PlacementInfoMsg::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::omnetpp::cMessage::parsimPack(b);
    b->pack(newlyPlacedChainsIds_arraysize);
    doParsimArrayPacking(b,this->newlyPlacedChainsIds,newlyPlacedChainsIds_arraysize);
}

void PlacementInfoMsg::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::omnetpp::cMessage::parsimUnpack(b);
    delete [] this->newlyPlacedChainsIds;
    b->unpack(newlyPlacedChainsIds_arraysize);
    if (newlyPlacedChainsIds_arraysize==0) {
        this->newlyPlacedChainsIds = 0;
    } else {
        this->newlyPlacedChainsIds = new uint32_t[newlyPlacedChainsIds_arraysize];
        doParsimArrayUnpacking(b,this->newlyPlacedChainsIds,newlyPlacedChainsIds_arraysize);
    }
}

void PlacementInfoMsg::setNewlyPlacedChainsIdsArraySize(unsigned int size)
{
    uint32_t *newlyPlacedChainsIds2 = (size==0) ? nullptr : new uint32_t[size];
    unsigned int sz = newlyPlacedChainsIds_arraysize < size ? newlyPlacedChainsIds_arraysize : size;
    for (unsigned int i=0; i<sz; i++)
        newlyPlacedChainsIds2[i] = this->newlyPlacedChainsIds[i];
    for (unsigned int i=sz; i<size; i++)
        newlyPlacedChainsIds2[i] = 0;
    newlyPlacedChainsIds_arraysize = size;
    delete [] this->newlyPlacedChainsIds;
    this->newlyPlacedChainsIds = newlyPlacedChainsIds2;
}

unsigned int PlacementInfoMsg::getNewlyPlacedChainsIdsArraySize() const
{
    return newlyPlacedChainsIds_arraysize;
}

uint32_t PlacementInfoMsg::getNewlyPlacedChainsIds(unsigned int k) const
{
    if (k>=newlyPlacedChainsIds_arraysize) throw omnetpp::cRuntimeError("Array of size %d indexed by %d", newlyPlacedChainsIds_arraysize, k);
    return this->newlyPlacedChainsIds[k];
}

void PlacementInfoMsg::setNewlyPlacedChainsIds(unsigned int k, uint32_t newlyPlacedChainsIds)
{
    if (k>=newlyPlacedChainsIds_arraysize) throw omnetpp::cRuntimeError("Array of size %d indexed by %d", newlyPlacedChainsIds_arraysize, k);
    this->newlyPlacedChainsIds[k] = newlyPlacedChainsIds;
}

class PlacementInfoMsgDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertynames;
  public:
    PlacementInfoMsgDescriptor();
    virtual ~PlacementInfoMsgDescriptor();

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

Register_ClassDescriptor(PlacementInfoMsgDescriptor)

PlacementInfoMsgDescriptor::PlacementInfoMsgDescriptor() : omnetpp::cClassDescriptor("PlacementInfoMsg", "omnetpp::cMessage")
{
    propertynames = nullptr;
}

PlacementInfoMsgDescriptor::~PlacementInfoMsgDescriptor()
{
    delete[] propertynames;
}

bool PlacementInfoMsgDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<PlacementInfoMsg *>(obj)!=nullptr;
}

const char **PlacementInfoMsgDescriptor::getPropertyNames() const
{
    if (!propertynames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
        const char **basenames = basedesc ? basedesc->getPropertyNames() : nullptr;
        propertynames = mergeLists(basenames, names);
    }
    return propertynames;
}

const char *PlacementInfoMsgDescriptor::getProperty(const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : nullptr;
}

int PlacementInfoMsgDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 1+basedesc->getFieldCount() : 1;
}

unsigned int PlacementInfoMsgDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeFlags(field);
        field -= basedesc->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISARRAY | FD_ISEDITABLE,
    };
    return (field>=0 && field<1) ? fieldTypeFlags[field] : 0;
}

const char *PlacementInfoMsgDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldName(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldNames[] = {
        "newlyPlacedChainsIds",
    };
    return (field>=0 && field<1) ? fieldNames[field] : nullptr;
}

int PlacementInfoMsgDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount() : 0;
    if (fieldName[0]=='n' && strcmp(fieldName, "newlyPlacedChainsIds")==0) return base+0;
    return basedesc ? basedesc->findField(fieldName) : -1;
}

const char *PlacementInfoMsgDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeString(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "uint32_t",
    };
    return (field>=0 && field<1) ? fieldTypeStrings[field] : nullptr;
}

const char **PlacementInfoMsgDescriptor::getFieldPropertyNames(int field) const
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

const char *PlacementInfoMsgDescriptor::getFieldProperty(int field, const char *propertyname) const
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

int PlacementInfoMsgDescriptor::getFieldArraySize(void *object, int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount();
    }
    PlacementInfoMsg *pp = (PlacementInfoMsg *)object; (void)pp;
    switch (field) {
        case 0: return pp->getNewlyPlacedChainsIdsArraySize();
        default: return 0;
    }
}

const char *PlacementInfoMsgDescriptor::getFieldDynamicTypeString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldDynamicTypeString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    PlacementInfoMsg *pp = (PlacementInfoMsg *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string PlacementInfoMsgDescriptor::getFieldValueAsString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldValueAsString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    PlacementInfoMsg *pp = (PlacementInfoMsg *)object; (void)pp;
    switch (field) {
        case 0: return ulong2string(pp->getNewlyPlacedChainsIds(i));
        default: return "";
    }
}

bool PlacementInfoMsgDescriptor::setFieldValueAsString(void *object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->setFieldValueAsString(object,field,i,value);
        field -= basedesc->getFieldCount();
    }
    PlacementInfoMsg *pp = (PlacementInfoMsg *)object; (void)pp;
    switch (field) {
        case 0: pp->setNewlyPlacedChainsIds(i,string2ulong(value)); return true;
        default: return false;
    }
}

const char *PlacementInfoMsgDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructName(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    };
}

void *PlacementInfoMsgDescriptor::getFieldStructValuePointer(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructValuePointer(object, field, i);
        field -= basedesc->getFieldCount();
    }
    PlacementInfoMsg *pp = (PlacementInfoMsg *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}


