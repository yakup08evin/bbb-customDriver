#include<linux/module.h>

static int __init helloWorld_init(void)
{
  pr_info("Hello world\n");
  return 0;
}
static void __exit helloWorld_cleanup(void)
{
  pr_info("Good bye\n");
}

module_init(helloWorld_init);
module_exit(helloWorld_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("YAKUP");
MODULE_DESCRIPTION("A simple hello world kernel module");
MODULE_INFO(board,"beagle-boneblack");


