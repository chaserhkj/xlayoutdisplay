#ifndef XLAYOUTDISPLAYS_EDID_H
#define XLAYOUTDISPLAYS_EDID_H

#include <memory>

// Edid info from monitor - assumes 1.4 spec
class Edid {
public:
    virtual ~Edid() {};

    virtual int maxCmHoriz() const = 0;
    virtual int maxCmVert() const = 0;
};

typedef std::shared_ptr<Edid> EdidP;

class EdidImpl : public Edid {
public:
    EdidImpl(const unsigned char *edid, const size_t length, const char *name);
    virtual ~EdidImpl();

    int maxCmHoriz() const;
    int maxCmVert() const;

private:
    unsigned char *edid;
};

#endif //XLAYOUTDISPLAYS_EDID_H