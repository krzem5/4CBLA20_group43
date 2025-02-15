/*
 * Copyright (c) Krzesimir Hyżyk - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Created on 15/02/2025 by Krzesimir Hyżyk
 */



#ifndef _UTIL_UTIL_H_
#define _UTIL_UTIL_H_ 1
#include <stdio.h>
#include <stdlib.h>



#define ASSERT(x) if (!(x)){printf("%u(%s): %s: Assertion failed\n",__LINE__,__func__,#x);_Exit(1);}



#endif
