# Partition Manager links the app at 0x0, but the UF2 bootloader loads it at
# the devicetree code partition. That build succeeds and the board never boots,
# so stop it here.
if(CONFIG_BOARD_SUPERMINI_NRF52840_NRF52840_UF2 AND CONFIG_PARTITION_MANAGER_ENABLED)
  message(FATAL_ERROR
    "Partition Manager is on for the UF2 variant. It links the app at 0x0, "
    "but the bootloader loads it at 0x26000. Keep SB_CONFIG_PARTITION_MANAGER "
    "off; Kconfig.sysbuild in this directory sets that default.")
endif()
