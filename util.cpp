#include "devio.h"
#include "stringbuilder.h"
#include "util.h"

using std::vector;
using devio::Byte;

// return list of bit fields which are set
std::string bitlist(const vector<Byte> &r, struct s_named_bitfield *bitfields, int fieldcount)
{
    auto s = stringbuilder();
    bool wasbits = false;
    
    for (int byte = 0; byte < 3; byte++)
    {
        for (int bit = 0; bit < 8; bit++)
        {
            if (r[byte].bit(bit))
            {
                if (wasbits)
                {
                    s << ", ";
                }
                else
                {
                    wasbits = true;
                }
                
                int bf;
                for (bf = 0; bf < fieldcount; bf++)
                {
                    if (bitfields[bf].byte == byte &&
                        bitfields[bf].bit == bit)
                    {
                        s << bitfields[bf].name;
                        break;
                    }
                }
                
                if (bf == fieldcount)
                {
                    s << "byte_" << byte << "_bit_" << bit;
                }
            }
        }
    }
    
    if (!wasbits)
    {
        s << "none";
    }
    
    return s;
}

