#define COUNT_OF(x)  (sizeof(x) / sizeof(x[0]))

struct s_named_bitfield
{
    int byte;
    int bit;
    const char *name;
};

using std::vector;

std::string bitlist(const vector<devio::Byte> &r, struct s_named_bitfield *bitfields, int fieldcount);
