#ifndef _CECP_FEATURES_H
#define _CECP_FEATURES_H

#include <stdio.h>

/* Features of the Chess Engine Communication Protocol supported by this program */
const char *cecp_features[] =
{
    "done=0",
    "setboard=1",
    "usermove=1",
    "draw=0",
    "sigint=0",
    "analyze=0",
    "myname=\"Pawned " _VERSION "\"",
    "variants=\"normal\"",
    "colors=0",
    "name=0",
    "nps=0",
    "debug=1",
    "memory=1",
    "smp=1",
    "done=1",
    0
};

#endif
