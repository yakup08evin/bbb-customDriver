#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/kdev_t.h>



#undef pr_fmt
#define	pr_fmt(fmt) "%s : " fmt,__func__

#define NO_OF_DEVICES 4

#define MEM_SIZE_MAX_PCDDEV1 1024
#define MEM_SIZE_MAX_PCDDEV2 512
#define MEM_SIZE_MAX_PCDDEV3 1024
#define MEM_SIZE_MAX_PCDDEV4 512

/* pseudo device's memory */
char device_buffer_pcddev1[DEV_MEM_MAX_PCDDEV1];
char device_buffer_pcddev2[DEV_MEM_MAX_PCDDEV2];
char device_buffer_pcddev3[DEV_MEM_MAX_PCDDEV3];
char device_buffer_pcddev4[DEV_MEM_MAX_PCDDEV4];


/*dEVİCE private structer */
struct pcdev_private_data
{
	char *buffer;
	unsigned size;
	const char *serial_number;
	int perm;
	struct cdev cdev;
}

/*Driver private data structer */
struct pcdrv_private_data
{
	int total_devices;
	struct pcdev_private_data pcdev_data[NO_OF_DEVICES];
}

dev_t device_number;

struct cdev pcd_cdev;

loff_t pcd_lseek(struct file *filp, loff_t off, int whence)
{

	loff_t temp;

	pr_info("lseek requested \n");
	pr_info("Current value of the file position = %lld\n",filp->f_pos);
	switch(whence)
	{
		case SEEK_SET:
			temp = off;
			if((temp > DEV_MEM_SIZE) || temp < 0 )
			{
				return -EINVAL;
			}
			filp->f_pos = off;
			break;
		case SEEK_CUR:
			temp = filp->f_pos + off;
			if((temp > DEV_MEM_SIZE) || temp < 0)
			{
				return -EINVAL;
			}
			filp->f_pos = temp;
			break;
		case SEEK_END:
			temp = DEV_MEM_SIZE + off;
                        if((temp > DEV_MEM_SIZE) || temp < 0)
                        {
                                return -EINVAL;
                        }
                        filp->f_pos = temp;
			break;
		default:
			return -EINVAL;
	}
	pr_info("Current value of the file position = %lld\n",filp->f_pos);
	return filp->f_pos;
}
ssize_t pcd_read(struct file *filp, char __user *buff, size_t count, loff_t *f_pos)
{
  	pr_info("read requested %zu bytes \n",count);
	pr_info("current file position = %lld\n",*f_pos);

	/*Adjust the count  */
	if(*f_pos + count > DEV_MEM_SIZE){
		count = DEV_MEM_SIZE - *f_pos;
	}
	/*Copy to user*/
	if(copy_to_user(buff,&device_buffer[*f_pos],count)){
		return -EFAULT;
	}
	/*update to current file position */
	*f_pos += count;

	pr_info("number of files successful read = %zu\n",count);
	pr_info("update file position = %lld\n",*f_pos);



	return count;
}
ssize_t pcd_write(struct file *filp, const char __user *buff, size_t count, loff_t *f_pos)
{
 	pr_info("write requested %zu bytes \n",count);
	pr_info("current file position = %lld\n",*f_pos);
        /*Adjust the count  */
        if(*f_pos + count > DEV_MEM_SIZE){
                count = DEV_MEM_SIZE - *f_pos;
        }
	if(!count){
		return -ENOMEM;
	}
	if(copy_from_user(&device_buffer[*f_pos],buff,count)){
		return -EFAULT;
	}
        *f_pos += count;

        pr_info("number of files successful written  = %zu\n",count);
        pr_info("update file position = %lld\n",*f_pos);



        return count;

}
int pcd_open(struct inode *inode, struct file *filp)
{  
	pr_info("open was  succsessful \n");
	return 0;
}
int pcd_release(struct inode *inode, struct file *filp)
{
	pr_info("close was successful \n");
	return 0;
}


struct file_operations pcd_fops = 
{
	.open = pcd_open,
	.write = pcd_write,
	.read = pcd_read,
	.release = pcd_release,
	.llseek = pcd_lseek,
	.owner = THIS_MODULE
};

struct class *class_pcd;

struct device *device_pcd;

static int __init pcd_driver_init(void)
{

	int ret;
	//1.Dynamically allocate a device number
	ret=alloc_chrdev_region(&device_number,0,1,"pcd_devices");
	if(ret < 0)
	{
		pr_err("Alloc chrdev region failed\n ");
		goto out;
	}
	pr_info("%s :  Device number <major>:<minor> = %d:%d\n",__func__,MAJOR(device_number),MINOR(device_number));


	//2.Initialize the cdev structure with fops
	cdev_init(&pcd_cdev,&pcd_fops);
	
	//3.Register a device (cdev structure) with VFS
	pcd_cdev.owner = THIS_MODULE;
	ret=cdev_add(&pcd_cdev,device_number,1);
        if(ret < 0)
        {
                pr_err("chrdev add  failed\n ");
                goto unreg_chrdev;
        }

	
	//create device class under /sys/class/ 
	class_pcd = class_create(THIS_MODULE,"pcd_class");
	if(IS_ERR(class_pcd))
	{
		pr_err("Class creaation failed\n");
		ret = PTR_ERR(class_pcd);
		goto cdev_del;
	}
	
	device_pcd = device_create(class_pcd,NULL,device_number,NULL,"pcd");
        if(IS_ERR(device_pcd))
        {
                pr_err("device pcd failed\n");
                ret = PTR_ERR(device_pcd);
                goto class_del;
        }

	
	pr_info("Module init was succesful\n");

	return 0;
class_del:
	 class_destroy(class_pcd);
cdev_del:
	 cdev_del(&pcd_cdev);
unreg_chrdev:
	 unregister_chrdev_region(device_number,1);
out:
	pr_info("Module init was successful\n");
	return ret;

}


static void __exit pcd_driver_cleanup(void)
{
	device_destroy(class_pcd,device_number);
	class_destroy(class_pcd);
	cdev_del(&pcd_cdev);
	unregister_chrdev_region(device_number,1);
	pr_info("module unloaded");

}

module_init(pcd_driver_init);
module_exit(pcd_driver_cleanup);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("YAKUP");
MODULE_DESCRIPTION("A pseudo  character driver");


