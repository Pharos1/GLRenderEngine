#pragma once
#ifndef FLAGS
#define FLAGS

#include "Shader.h"

enum PostProcFlag {
	PostProcFlag_none = 0,
	PostProcFlag_hdr = 1 << 0, //For removal
	PostProcFlag_gammaCorrection = 1 << 1,
	PostProcFlag_bloom = 1 << 2,
	PostProcFlag_blur = 1 << 3,
	PostProcFlag_reinhard = 1 << 4, //For removal
	PostProcFlag_6 = 1 << 5,
	PostProcFlag_7 = 1 << 6,
	PostProcFlag_8 = 1 << 7,
	PostProcFlag_9 = 1 << 8,
	PostProcFlag_10 = 1 << 9,
	PostProcFlag_11 = 1 << 10,
	PostProcFlag_12 = 1 << 11,
	PostProcFlag_13 = 1 << 12,
	PostProcFlag_14 = 1 << 13,
	PostProcFlag_15 = 1 << 14,
	PostProcFlag_16 = 1 << 15,
};

class PostProcFlags{
public:
    //Sets flag to true
    void setFlag(PostProcFlag flag){
        flagValue |= (int)flag;
    }

    //Sets flag to false
    void unsetFlag(PostProcFlag flag){
        flagValue &= ~(int)flag;
    }

    //Sets a flag value from true to false and vice versa
    void flipFlag(PostProcFlag flag){
        flagValue ^= (int)flag;
    }
    bool hasFlag(PostProcFlag flag) {
        return (flagValue & (int)flag) == (int)flag;
    }
    bool getFlags(){
        return flagValue;
    }
    void set(Shader& shader, const char* name) {
        shader.use();
        shader.set1ui(name, flagValue);
    }

    PostProcFlags(uint16_t flags) {
        flagValue = flags;
    }
    PostProcFlags() {};

    uint16_t flagValue = 0;
};
//unsetting a flag can be done by flags &= ~flag
#endif