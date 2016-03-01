/*
 * MST FTM drv Support
 *
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kobject.h>
#include <linux/slab.h>
#include <linux/syscalls.h>
#include <linux/gpio.h>
#include <linux/fs.h>
#include <linux/platform_device.h>
#include <linux/of_gpio.h>
#include <linux/delay.h>
#include <linux/regulator/consumer.h>
#include <linux/spinlock.h>
#include <linux/smc.h>
#include <linux/wakelock.h>
#include <mach/exynos-pm.h>

#include "mst_ftmdrv.h"

#define MST_LDO3_0 "MST_LEVEL_3.0V"
#define MST_NOT_SUPPORT		(0x1 << 3)

#if 0
static int mst_en = 0;
static int mst_pd = 0;
static int mst_md = 0;
#if defined(CONFIG_MST_NOBLE_TARGET) || defined(CONFIG_MST_ZEN_TARGET)
static int mst_phase_en = 0;
#endif
#endif

static bool mst_power_on = 0;
static int escape_loop = 1;
static struct class *mst_ftmdrv_class;
struct device *mst_ftmdrv_dev;
static spinlock_t event_lock;
static struct wake_lock   mst_wakelock;

EXPORT_SYMBOL_GPL(mst_ftmdrv_dev);

static void of_mst_hw_onoff(bool on){
	struct regulator *regulator3_0;
	int ret;

	regulator3_0 = regulator_get(NULL, MST_LDO3_0);
	if (IS_ERR(regulator3_0)) {
		printk("%s : regulator 3.0 is not available", __func__);
		return;
	}

	if (mst_power_on == on) {
		printk("mst-drv : mst_power_onoff : already %d\n", on);
		regulator_put(regulator3_0);
		return;
	}

	mst_power_on = on;

	printk("mst-drv : mst_power_onoff : %d\n", on);

	if(regulator3_0 == NULL){
		printk(KERN_ERR "%s: regulator3_0 is invalid(NULL)\n", __func__);
		return ;
	}

	if(on) {
		ret = regulator_enable(regulator3_0);
		if (ret < 0) {
			printk("%s : regulator 3.0 is not enable", __func__);
		}
	}else{
		regulator_disable(regulator3_0);
	}

	regulator_put(regulator3_0);
}

/* mst ftm suspend&resume code */
static int mst_ftm_device_suspend(struct platform_device *dev, pm_message_t state)
{
	u64 r0 = 0, r1 = 0, r2 = 0, r3 = 0;
	int result=0;
	
	printk(KERN_ERR "%s\n", __func__);
	printk(KERN_ERR "MST_FTM_DRV]]] suspend");
	//Will Add here
	r0 = (0x8300000c);
	result = exynos_smc(r0, r1, r2, r3);

	if(result == MST_NOT_SUPPORT){
		printk(KERN_INFO "MST_FTM_DRV]]] suspend do nothing after smc : %x\n", result);
	}else{
		printk(KERN_INFO "MST_FTM_DRV]]] suspend success after smc : %x\n", result);
	}

	
	return 0;
}
static int mst_ftm_device_resume(struct platform_device *dev)
{
	u64 r0 = 0, r1 = 0, r2 = 0, r3 = 0;
	int result=0;
	
	printk(KERN_ERR "%s\n", __func__);
	printk(KERN_ERR "MST_FTM_DRV]]] resume");
	//Will Add here
	r0 = (0x8300000d);
	result = exynos_smc(r0, r1, r2, r3);

	if(result == MST_NOT_SUPPORT){
		printk(KERN_INFO "MST_FTM_DRV]]] resume do nothing after smc : %x\n", result);
	}else{
		printk(KERN_INFO "MST_FTM_DRV]]] resume success after smc : %x\n", result);
	}

	return 0;
}

static ssize_t show_mst_ftmdrv(struct device *dev,
        struct device_attribute *attr, char *buf)
{
    if (!dev)
        return -ENODEV;

    // todo
    return sprintf(buf, "%s\n", "MST ftmdrv data");
}

static ssize_t store_mst_ftmdrv(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	u64 r0 = 0, r1 = 0, r2 = 0, r3 = 0;
	char test_result[256]={0,};
	int result=0;

	sscanf(buf, "%88s\n", test_result);
	printk(KERN_ERR "MST Store test result : %s\n", test_result);

/*
#if defined(CONFIG_MST_NOBLE_TARGET) || defined(CONFIG_MST_ZEN_TARGET)
	printk("[MST] Data Value : %d, %d, %d, %d\n", mst_en, mst_pd, mst_md, mst_phase_en);
#else
	printk("[MST] Data Value : %d, %d, %d\n", mst_en, mst_pd, mst_md);
#endif
*/

	switch(test_result[0]){
		case '1':
			of_mst_hw_onoff(1);
			printk(KERN_INFO "%s\n", __func__);
			printk(KERN_INFO "MST_FTM_DRV]]] Track1 data transmit\n");
			//Will Add here
			r0 = (0x8300000f);
			r1 = 1;
			result = exynos_smc(r0, r1, r2, r3);
			if(result == 0){
				printk(KERN_INFO "MST_FTM_DRV]]] Track1 data sent : %d\n", result);
			}else if(result == -1){
				printk(KERN_INFO "MST_FTM_DRV]]] Invalid Track number : %d\n", result);
			}else{
				printk(KERN_INFO "MST_FTM_DRV]]] Invalid SMC return : %d\n", result);
			}	
			of_mst_hw_onoff(0);
			break;
	
		case '2':
			of_mst_hw_onoff(1);
			printk(KERN_INFO "%s\n", __func__);
			printk(KERN_INFO "MST_FTM_DRV]]] Track2 data transmit\n");
			//Will Add here
			r0 = (0x8300000f);
			r1 = 2;
			result = exynos_smc(r0, r1, r2, r3);
			if(result == 0){
				printk(KERN_INFO "MST_FTM_DRV]]] Track1 data sent : %d\n", result);
			}else if(result == -1){
				printk(KERN_INFO "MST_FTM_DRV]]] Invalid Track number : %d\n", result);
			}else{
				printk(KERN_INFO "MST_FTM_DRV]]] Invalid SMC return : %d\n", result);
			}
			of_mst_hw_onoff(0);
			break;
				
		case '3':
#if 0			
			case_i = 1;
			printk("MST send track1 data, case_i : %d\n", case_i);
			if (transmit_mst_data(case_i)) {
				printk("track1 data is sent\n");
			}

			mdelay(300);
			
			case_i = 2; 
			printk("MST send track2 data, case_i : %d\n", case_i);
			if (transmit_mst_data(case_i)) {
				printk("track2 data is sent\n");
			}
#endif
			break;

		case '4':
			if(escape_loop){
				wake_lock_init(&mst_wakelock, WAKE_LOCK_SUSPEND, "mst_wakelock");
				wake_lock(&mst_wakelock);
			}
			escape_loop = 0;
			while( 1 ) {
				if(escape_loop == 1)
					break;
				of_mst_hw_onoff(1);
				mdelay(10);
				printk("MST_FTM_DRV]]] Track2 data transmit to infinity\n");
				r0 = (0x8300000f);
				r1 = 2;
				result = exynos_smc(r0, r1, r2, r3);
				printk(KERN_INFO "MST_FTM_DRV]]] Track2 data transmit to infinity after smc : %d\n", result);
				of_mst_hw_onoff(0);
				mdelay(1000);
			}

			break;

		case '5':
			if(!escape_loop)
				wake_lock_destroy(&mst_wakelock);
			escape_loop = 1;
			printk("MST escape_loop value = 1\n");
			break;

		case '6':
			of_mst_hw_onoff(1);
			printk(KERN_INFO "%s\n", __func__);
			printk(KERN_INFO "MST_LDO_DRV]]] Track3 data transmit\n");
			//Will Add here
			r0 = (0x8300000f);
			r1 = 3;
			result = exynos_smc(r0, r1, r2, r3);
			printk(KERN_INFO "MST_LDO_DRV]]] Track3 data sent : %d\n", result);
			of_mst_hw_onoff(0);
			break;

		default:
			printk("MST data send failed\n");
			break;
	}

	of_mst_hw_onoff(0);

	return count;
}

static DEVICE_ATTR(transmit, 0777, show_mst_ftmdrv, store_mst_ftmdrv);
#if 0
static int sec_mst_gpio_init(struct device *dev)
{
	int ret = 0;
	mst_en = of_get_named_gpio(dev->of_node, "sec-mst,mst-pwr-gpio", 0);
	mst_pd = of_get_named_gpio(dev->of_node, "sec-mst,mst-pd-gpio", 0);
	mst_md = of_get_named_gpio(dev->of_node, "sec-mst,mst-md-gpio", 0);
#if defined(CONFIG_MST_NOBLE_TARGET) || defined(CONFIG_MST_ZEN_TARGET)
	mst_phase_en = of_get_named_gpio(dev->of_node, "sec-mst,mst-phase-en", 0);
#endif

#if defined(CONFIG_MST_NOBLE_TARGET) || defined(CONFIG_MST_ZEN_TARGET)
		printk("[MST] Data Value : %d, %d, %d, %d\n", mst_en, mst_pd, mst_md, mst_phase_en);
#else
		printk("[MST] Data Value : %d, %d, %d\n", mst_en, mst_pd, mst_md);
#endif

	/* check if gpio pin is inited */
	if (mst_en < 0) {
		printk(KERN_ERR "%s : Cannot create the gpio\n", __func__);
		return 1;
	}
	printk(KERN_ERR "MST_DRV]]] gpio en inited\n");

	if (mst_pd < 0) {
		printk(KERN_ERR "%s : Cannot create the gpio\n", __func__);
		return 1;
	}
	printk(KERN_ERR "MST_DRV]]] gpio pd inited\n");

	if (mst_md < 0) {
		printk(KERN_ERR "%s : Cannot create the gpio\n", __func__);
		return 1;
	}
	printk(KERN_ERR "MST_DRV]]] gpio md inited\n");

#if defined(CONFIG_MST_NOBLE_TARGET) || defined(CONFIG_MST_ZEN_TARGET)
	if (mst_phase_en < 0) {
		printk(KERN_ERR "%s : Cannot create the gpio\n", __func__);
		return 1;
	}
	printk(KERN_ERR "MST_DRV]]] gpio mst_phase_en inited\n");
#endif

	/* gpio request */
	ret = gpio_request(mst_en, "sec-mst,mst-pwr-gpio");
	if (ret) {
		printk(KERN_ERR "[MST] failed to get en gpio : %d, %d\n", ret, mst_en);
	}

	ret = gpio_request(mst_pd, "sec-mst,mst-pd-gpio");
	if (ret) {
		printk(KERN_ERR "[MST] failed to get pd gpio : %d, %d\n", ret, mst_pd);
	}

	ret = gpio_request(mst_md, "sec-mst,mst-md-gpio");
	if (ret) {
		printk(KERN_ERR "[MST] failed to get md gpio : %d, %d\n", ret, mst_md);
	}

#if defined(CONFIG_MST_NOBLE_TARGET) || defined(CONFIG_MST_ZEN_TARGET)
	ret = gpio_request(mst_phase_en, "sec-mst,mst-phase-en");
	if (ret) {
		printk(KERN_ERR "[MST] failed to get mst_phase_en gpio : %d, %d\n", ret, mst_phase_en);
	}
#endif

	/* set gpio direction */
	if (!(ret < 0)  && (mst_en > 0)) {
		gpio_direction_output(mst_en, 0);
		printk(KERN_ERR "%s : Send Output\n", __func__);
	}

	if (!(ret < 0) && (mst_pd > 0)) {
		gpio_direction_output(mst_pd, 0);
		printk(KERN_ERR "%s : Send Output\n", __func__);
	}

	if (!(ret < 0)  && (mst_md > 0)) {
		gpio_direction_output(mst_md, 0);
		printk(KERN_ERR "%s : Send Output\n", __func__);
	}

#if defined(CONFIG_MST_NOBLE_TARGET) || defined(CONFIG_MST_ZEN_TARGET)
	if (!(ret < 0)  && (mst_phase_en > 0)) {
		gpio_direction_output(mst_phase_en, 0);
		printk(KERN_ERR "%s : Send Output\n", __func__);
	}
#endif

	return 0;
}
#endif
static int mst_tempdevice_probe(struct platform_device *pdev)
{
	int err;

	printk("%s init start\n", __func__);

	spin_lock_init(&event_lock);

	//if (sec_mst_gpio_init(&pdev->dev))
	//	return -1;

	printk(KERN_ALERT "%s\n", __func__);
	mst_ftmdrv_class = class_create(THIS_MODULE, "mst");
	if (IS_ERR(mst_ftmdrv_class)) {
		err = PTR_ERR(mst_ftmdrv_class);
		goto error;
	}

	mst_ftmdrv_dev = device_create(mst_ftmdrv_class,
			NULL /* parent */, 0 /* dev_t */, NULL /* drvdata */,
			MST_FTMDRV_DEV);
	if (IS_ERR(mst_ftmdrv_dev)) {
		err = PTR_ERR(mst_ftmdrv_dev);
		goto error_destroy;
	}

	/* register this mst device with the driver core */
	err = device_create_file(mst_ftmdrv_dev, &dev_attr_transmit);
	if (err)
		goto error_destroy;

	printk(KERN_DEBUG "MST ftmdrv driver (%s) is initialized.\n", MST_FTMDRV_DEV);
	return 0;

error_destroy:
	kfree(mst_ftmdrv_dev);
	device_destroy(mst_ftmdrv_class, 0);
error:
	printk(KERN_ERR "%s: MST ftmdrv driver initialization failed\n", __FILE__);
	return err;

}

#ifdef CONFIG_CPU_IDLE
static int sec_mst_notifier(struct notifier_block *self,
				unsigned long cmd, void *v)
{
	u64 r0 = 0, r1 = 0, r2 = 0, r3 = 0;
	int result=0;

	switch (cmd) {
	case LPA_ENTER:
		/* save gpios & set previous state */
		r0 = (0x83000011);
		result = exynos_smc(r0, r1, r2, r3);
		break;
	case LPA_EXIT:
		/* restore gpios */
		r0 = (0x8300000d);
		result = exynos_smc(r0, r1, r2, r3);
		break;
	}

	return NOTIFY_OK;
}
#endif

static struct of_device_id mst_match_table[] = {
	{ .compatible = "sec-mst",},
	{},
};

static struct platform_driver sec_mst_driver = {
	.driver = {
		.owner = THIS_MODULE,
		.name = "sec-mst",
		.of_match_table = mst_match_table,
	},
	.probe = mst_tempdevice_probe,
	.suspend = mst_ftm_device_suspend,
	.resume = mst_ftm_device_resume,
};

#ifdef CONFIG_CPU_IDLE
static struct notifier_block sec_mst_notifier_block = {
	.notifier_call = sec_mst_notifier,
	.priority = 1,
};
#endif /*CONFIG_CPU_IDLE */

static int __init mst_ftmdrv_init(void)
{
	int ret=0;
	printk(KERN_ERR "%s\n", __func__);
	
#ifdef CONFIG_CPU_IDLE
		exynos_pm_register_notifier(&sec_mst_notifier_block);
#endif

	ret = platform_driver_register(&sec_mst_driver);

	printk(KERN_ERR "MST_DRV]]] init , ret val : %d\n",ret);

	return ret;
}

static void __exit mst_ftmdrv_exit (void)
{
    class_destroy(mst_ftmdrv_class);
    printk(KERN_ALERT "%s\n", __func__);
}

MODULE_AUTHOR("JASON KANG, j_seok.kang@samsung.com");
MODULE_DESCRIPTION("MST ftmdrv driver");
MODULE_VERSION("0.1");

module_init(mst_ftmdrv_init);
module_exit(mst_ftmdrv_exit);


