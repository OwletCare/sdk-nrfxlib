/*
 * Copyright (c) 2020 - 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/**
 * @file
 *   This file contains system-agnostic implementation of the nRF 802.15.4 GPIOTE
 *   critical section abstraction.
 */

#include "platform/nrf_802154_gpiote.h"

#include "hal/nrf_gpio.h"
#include "nrf_802154_sl_utils.h"
#include "platform/nrf_802154_irq.h"

static volatile bool     m_gpiote_irq_enabled;      ///< Whether the GPIOTE interrupt was turned on before entering the first critical section.
static volatile uint32_t m_gpiote_irq_disabled_cnt; ///< Counter of how many times GPIOTE interrupt was disabled while entering critical section.

void nrf_802154_gpiote_critical_section_enter(void)
{
    nrf_802154_sl_mcu_critical_state_t mcu_cs;
    uint32_t                           cnt;

    nrf_802154_sl_mcu_critical_enter(mcu_cs);
    cnt = m_gpiote_irq_disabled_cnt;

    if (cnt == 0U)
    {
        m_gpiote_irq_enabled = nrf_802154_irq_is_enabled(GPIOTE_IRQn);
        nrf_802154_irq_disable(GPIOTE_IRQn);
    }

    cnt++;
    m_gpiote_irq_disabled_cnt = cnt;

    nrf_802154_sl_mcu_critical_exit(mcu_cs);
}

void nrf_802154_gpiote_critical_section_exit(void)
{
    nrf_802154_sl_mcu_critical_state_t mcu_cs;
    uint32_t                           cnt;

    nrf_802154_sl_mcu_critical_enter(mcu_cs);

    cnt = m_gpiote_irq_disabled_cnt;
    cnt--;

    if (cnt == 0U)
    {
        if (m_gpiote_irq_enabled)
        {
            nrf_802154_irq_enable(GPIOTE_IRQn);
        }
    }

    m_gpiote_irq_disabled_cnt = cnt;

    nrf_802154_sl_mcu_critical_exit(mcu_cs);
}
