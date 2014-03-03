#include "SHSAnimations.h"


LEDStripAnimator * BuildTestAnimation ( LEDStrip * Strip )
{

	LEDStripAnimator * Animator = new LEDStripAnimator ( Strip, 14 );

	uint32_t Colors1 [ 14 ] = { 0xFFFFFF, 0xCCCCFF, 0x9999FF, 0x6666FF, 0x3333FF, 0x0066CC, 0x009999, 0x00CC66, 0x00FF33, 0x00FF00, 0x33FF33, 0x66FF66, 0x99FF99, 0xCCFFCC };
	uint32_t Colors2 [ 14 ] = { 0xCCCCFF, 0x9999FF, 0x6666FF, 0x3333FF, 0x0066CC, 0x009999, 0x00CC66, 0x00FF33, 0x00FF00, 0x33FF33, 0x66FF66, 0x99FF99, 0xCCFFCC, 0xFFFFFF };
	uint32_t Colors3 [ 14 ] = { 0x9999FF, 0x6666FF, 0x3333FF, 0x0066CC, 0x009999, 0x00CC66, 0x00FF33, 0x00FF00, 0x33FF33, 0x66FF66, 0x99FF99, 0xCCFFCC, 0xFFFFFF, 0xCCCCFF };
	uint32_t Colors4 [ 14 ] = { 0x6666FF, 0x3333FF, 0x0066CC, 0x009999, 0x00CC66, 0x00FF33, 0x00FF00, 0x33FF33, 0x66FF66, 0x99FF99, 0xCCFFCC, 0xFFFFFF, 0xCCCCFF, 0x9999FF };
	uint32_t Colors5 [ 14 ] = { 0x3333FF, 0x0066CC, 0x009999, 0x00CC66, 0x00FF33, 0x00FF00, 0x33FF33, 0x66FF66, 0x99FF99, 0xCCFFCC, 0xFFFFFF, 0xCCCCFF, 0x9999FF, 0x6666FF };
	uint32_t Colors6 [ 14 ] = { 0x0066CC, 0x009999, 0x00CC66, 0x00FF33, 0x00FF00, 0x33FF33, 0x66FF66, 0x99FF99, 0xCCFFCC, 0xFFFFFF, 0xCCCCFF, 0x9999FF, 0x6666FF, 0x3333FF };
	uint32_t Colors7 [ 14 ] = { 0x009999, 0x00CC66, 0x00FF33, 0x00FF00, 0x33FF33, 0x66FF66, 0x99FF99, 0xCCFFCC, 0xFFFFFF, 0xCCCCFF, 0x9999FF, 0x6666FF, 0x3333FF, 0x0066CC };
	uint32_t Colors8 [ 14 ] = { 0x00CC66, 0x00FF33, 0x00FF00, 0x33FF33, 0x66FF66, 0x99FF99, 0xCCFFCC, 0xFFFFFF, 0xCCCCFF, 0x9999FF, 0x6666FF, 0x3333FF, 0x0066CC, 0x009999 };
	uint32_t Colors9 [ 14 ] = { 0x00FF33, 0x00FF00, 0x33FF33, 0x66FF66, 0x99FF99, 0xCCFFCC, 0xFFFFFF, 0xCCCCFF, 0x9999FF, 0x6666FF, 0x3333FF, 0x0066CC, 0x009999, 0x00CC66 };
	uint32_t Colors10 [ 14 ] = { 0x00FF00, 0x33FF33, 0x66FF66, 0x99FF99, 0xCCFFCC, 0xFFFFFF, 0xCCCCFF, 0x9999FF, 0x6666FF, 0x3333FF, 0x0066CC, 0x009999, 0x00CC66, 0x00FF33 };
	uint32_t Colors11 [ 14 ] = { 0x33FF33, 0x66FF66, 0x99FF99, 0xCCFFCC, 0xFFFFFF, 0xCCCCFF, 0x9999FF, 0x6666FF, 0x3333FF, 0x0066CC, 0x009999, 0x00CC66, 0x00FF33, 0x00FF00 };
	uint32_t Colors12 [ 14 ] = { 0x66FF66, 0x99FF99, 0xCCFFCC, 0xFFFFFF, 0xCCCCFF, 0x9999FF, 0x6666FF, 0x3333FF, 0x0066CC, 0x009999, 0x00CC66, 0x00FF33, 0x00FF00, 0x33FF33 };
	uint32_t Colors13 [ 14 ] = { 0x99FF99, 0xCCFFCC, 0xFFFFFF, 0xCCCCFF, 0x9999FF, 0x6666FF, 0x3333FF, 0x0066CC, 0x009999, 0x00CC66, 0x00FF33, 0x00FF00, 0x33FF33, 0x66FF66 };
	uint32_t Colors14 [ 14 ] = { 0xCCFFCC, 0xFFFFFF, 0xCCCCFF, 0x9999FF, 0x6666FF, 0x3333FF, 0x0066CC, 0x009999, 0x00CC66, 0x00FF33, 0x00FF00, 0x33FF33, 0x66FF66, 0x99FF99 };

	Animator -> SetFrame ( 0, Colors1, 0, 14, 0.04 );
	Animator -> SetFrame ( 1, Colors2, 0, 14, 0.04 );
	Animator -> SetFrame ( 2, Colors3, 0, 14, 0.04 );
	Animator -> SetFrame ( 3, Colors4, 0, 14, 0.04 );
	Animator -> SetFrame ( 4, Colors5, 0, 14, 0.04 );
	Animator -> SetFrame ( 5, Colors6, 0, 14, 0.04 );
	Animator -> SetFrame ( 6, Colors7, 0, 14, 0.04 );
	Animator -> SetFrame ( 7, Colors8, 0, 14, 0.04 );
	Animator -> SetFrame ( 8, Colors9, 0, 14, 0.04 );
	Animator -> SetFrame ( 9, Colors10, 0, 14, 0.04 );
	Animator -> SetFrame ( 10, Colors11, 0, 14, 0.04 );
	Animator -> SetFrame ( 11, Colors12, 0, 14, 0.04 );
	Animator -> SetFrame ( 12, Colors13, 0, 14, 0.04 );
	Animator -> SetFrame ( 13, Colors14, 0, 14, 0.04 );

	Animator -> SetClearedLEDS ( 0, 14, true );

	return Animator;

};