#ifndef SHS_2605_CPP_ALIGN_H
#define SHS_2605_CPP_ALIGN_H

#define alignof(x) ( sizeof ( struct __CPPAlignHelper__ <x> ) - sizeof ( x ) )

template <typename T>
struct __CPPAlignHelper__
{

	char base;
	T t;

};

#endif
