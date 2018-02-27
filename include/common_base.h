#ifndef COMMON_BASE_SDF878DF_H_
#define COMMON_BASE_SDF878DF_H_

#include <cstring>
#include <cassert>
// 0资金  1股份   2当日委托  3当日成交     4可撤单   5股东代码  6融资余额   7融券余额  8可融证券</param>
enum class TypeQueryCategory : char
{
    CAPITAL = 0,
    STOCK = 1,
    TODAY_DELEGATE = 2,
    TODAY_FILL = 3,
    CAN_RECALL_ORDER,
    SHARED_HOLDER_CODE = 5,
    RZYE,
    RQYE,
    KRZQ,
};

enum class TypeMarket : char
{
    SZ = 0,
    SH
};

struct T_AccountData
{
    char shared_holder_code[64]; 
    char name[64];
    TypeMarket type;
    char capital_code[64]; 
    char seat_code[64];
    char rzrq_tag[64];  //融资融券 标识
};


class Buffer
{
public:
	// notice size + 1 == 1024 is suitable for avoid memory fragment
    explicit Buffer(unsigned int size=64) : size_(size), p_data_(nullptr) 
    {
        p_data_ = new char[size + 1];
        memset(p_data_, 0, size + 1); 
    }
    explicit Buffer(const char* p_str, unsigned int size) 
    {
        assert(p_str);
        p_data_ = new char[size + 1];
        size_ = size;
        memcpy(p_data_, p_str, size);
        p_data_[size] = '\0';
    } 

    ~Buffer(){ if( p_data_ ) delete[] p_data_; p_data_ = nullptr;};

    char * data() { return p_data_; }
    const char *c_data() const {return p_data_;}
    unsigned int size() const {return size_;}
    void reset() { memset(p_data_, 0, size_ + 1); }

private:

    Buffer(Buffer&);
    Buffer& operator = (Buffer&);

    unsigned int size_;
    char *p_data_;
};

#endif // COMMON_BASE_SDF878DF_H_