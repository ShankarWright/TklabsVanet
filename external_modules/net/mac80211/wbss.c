/*
 * WBSS mode implementation
 * Jose A. Mena <jose.mena@tklabs.com>
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/if_ether.h>
#include <linux/skbuff.h>
#include <linux/if_arp.h>
#include <linux/etherdevice.h>
#include <linux/rtnetlink.h>
#include <net/mac80211.h>

#include "ieee80211_i.h"
//#include "driver-ops.h"
#include "rate.h"

 #define IEEE80211_WBSS_MAX_STA_ENTRIES 128

static void ieee80211_wbss_timer(unsigned long data)
{
	struct ieee80211_sub_if_data *sdata =
		(struct ieee80211_sub_if_data *) data;
	struct ieee80211_if_wbss *ifwbss = &sdata->u.wbss;
	struct ieee80211_local *local = sdata->local;

	printk (KERN_INFO "ieee80211_wbss_timer(), jiffies = %lu\n", jiffies);

	//if (local->quiescing) {
	//	ifwbss->timer_running = true;
	//	return;
	//}

	//ieee80211_queue_work(&local->hw, &sdata->work);
}

static void __ieee80211_sta_join_wbss(void)
{

	printk(KERN_INFO "__ieee80211_sta_join_wbss()\n");

}

static void ieee80211_sta_join_wbss(void) 
{
	printk(KERN_INFO "ieee80211_sta_join_wbss()\n");

}

/*this function will be called by 1609.4 module or user space
 *to start a wbss */
int ieee80211_wbss_join(struct ieee80211_sub_if_data *sdata,
			struct cfg80211_ibss_params *params) {

	printk (KERN_INFO "ieee80211_wbss_join()\n"); /*JM*/

	ieee80211_queue_work(&sdata->local->hw, &sdata->work);

	return 0;
}

/*for other types of interfaces a station is added when
 *association is achieved, is this required with WAVE stations??*/
 
static struct sta_info *
ieee80211_wbss_add_sta(struct ieee80211_sub_if_data *sdata,
		       const u8 *bssid, const u8 *addr, u32 supp_rates)
	__acquires(RCU)
{
	struct ieee80211_if_wbss *ifwbss = &sdata->u.wbss;
	struct ieee80211_local *local = sdata->local;
	struct sta_info *sta;
	int band = local->hw.conf.channel->band;

	if (local->num_sta >= IEEE80211_WBSS_MAX_STA_ENTRIES) {
		if (net_ratelimit())
			printk(KERN_DEBUG "%s: No room for a new WBSS STA entry %pM\n",
			       sdata->name, addr);
		rcu_read_lock();
		return NULL;
	}


	sta = sta_info_alloc(sdata, addr, GFP_KERNEL);
	if (!sta) {
		rcu_read_lock();
		return NULL;
	}

	sta->last_rx = jiffies;

	/* make sure mandatory rates are always added */
	sta->sta.supp_rates[band] = supp_rates |
			ieee80211_mandatory_rates(local, band);

#ifdef CONFIG_MAC80211_VERBOSE_DEBUG
	wiphy_debug(sdata->local->hw.wiphy,
		    "Adding new WBSS station %pM (dev=%s)\n",
		    addr, sdata->name);
#endif

	rate_control_rate_init(sta);

	/* If it fails, maybe we raced another insertion? */
	if (sta_info_insert_rcu(sta))
		return sta_info_get(sdata, addr);

	return sta;
}

void ieee80211_wbss_setup_sdata(struct ieee80211_sub_if_data *sdata)
{
	int ret;
	int i;

	struct ieee80211_if_wbss *ifwbss = &sdata->u.wbss;

	/*the bssid has to be all ffs*/
	for (i = 0; i < ETH_ALEN; i++) {
		*(ifwbss->bssid + i) = 0xff;
	}

	setup_timer(&ifwbss->timer, ieee80211_wbss_timer,
		    (unsigned long) sdata);

	printk (KERN_INFO "ieee80211_wbss_setup_sdata()\n"); 

	/*for now I will call ieee80211_wbss_join() myself*/

	ret = ieee80211_wbss_join(sdata, NULL);
	
}


void ieee80211_wbss_work(struct ieee80211_sub_if_data *sdata)
{
	printk(KERN_INFO "wbss is working\n");
}

