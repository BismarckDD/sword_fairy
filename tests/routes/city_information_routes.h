/* Test input routes only. Actual7606 conversations drive original quest flags. */
#include "city_information_0_route.h"
#include "city_information_1_route.h"
#include "city_information_2_route.h"
#include "city_information_3_route.h"
#include "city_information_4_route.h"
#include "city_information_5_route.h"
#include "city_information_6_route.h"
#include "city_information_7_route.h"
#include "city_information_8_route.h"
#include "city_information_9_route.h"
static const struct {unsigned npc,facing;const uint16_t (*route)[2];unsigned count;} city_information_routes[]={
    {16,0,city_information_0_route,sizeof city_information_0_route/sizeof city_information_0_route[0]},
    {17,0,city_information_1_route,sizeof city_information_1_route/sizeof city_information_1_route[0]},
    {5,0,city_information_2_route,sizeof city_information_2_route/sizeof city_information_2_route[0]},
    {6,0,city_information_3_route,sizeof city_information_3_route/sizeof city_information_3_route[0]},
    {18,3,city_information_4_route,sizeof city_information_4_route/sizeof city_information_4_route[0]},
    {19,3,city_information_5_route,sizeof city_information_5_route/sizeof city_information_5_route[0]},
    {20,3,city_information_6_route,sizeof city_information_6_route/sizeof city_information_6_route[0]},
    {21,3,city_information_7_route,sizeof city_information_7_route/sizeof city_information_7_route[0]},
    {0,0,city_information_8_route,sizeof city_information_8_route/sizeof city_information_8_route[0]},
    {2,0,city_information_9_route,sizeof city_information_9_route/sizeof city_information_9_route[0]},
};
