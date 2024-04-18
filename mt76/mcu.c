// SPDX-License-Identifier: ISC
/*
 * Copyright (C) 2019 Lorenzo Bianconi <lorenzo.bianconi83@gmail.com>
 */

#include "mt76.h"

struct sk_buff *
__mt7902_mt76_mcu_msg_alloc(struct mt7902_mt76_dev *dev, const void *data,
		     int len, int data_len, gfp_t gfp)
{
	const struct mt7902_mt76_mcu_ops *ops = dev->mcu_ops;
	struct sk_buff *skb;

	len = max_t(int, len, data_len);
	len = ops->headroom + len + ops->tailroom;

	skb = alloc_skb(len, gfp);
	if (!skb)
		return NULL;

	memset(skb->head, 0, len);
	skb_reserve(skb, ops->headroom);

	if (data && data_len)
		skb_put_data(skb, data, data_len);

	return skb;
}
EXPORT_SYMBOL_GPL(__mt7902_mt76_mcu_msg_alloc);

struct sk_buff *mt7902_mt76_mcu_get_response(struct mt7902_mt76_dev *dev,
				      unsigned long expires)
{
	unsigned long timeout;

	// dump_stack();
	dev_info(dev->dev, "mt7902_mt76_mcu_get_response expires: %ld\n", expires);

	if (!time_is_after_jiffies(expires)) {
		dev_info(dev->dev, "mt7902_mt76_mcu_get_response time expired\n");
		return NULL;
	}

	timeout = expires - jiffies;
	dev_info(dev->dev, "mt7902_mt76_mcu_get_response timeout: %ld\n", timeout);

	wait_event_timeout(dev->mcu.wait,
			   (!skb_queue_empty(&dev->mcu.res_q) ||
			    test_bit(MT76_MCU_RESET, &dev->phy.state)),
			   timeout);

	struct sk_buff * ret = skb_dequeue(&dev->mcu.res_q);
	dev_info(dev->dev, "mt7902_mt76_mcu_get_response ret: %p\n", ret);
	dump_stack();
	return ret; 
}
EXPORT_SYMBOL_GPL(mt7902_mt76_mcu_get_response);

void mt7902_mt76_mcu_rx_event(struct mt7902_mt76_dev *dev, struct sk_buff *skb)
{
	skb_queue_tail(&dev->mcu.res_q, skb);
	wake_up(&dev->mcu.wait);
}
EXPORT_SYMBOL_GPL(mt7902_mt76_mcu_rx_event);

int mt7902_mt76_mcu_send_and_get_msg(struct mt7902_mt76_dev *dev, int cmd, const void *data,
			      int len, bool wait_resp, struct sk_buff **ret_skb)
{
	struct sk_buff *skb;
	
	dev_info(dev->dev, "Entering mt7902_mt76_mcu_send_and_get_msg cmd: %d\n", cmd);

	if (dev->mcu_ops->mcu_send_msg)
		return dev->mcu_ops->mcu_send_msg(dev, cmd, data, len, wait_resp);

	skb = mt7902_mt76_mcu_msg_alloc(dev, data, len);

	dev_info(dev->dev, "mt7902_mt76_mcu_msg_alloc %p ", skb);

	if (!skb)
		return -ENOMEM;

	return mt7902_mt76_mcu_skb_send_and_get_msg(dev, skb, cmd, wait_resp, ret_skb);
}
EXPORT_SYMBOL_GPL(mt7902_mt76_mcu_send_and_get_msg);

int mt7902_mt76_mcu_skb_send_and_get_msg(struct mt7902_mt76_dev *dev, struct sk_buff *skb,
				  int cmd, bool wait_resp,
				  struct sk_buff **ret_skb)
{
	unsigned long expires;
	int ret, seq;

	if (ret_skb)
		*ret_skb = NULL;

	mutex_lock(&dev->mcu.mutex);
	dev_info(dev->dev, "mt7902_mt76_mcu_skb_send_and_get_msg acquiring mutex\n");

	ret = dev->mcu_ops->mcu_skb_send_msg(dev, skb, cmd, &seq);
	dev_info(dev->dev, "mt7902_mt76_mcu_skb_send_and_get_msg cmd: %d, ret: %d, wait_resp: %d\n", cmd, ret, wait_resp);
	if (ret < 0)
		goto out;

	if (!wait_resp) {
		ret = 0;
		goto out;
	}

	expires = jiffies + dev->mcu.timeout;

	do {
		dev_info(dev->dev, "mt7902_mt76_mcu_skb_send_and_get_msg expires: %ld\n", expires);
		skb = mt7902_mt76_mcu_get_response(dev, expires);

		dev_info(dev->dev, "SKB info: %p\n", skb);
		// unsigned char *data_ptr;
		// data_ptr = skb->data;
		// // dev_info(dev->dev, "mt7902_mt76_mcu_get_response Response : %p\n", skb->data);
		//
		// for (int i = 0; i < skb->len; i++) {
		// 	dev_info(dev->dev, "%02X ", data_ptr[i]);
		// }
		// dev_info(dev->dev, "\n");

		ret = dev->mcu_ops->mcu_parse_response(dev, cmd, skb, seq);
		dev_info(dev->dev, "mt7902_mt76_mcu_skb_send_and_get_msg > mcu_parse_response cmd: %d, ret: %d\n", cmd, ret);
		if (!ret && ret_skb)
			*ret_skb = skb;
		else
			dev_kfree_skb(skb);
	} while (ret == -EAGAIN);

out:
	mutex_unlock(&dev->mcu.mutex);
	dev_info(dev->dev, "mt7902_mt76_mcu_skb_send_and_get_msg unlocking mutex\n");

	return ret;
}
EXPORT_SYMBOL_GPL(mt7902_mt76_mcu_skb_send_and_get_msg);

int __mt7902_mt76_mcu_send_firmware(struct mt7902_mt76_dev *dev, int cmd, const void *data,
			     int len, int max_len)
{
	int err, cur_len;

	while (len > 0) {
		cur_len = min_t(int, max_len, len);

		err = mt7902_mt76_mcu_send_msg(dev, cmd, data, cur_len, false);
		if (err)
			return err;

		data += cur_len;
		len -= cur_len;

		if (dev->queue_ops->tx_cleanup)
			dev->queue_ops->tx_cleanup(dev,
						   dev->q_mcu[MT_MCUQ_FWDL],
						   false);
	}

	return 0;
}
EXPORT_SYMBOL_GPL(__mt7902_mt76_mcu_send_firmware);
