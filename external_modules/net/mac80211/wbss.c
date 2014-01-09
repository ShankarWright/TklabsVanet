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
 #define DEFAULT_CCH_TIME HZ * 1 /*1 second*/
 #define DEFAULT_SCH_TIME HZ * 1


static int ieee80211_wbss_set_channel(struct ieee80211_sub_if_data *sdata,
									int channel_number) 
{
	struct wiphy *wiphy = sdata->wdev.wiphy;
	struct ieee80211_supported_band *sband = wiphy->bands[IEEE80211_BAND_5GHZ];
	struct ieee80211_channel *chan, *old_chan;
	struct ieee80211_local *local = sdata->local;

	if (!sband) {
		printk(KERN_INFO "IEEE80211_BAND_5GHZ not supported\n");
		return -1;
	}
	if (channel_number > sband->n_channels || channel_number < 0) {
		printk(KERN_INFO "Invalid Channel\n");
		return -1;
	}

	chan = &(sband->channels[channel_number]);
	old_chan = local->oper_channel;

	if (chan == old_chan) {
		printk(KERN_INFO "Channel is the same, no change\n");
		return -1;
	} 
	printk(KERN_INFO "changing to channel %d\n", channel_number);
	printk(KERN_INFO "new channel: \n band: %d, freq: %d, bw: %dMHz\n",
												chan->band,
												chan->center_freq,
												chan->target_bw);



	if (!ieee80211_set_channel_type(local, sdata, NL80211_CHAN_NO_HT)) {
		return -EBUSY;
	}

	//return ieee80211_set_channel(wiphy, sdata->dev, chan, NL80211_CHAN_NO_HT);
	local->oper_channel = chan;

	ieee80211_hw_config(local, IEEE80211_CONF_CHANGE_CHANNEL);

	return 0;
	
}

static void ieee80211_wbss_timer(unsigned long data)
{
	struct ieee80211_sub_if_data *sdata =
		(struct ieee80211_sub_if_data *) data;

	struct ieee80211_if_wbss *ifwbss = &sdata->u.wbss;
	struct ieee80211_local *local = sdata->local;

	struct ieee80211_channel *chan = sdata->local->oper_channel;

	struct wiphy *wiphy = sdata->wdev.wiphy;

	printk(KERN_INFO "ieee80211_wbss_timer(), jiffies = %lu\n", jiffies);

	if (chan) {
		printk(KERN_INFO "current channel: band: %d, freq: %d, bw: %dMHz\n",
												chan->band,
												chan->center_freq,
												chan->target_bw);

		ieee80211_queue_work(&sdata->local->hw, &sdata->work);

	} 

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
int ieee80211_wbss_join(struct ieee80211_sub_if_data *sdata) {

	struct ieee80211_if_wbss *ifwbss = &sdata->u.wbss;
	
	//struct ieee80211_channel *chan = sdata->local->oper_channel;
	int i;
	printk (KERN_INFO "ieee80211_wbss_join()\n"); /*JM*/


	//ieee80211_queue_work(&sdata->local->hw, &sdata->work);

	mod_timer(&ifwbss->timer,
		  round_jiffies(jiffies + DEFAULT_SCH_TIME));



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
void ieee80211_wbss_set_bitrate(struct ieee80211_sub_if_data *sdata, int bitrate_idx) {

	struct ieee80211_if_wbss *ifwbss = &sdata->u.wbss;
	struct ieee80211_supported_band *sband = sdata->wdev.wiphy->bands[IEEE80211_BAND_5GHZ];
	struct ieee80211_hw *hw = &sdata->local->hw;

	hw->rate = &sband->bitrates[bitrate_idx];
	printk(KERN_INFO "bitrate set to %dMB\n", hw->rate->bitrate/10);
}
void ieee80211_wbss_setup_sdata(struct ieee80211_sub_if_data *sdata)
{
	int ret;
	int i;

	struct ieee80211_if_wbss *ifwbss = &sdata->u.wbss;
	struct ieee80211_supported_band *sband = sdata->wdev.wiphy->bands[IEEE80211_BAND_5GHZ];

	printk (KERN_INFO "ieee80211_wbss_setup_sdata()\n");

	/*the bssid has to be all ffs*/
	for (i = 0; i < ETH_ALEN; i++) {
		*(ifwbss->bssid + i) = 0xff;
	}

	setup_timer(&ifwbss->timer, ieee80211_wbss_timer,
		    (unsigned long) sdata);

	 

	printk(KERN_INFO "supported bit rates: %d\n", sband->n_bitrates);
	for (i = 0; i < sband->n_bitrates; i++) {
		printk(KERN_INFO "bitrate: %d flags: 0%x, hw value: %d, hw_value_short: %d\n", 
								sband->bitrates[i].bitrate,
								sband->bitrates[i].flags,
								sband->bitrates[i].hw_value,
								sband->bitrates[i].hw_value_short);
		ifwbss->basic_rates |= BIT(i);
	}

	printk(KERN_INFO "basic_rates = 0x%x\n", ifwbss->basic_rates);

	/*for now call ieee80211_wbss_join()*/

	ret = ieee80211_wbss_join(sdata);
	
}


void ieee80211_wbss_work(struct ieee80211_sub_if_data *sdata)
{
	struct ieee80211_if_wbss *ifwbss = &sdata->u.wbss;
	struct ieee80211_local *local = sdata->local;
	struct ieee80211_supported_band *sband = sdata->wdev.wiphy->bands[IEEE80211_BAND_5GHZ];

	//struct ieee80211_channel *chan = ifwbss->channel;
	struct ieee80211_channel *chan = sdata->local->oper_channel;

	struct wiphy *wiphy = sdata->wdev.wiphy;

	int i;
	static int channel = 0;

	if (channel < sband->n_channels) {
		ieee80211_wbss_set_channel(sdata, channel++);
		mod_timer(&ifwbss->timer,
		  round_jiffies(jiffies + DEFAULT_SCH_TIME));

	}

	/*test code to check bitrate change*/
	static int br_idx = 0;
	ieee80211_wbss_set_bitrate(sdata, br_idx++);
	if(br_idx > sband->n_bitrates) {
		br_idx = 0;
	} 


}


