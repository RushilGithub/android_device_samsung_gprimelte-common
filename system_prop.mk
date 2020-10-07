# OpenGLES
PRODUCT_PROPERTY_OVERRIDES += \
    ro.opengles.version=196608

# Radio
PRODUCT_PROPERTY_OVERRIDES += \
	persist.radio.snapshot_enabled=1 \
	persist.radio.snapshot_timer=22 \
	persist.radio.lte_vrte_ltd=1 \
	persist.radio.jbims=1 \
	persist.radio.no_wait_for_card=1

# Telephony
PRODUCT_PROPERTY_OVERRIDES += \
	ro.telephony.default_network=10

# Volume
PRODUCT_PROPERTY_OVERRIDES += \
	ro.config.vc_call_vol_steps=15 \
	ro.config.media_vol_steps=30

PRODUCT_PROPERTY_OVERRIDES += \
	ro.config.low_ram=true
