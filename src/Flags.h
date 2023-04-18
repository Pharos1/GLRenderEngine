#pragma once
#ifndef FLAGS
#define FLAGS

enum PostProcFlags {
	PostProcFlags_none = 0,
	PostProcFlags_hdr = 1 << 0,
	PostProcFlags_gammaCorrection = 1 << 1,
	PostProcFlags_bloom = 1 << 2,
	PostProcFlags_blur = 1 << 3,
	PostProcFlags_5 = 1 << 4,
	PostProcFlags_6 = 1 << 5,
	PostProcFlags_7 = 1 << 6,
	PostProcFlags_8 = 1 << 7,
	PostProcFlags_9 = 1 << 8,
	PostProcFlags_10 = 1 << 9,
	PostProcFlags_11 = 1 << 10,
	PostProcFlags_12 = 1 << 11,
	PostProcFlags_13 = 1 << 12,
	PostProcFlags_14 = 1 << 13,
	PostProcFlags_15 = 1 << 14,
	PostProcFlags_16 = 1 << 15,
};
//unsetting a flag can be done by flags &= ~flag
#endif