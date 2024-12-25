#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>

static int __init first_driver_init(void)
{
    printk(KERN_INFO "[FirstDriver] init\n");    
    return 0;
}

void __exit first_driver_exit(void)
{
    printk(KERN_INFO "[FirstDriver] exit\n");
}

module_init(first_driver_init);
module_exit(first_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Group 11");
MODULE_DESCRIPTION("First driver");
