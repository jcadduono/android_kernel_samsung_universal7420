#
# Makefile for Secure OS booster API
#
obj-$(CONFIG_SECURE_OS_BOOSTER_API) += sec_os_booster.o

# MobiCore kernel driver path
GUD_ROOT_FOLDER := $(srctree)/drivers/gud/gud-exynos7420

# Use the available platform folder
ccflags-y += -I$(GUD_ROOT_FOLDER)/MobiCoreDriver/platforms/EXYNOS_7420_STD

# Includes MobiCore kernel driver
ccflags-y += -I$(GUD_ROOT_FOLDER)/MobiCoreDriver
ccflags-y += -I$(GUD_ROOT_FOLDER)/MobiCoreDriver/public
