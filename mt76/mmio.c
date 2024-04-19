// SPDX-License-Identifier: ISC
/*
 * Copyright (C) 2016 Felix Fietkau <nbd@nbd.name>
 */

#include "mt76.h"
#include "dma.h"
#include "trace.h"

static u32 mt7902_mt76_mmio_rr(struct mt7902_mt76_dev *dev, u32 offset)
{
	u32 val;

	val = readl(dev->mmio.regs + offset);
	// trace_reg_rr(dev, offset, val);

	return val;
}

static void mt7902_mt76_mmio_wr(struct mt7902_mt76_dev *dev, u32 offset, u32 val)
{
	// trace_reg_wr(dev, offset, val);
	writel(val, dev->mmio.regs + offset);
}

static u32 mt7902_mt76_mmio_rmw(struct mt7902_mt76_dev *dev, u32 offset, u32 mask, u32 val)
{
	val |= mt7902_mt76_mmio_rr(dev, offset) & ~mask;
	mt7902_mt76_mmio_wr(dev, offset, val);
	return val;
}

static void mt7902_mt76_mmio_write_copy(struct mt7902_mt76_dev *dev, u32 offset,
				 const void *data, int len)
{
	__iowrite32_copy(dev->mmio.regs + offset, data, DIV_ROUND_UP(len, 4));
}

static void mt7902_mt76_mmio_read_copy(struct mt7902_mt76_dev *dev, u32 offset,
				void *data, int len)
{
	__ioread32_copy(data, dev->mmio.regs + offset, DIV_ROUND_UP(len, 4));
}

static int mt7902_mt76_mmio_wr_rp(struct mt7902_mt76_dev *dev, u32 base,
			   const struct mt7902_mt76_reg_pair *data, int len)
{
	while (len > 0) {
		mt7902_mt76_mmio_wr(dev, data->reg, data->value);
		data++;
		len--;
	}

	return 0;
}

static int mt7902_mt76_mmio_rd_rp(struct mt7902_mt76_dev *dev, u32 base,
			   struct mt7902_mt76_reg_pair *data, int len)
{
	while (len > 0) {
		data->value = mt7902_mt76_mmio_rr(dev, data->reg);
		data++;
		len--;
	}

	return 0;
}

void mt7902_mt76_set_irq_mask(struct mt7902_mt76_dev *dev, u32 addr,
		       u32 clear, u32 set)
{
	unsigned long flags;

	spin_lock_irqsave(&dev->mmio.irq_lock, flags);
	dev->mmio.irqmask &= ~clear;
	dev->mmio.irqmask |= set;
	if (addr) {
		if (mtk_wed_device_active(&dev->mmio.wed))
			mtk_wed_device_irq_set_mask(&dev->mmio.wed,
						    dev->mmio.irqmask);
		else
			mt7902_mt76_mmio_wr(dev, addr, dev->mmio.irqmask);
	}
	spin_unlock_irqrestore(&dev->mmio.irq_lock, flags);
}
EXPORT_SYMBOL_GPL(mt7902_mt76_set_irq_mask);

void mt7902_mt76_mmio_init(struct mt7902_mt76_dev *dev, void __iomem *regs)
{
	static const struct mt7902_mt76_bus_ops mt7902_mt76_mmio_ops = {
		.rr = mt7902_mt76_mmio_rr,
		.rmw = mt7902_mt76_mmio_rmw,
		.wr = mt7902_mt76_mmio_wr,
		.write_copy = mt7902_mt76_mmio_write_copy,
		.read_copy = mt7902_mt76_mmio_read_copy,
		.wr_rp = mt7902_mt76_mmio_wr_rp,
		.rd_rp = mt7902_mt76_mmio_rd_rp,
		.type = MT76_BUS_MMIO,
	};

	dev->bus = &mt7902_mt76_mmio_ops;
	dev->mmio.regs = regs;

	spin_lock_init(&dev->mmio.irq_lock);
}
EXPORT_SYMBOL_GPL(mt7902_mt76_mmio_init);
