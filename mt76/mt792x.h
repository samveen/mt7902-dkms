/* SPDX-License-Identifier: ISC */
/* Copyright (C) 2023 MediaTek Inc. */

#ifndef __MT792X_H
#define __MT792X_H

#include <linux/version.h>
#include <linux/interrupt.h>
#include <linux/ktime.h>

#include "mt76_connac_mcu.h"
#include "mt792x_regs.h"
#include "mt792x_acpi_sar.h"

#define MT792x_PM_TIMEOUT	(HZ / 12)
#define MT792x_HW_SCAN_TIMEOUT	(HZ / 10)

#define MT792x_MAX_INTERFACES	4
#define MT792x_WTBL_SIZE	20
#define MT792x_WTBL_RESERVED	(MT792x_WTBL_SIZE - 1)
#define MT792x_WTBL_STA		(MT792x_WTBL_RESERVED - MT792x_MAX_INTERFACES)

#define MT792x_CFEND_RATE_DEFAULT	0x49	/* OFDM 24M */
#define MT792x_CFEND_RATE_11B		0x03	/* 11B LP, 11M */

#define MT792x_FW_TAG_FEATURE	4
#define MT792x_FW_CAP_CNM	BIT(7)

#define MT792x_CHIP_CAP_CLC_EVT_EN BIT(0)

/* NOTE: used to map mt7902_mt76_rates. idx may change if firmware expands table */
#define MT792x_BASIC_RATES_TBL	11

#define MT792x_WATCHDOG_TIME	(HZ / 4)

#define MT792x_DRV_OWN_RETRY_COUNT	10
#define MT792x_MCU_INIT_RETRY_COUNT	10
#define MT792x_WFSYS_INIT_RETRY_COUNT	2

#define MT7902_FIRMWARE_WM	"mediatek/WIFI_RAM_CODE_MT7902_1.bin"
#define MT7902_ROM_PATCH	"mediatek/WIFI_MT7902_patch_mcu_1_1_hdr.bin"

#define MT7921_FIRMWARE_WM	"mediatek/WIFI_RAM_CODE_MT7961_1.bin"
#define MT7922_FIRMWARE_WM	"mediatek/WIFI_RAM_CODE_MT7922_1.bin"
#define MT7925_FIRMWARE_WM	"mediatek/mt7925/WIFI_RAM_CODE_MT7925_1_1.bin"

#define MT7921_ROM_PATCH	"mediatek/WIFI_MT7961_patch_mcu_1_2_hdr.bin"
#define MT7922_ROM_PATCH	"mediatek/WIFI_MT7922_patch_mcu_1_1_hdr.bin"
#define MT7925_ROM_PATCH	"mediatek/mt7925/WIFI_MT7925_PATCH_MCU_1_1_hdr.bin"

#define MT792x_SDIO_HDR_TX_BYTES	GENMASK(15, 0)
#define MT792x_SDIO_HDR_PKT_TYPE	GENMASK(17, 16)

struct mt7902_mt792x_vif;
struct mt7902_mt792x_sta;

struct mt7902_mt792x_realease_info {
	__le16 len;
	u8 pad_len;
	u8 tag;
} __packed;

struct mt7902_mt792x_fw_features {
	u8 segment;
	u8 data;
	u8 rsv[14];
} __packed;

enum {
	MT792x_CLC_POWER,
	MT792x_CLC_CHAN,
	MT792x_CLC_MAX_NUM,
};

enum mt7902_mt792x_reg_power_type {
	MT_AP_UNSET = 0,
	MT_AP_DEFAULT,
	MT_AP_LPI,
	MT_AP_SP,
	MT_AP_VLP,
};

DECLARE_EWMA(avg_signal, 10, 8)

struct mt7902_mt792x_sta {
	struct mt7902_mt76_wcid wcid; /* must be first */

	struct mt7902_mt792x_vif *vif;

	u32 airtime_ac[8];

	int ack_signal;
	struct ewma_avg_signal avg_ack_signal;

	unsigned long last_txs;

	struct mt7902_mt76_connac_sta_key_conf bip;
};

DECLARE_EWMA(rssi, 10, 8);

struct mt7902_mt792x_vif {
	struct mt7902_mt76_vif mt76; /* must be first */

	struct mt7902_mt792x_sta sta;
	struct mt7902_mt792x_sta *wep_sta;

	struct mt7902_mt792x_phy *phy;

	struct ewma_rssi rssi;

	struct ieee80211_tx_queue_params queue_params[IEEE80211_NUM_ACS];
};

struct mt7902_mt792x_phy {
	struct mt7902_mt76_phy *mt76;
	struct mt7902_mt792x_dev *dev;

	struct ieee80211_sband_iftype_data iftype[NUM_NL80211_BANDS][NUM_NL80211_IFTYPES];

	u64 omac_mask;

	u16 noise;

	s16 coverage_class;
	u8 slottime;

	u32 rx_ampdu_ts;
	u32 ampdu_ref;

	struct mt7902_mt76_mib_stats mib;

	u8 sta_work_count;
	u8 clc_chan_conf;
	enum mt7902_mt792x_reg_power_type power_type;

	struct sk_buff_head scan_event_list;
	struct delayed_work scan_work;
#ifdef CONFIG_ACPI
	void *acpisar;
#endif
	void *clc[MT792x_CLC_MAX_NUM];
	u64 chip_cap;

	struct work_struct roc_work;
	struct timer_list roc_timer;
	wait_queue_head_t roc_wait;
	u8 roc_token_id;
	bool roc_grant;
};

struct mt7902_mt792x_irq_map {
	u32 host_irq_enable;
	struct {
		u32 all_complete_mask;
		u32 mcu_complete_mask;
	} tx;
	struct {
		u32 data_complete_mask;
		u32 wm_complete_mask;
		u32 wm2_complete_mask;
	} rx;
};

#define mt7902_mt792x_init_reset(dev)		((dev)->hif_ops->init_reset(dev))
#define mt7902_mt792x_dev_reset(dev)		((dev)->hif_ops->reset(dev))
#define mt7902_mt792x_mcu_init(dev)		((dev)->hif_ops->mcu_init(dev))
#define __mt7902_mt792x_mcu_drv_pmctrl(dev)	((dev)->hif_ops->drv_own(dev))
#define	__mt7902_mt792x_mcu_fw_pmctrl(dev)	((dev)->hif_ops->fw_own(dev))

struct mt7902_mt792x_hif_ops {
	int (*init_reset)(struct mt7902_mt792x_dev *dev);
	int (*reset)(struct mt7902_mt792x_dev *dev);
	int (*mcu_init)(struct mt7902_mt792x_dev *dev);
	int (*drv_own)(struct mt7902_mt792x_dev *dev);
	int (*fw_own)(struct mt7902_mt792x_dev *dev);
};

struct mt7902_mt792x_dev {
	union { /* must be first */
		struct mt7902_mt76_dev mt76;
		struct mt7902_mt76_phy mphy;
	};

	const struct mt7902_mt76_bus_ops *bus_ops;
	struct mt7902_mt792x_phy phy;

	struct work_struct reset_work;
	bool hw_full_reset:1;
	bool hw_init_done:1;
	bool fw_assert:1;
	bool has_eht:1;

	struct work_struct init_work;

	u8 fw_debug;
	u8 fw_features;

	struct mt7902_mt76_connac_pm pm;
	struct mt7902_mt76_connac_coredump coredump;
	const struct mt7902_mt792x_hif_ops *hif_ops;
	const struct mt7902_mt792x_irq_map *irq_map;

	struct work_struct ipv6_ns_work;
	/* IPv6 addresses for WoWLAN */
	struct sk_buff_head ipv6_ns_list;

	enum environment_cap country_ie_env;
	u32 backup_l1;
	u32 backup_l2;
};

static inline struct mt7902_mt792x_dev *
mt7902_mt792x_hw_dev(struct ieee80211_hw *hw)
{
	struct mt7902_mt76_phy *phy = hw->priv;

	return container_of(phy->dev, struct mt7902_mt792x_dev, mt76);
}

static inline struct mt7902_mt792x_phy *
mt7902_mt792x_hw_phy(struct ieee80211_hw *hw)
{
	struct mt7902_mt76_phy *phy = hw->priv;

	return phy->priv;
}

static inline void
mt7902_mt792x_get_status_freq_info(struct mt7902_mt76_rx_status *status, u8 chfreq)
{
	if (chfreq > 180) {
		status->band = NL80211_BAND_6GHZ;
		chfreq = (chfreq - 181) * 4 + 1;
	} else if (chfreq > 14) {
		status->band = NL80211_BAND_5GHZ;
	} else {
		status->band = NL80211_BAND_2GHZ;
	}
	status->freq = ieee80211_channel_to_frequency(chfreq, status->band);
}

static inline bool mt7902_mt792x_dma_need_reinit(struct mt7902_mt792x_dev *dev)
{
	return !mt7902_mt76_get_field(dev, MT_WFDMA_DUMMY_CR, MT_WFDMA_NEED_REINIT);
}

#define mt7902_mt792x_mutex_acquire(dev)	\
	mt7902_mt76_connac_mutex_acquire(&(dev)->mt76, &(dev)->pm)
#define mt7902_mt792x_mutex_release(dev)	\
	mt7902_mt76_connac_mutex_release(&(dev)->mt76, &(dev)->pm)

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 11, 0))
void mt7902_mt792x_stop(struct ieee80211_hw *hw, bool suspend);
#else
void mt7902_mt792x_stop(struct ieee80211_hw *hw);
#endif
void mt7902_mt792x_pm_wake_work(struct work_struct *work);
void mt7902_mt792x_pm_power_save_work(struct work_struct *work);
void mt7902_mt792x_reset(struct mt7902_mt76_dev *mdev);
void mt7902_mt792x_update_channel(struct mt7902_mt76_phy *mphy);
void mt7902_mt792x_mac_reset_counters(struct mt7902_mt792x_phy *phy);
void mt7902_mt792x_mac_init_band(struct mt7902_mt792x_dev *dev, u8 band);
void mt7902_mt792x_mac_assoc_rssi(struct mt7902_mt792x_dev *dev, struct sk_buff *skb);
struct mt7902_mt76_wcid *mt7902_mt792x_rx_get_wcid(struct mt7902_mt792x_dev *dev, u16 idx,
				     bool unicast);
void mt7902_mt792x_mac_update_mib_stats(struct mt7902_mt792x_phy *phy);
void mt7902_mt792x_mac_set_timeing(struct mt7902_mt792x_phy *phy);
void mt7902_mt792x_mac_work(struct work_struct *work);
void mt7902_mt792x_remove_interface(struct ieee80211_hw *hw,
			     struct ieee80211_vif *vif);
void mt7902_mt792x_tx(struct ieee80211_hw *hw, struct ieee80211_tx_control *control,
	       struct sk_buff *skb);
int mt7902_mt792x_conf_tx(struct ieee80211_hw *hw, struct ieee80211_vif *vif,
		   unsigned int link_id, u16 queue,
		   const struct ieee80211_tx_queue_params *params);
int mt7902_mt792x_get_stats(struct ieee80211_hw *hw,
		     struct ieee80211_low_level_stats *stats);
u64 mt7902_mt792x_get_tsf(struct ieee80211_hw *hw, struct ieee80211_vif *vif);
void mt7902_mt792x_set_tsf(struct ieee80211_hw *hw, struct ieee80211_vif *vif,
		    u64 timestamp);
void mt7902_mt792x_tx_worker(struct mt7902_mt76_worker *w);
void mt7902_mt792x_roc_timer(struct timer_list *timer);
void mt7902_mt792x_flush(struct ieee80211_hw *hw, struct ieee80211_vif *vif,
		  u32 queues, bool drop);
int mt7902_mt792x_assign_vif_chanctx(struct ieee80211_hw *hw,
			      struct ieee80211_vif *vif,
			      struct ieee80211_bss_conf *link_conf,
			      struct ieee80211_chanctx_conf *ctx);
void mt7902_mt792x_unassign_vif_chanctx(struct ieee80211_hw *hw,
				 struct ieee80211_vif *vif,
				 struct ieee80211_bss_conf *link_conf,
				 struct ieee80211_chanctx_conf *ctx);
void mt7902_mt792x_set_wakeup(struct ieee80211_hw *hw, bool enabled);
void mt7902_mt792x_get_et_strings(struct ieee80211_hw *hw, struct ieee80211_vif *vif,
			   u32 sset, u8 *data);
int mt7902_mt792x_get_et_sset_count(struct ieee80211_hw *hw, struct ieee80211_vif *vif,
			     int sset);
void mt7902_mt792x_get_et_stats(struct ieee80211_hw *hw, struct ieee80211_vif *vif,
			 struct ethtool_stats *stats, u64 *data);
void mt7902_mt792x_sta_statistics(struct ieee80211_hw *hw,
			   struct ieee80211_vif *vif,
			   struct ieee80211_sta *sta,
			   struct station_info *sinfo);
void mt7902_mt792x_set_coverage_class(struct ieee80211_hw *hw, s16 coverage_class);
void mt7902_mt792x_dma_cleanup(struct mt7902_mt792x_dev *dev);
int mt7902_mt792x_dma_enable(struct mt7902_mt792x_dev *dev);
int mt7902_mt792x_wpdma_reset(struct mt7902_mt792x_dev *dev, bool force);
int mt7902_mt792x_wpdma_reinit_cond(struct mt7902_mt792x_dev *dev);
int mt7902_mt792x_dma_disable(struct mt7902_mt792x_dev *dev, bool force);
irqreturn_t mt7902_mt792x_irq_handler(int irq, void *dev_instance);
void mt7902_mt792x_rx_poll_complete(struct mt7902_mt76_dev *mdev, enum mt7902_mt76_rxq_id q);
int mt7902_mt792x_poll_tx(struct napi_struct *napi, int budget);
int mt7902_mt792x_poll_rx(struct napi_struct *napi, int budget);
void mt7902_mt792x_irq_tasklet(unsigned long data);
int mt7902_mt792x_wfsys_reset(struct mt7902_mt792x_dev *dev);
int mt7902_mt792x_tx_stats_show(struct seq_file *file, void *data);
int mt7902_mt792x_queues_acq(struct seq_file *s, void *data);
int mt7902_mt792x_queues_read(struct seq_file *s, void *data);
int mt7902_mt792x_pm_stats(struct seq_file *s, void *data);
int mt7902_mt792x_pm_idle_timeout_set(void *data, u64 val);
int mt7902_mt792x_pm_idle_timeout_get(void *data, u64 *val);
int mt7902_mt792x_init_wiphy(struct ieee80211_hw *hw);
struct ieee80211_ops *
mt7902_mt792x_get_mac80211_ops(struct device *dev,
			const struct ieee80211_ops *mac80211_ops,
			void *drv_data, u8 *fw_features);
int mt7902_mt792x_init_wcid(struct mt7902_mt792x_dev *dev);
int mt7902_mt792x_mcu_drv_pmctrl(struct mt7902_mt792x_dev *dev);
int mt7902_mt792x_mcu_fw_pmctrl(struct mt7902_mt792x_dev *dev);

static inline char *mt7902_mt792x_ram_name(struct mt7902_mt792x_dev *dev)
{
	switch (mt7902_mt76_chip(&dev->mt76)) {
        case 0x7902:
                return MT7902_FIRMWARE_WM;
	case 0x7922:
		return MT7922_FIRMWARE_WM;
	case 0x7925:
		return MT7925_FIRMWARE_WM;
	default:
		return MT7921_FIRMWARE_WM;
	}
}

static inline char *mt7902_mt792x_patch_name(struct mt7902_mt792x_dev *dev)
{
	switch (mt7902_mt76_chip(&dev->mt76)) {
	case 0x7902:
		return MT7902_ROM_PATCH;
	case 0x7922:
		return MT7922_ROM_PATCH;
	case 0x7925:
		return MT7925_ROM_PATCH;
	default:
		return MT7921_ROM_PATCH;
	}
}

int mt7902_mt792x_load_firmware(struct mt7902_mt792x_dev *dev);

/* usb */
#define MT_USB_TYPE_VENDOR	(USB_TYPE_VENDOR | 0x1f)
#define MT_USB_TYPE_UHW_VENDOR	(USB_TYPE_VENDOR | 0x1e)
int mt7902_mt792xu_dma_init(struct mt7902_mt792x_dev *dev, bool resume);
int mt7902_mt792xu_mcu_power_on(struct mt7902_mt792x_dev *dev);
int mt7902_mt792xu_wfsys_reset(struct mt7902_mt792x_dev *dev);
int mt7902_mt792xu_init_reset(struct mt7902_mt792x_dev *dev);
u32 mt7902_mt792xu_rr(struct mt7902_mt76_dev *dev, u32 addr);
void mt7902_mt792xu_wr(struct mt7902_mt76_dev *dev, u32 addr, u32 val);
u32 mt7902_mt792xu_rmw(struct mt7902_mt76_dev *dev, u32 addr, u32 mask, u32 val);
void mt7902_mt792xu_copy(struct mt7902_mt76_dev *dev, u32 offset, const void *data, int len);
void mt7902_mt792xu_disconnect(struct usb_interface *usb_intf);
void mt7902_mt792xu_stop(struct ieee80211_hw *hw);

static inline void
mt7902_mt792x_skb_add_usb_sdio_hdr(struct mt7902_mt792x_dev *dev, struct sk_buff *skb,
			    int type)
{
	u32 hdr, len;

	len = mt7902_mt76_is_usb(&dev->mt76) ? skb->len : skb->len + sizeof(hdr);
	hdr = FIELD_PREP(MT792x_SDIO_HDR_TX_BYTES, len) |
	      FIELD_PREP(MT792x_SDIO_HDR_PKT_TYPE, type);

	put_unaligned_le32(hdr, skb_push(skb, sizeof(hdr)));
}

int __mt7902_mt792xe_mcu_drv_pmctrl(struct mt7902_mt792x_dev *dev);
int mt7902_mt792xe_mcu_drv_pmctrl(struct mt7902_mt792x_dev *dev);
int mt7902_mt792xe_mcu_fw_pmctrl(struct mt7902_mt792x_dev *dev);

#ifdef CONFIG_ACPI
int mt7902_mt792x_init_acpi_sar(struct mt7902_mt792x_dev *dev);
int mt7902_mt792x_init_acpi_sar_power(struct mt7902_mt792x_phy *phy, bool set_default);
u8 mt7902_mt792x_acpi_get_flags(struct mt7902_mt792x_phy *phy);
u8 mt7902_mt792x_acpi_get_mtcl_conf(struct mt7902_mt792x_phy *phy, char *alpha2);
#else
static inline int mt7902_mt792x_init_acpi_sar(struct mt7902_mt792x_dev *dev)
{
	return 0;
}

static inline int mt7902_mt792x_init_acpi_sar_power(struct mt7902_mt792x_phy *phy,
					     bool set_default)
{
	return 0;
}

static inline u8 mt7902_mt792x_acpi_get_flags(struct mt7902_mt792x_phy *phy)
{
	return 0;
}

static inline u8 mt7902_mt792x_acpi_get_mtcl_conf(struct mt7902_mt792x_phy *phy, char *alpha2)
{
	return 0xf;
}
#endif

#endif /* __MT7925_H */
